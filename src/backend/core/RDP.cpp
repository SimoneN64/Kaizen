#include <core/RDP.hpp>
#include <log.hpp>
#include <core/RSP.hpp>
#include <parallel-rdp/ParallelRDPWrapper.hpp>
#include <core/mmio/Interrupt.hpp>

namespace n64 {
RDP::RDP() {
  rdram = (u8*)calloc(RDRAM_SIZE, 1);
  Reset();
}

void RDP::Reset() {
  dpc.status.raw = 0x80;
  memset(rdram, 0, RDRAM_SIZE);
  memset(cmd_buf, 0, 0x100000);
}

static const int cmd_lens[64] = {
  2, 2, 2, 2, 2, 2, 2, 2, 8, 12, 24, 28, 24, 28, 40, 44,
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2,  2,  2,  2,  2,  2,  2,
  2, 2, 2, 2, 4, 4, 2, 2, 2, 2,  2,  2,  2,  2,  2,  2,
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2,  2,  2,  2,  2,  2,  2
};

auto RDP::Read(u32 addr) const -> u32 {
  switch(addr) {
    case 0x04100000: return dpc.start;
    case 0x04100004: return dpc.end;
    case 0x04100008: return dpc.current;
    case 0x0410000C:
      return dpc.status.raw;
    case 0x04100010: return dpc.clock;
    case 0x04100014: return dpc.status.cmdBusy;
    case 0x04100018: return dpc.status.pipeBusy;
    case 0x0410001C: return dpc.tmem;
    default:
      Util::panic("Unhandled DP Command Registers read (addr: {:08X})", addr);
  }
}

void RDP::Write(MI& mi, Registers& regs, RSP& rsp, u32 addr, u32 val) {
  switch(addr) {
    case 0x04100000: WriteStart(val); break;
    case 0x04100004: WriteEnd(mi, regs, rsp, val); break;
    case 0x0410000C: WriteStatus(mi, regs, rsp, val); break;
    default:
      Util::panic("Unhandled DP Command Registers write (addr: {:08X}, val: {:08X})", addr, val);
  }
}

void RDP::WriteStatus(MI& mi, Registers& regs, RSP& rsp, u32 val) {
  DPCStatusWrite temp{};
  temp.raw = val;

  CLEAR_SET(dpc.status.xbusDmemDma, temp.clearXbusDmemDma, temp.setXbusDmemDma);
  if(temp.clearFreeze) {
    dpc.status.freeze = false;
  }

  if(temp.setFreeze) {
    dpc.status.freeze = true;
  }
  CLEAR_SET(dpc.status.flush, temp.clearFlush, temp.setFlush);
  CLEAR_SET(dpc.status.cmdBusy, temp.clearCmd, false);
  if(temp.clearClock) dpc.clock = 0;
  CLEAR_SET(dpc.status.pipeBusy, temp.clearPipe, false);
  CLEAR_SET(dpc.status.tmemBusy, temp.clearTmem, false);

  if(!dpc.status.freeze) {
    RunCommand(mi, regs, rsp);
  }
}

void RDP::RunCommand(MI& mi, Registers& regs, RSP& rsp) {
  if (dpc.status.freeze) {
    return;
  }
  dpc.status.pipeBusy = true;
  dpc.status.startGclk = true;
  if(dpc.end > dpc.current) {
    dpc.status.freeze = true;

    static int remaining_cmds = 0;

    const u32 current = dpc.current & 0xFFFFF8;
    const u32 end = dpc.end & 0xFFFFF8;

    int len = end - current;
    if (len <= 0) return;

    if (len + (remaining_cmds * 4) > 0xFFFFF) {
      Util::panic("Too many RDP commands");
      return;
    }

    if (dpc.status.xbusDmemDma) {
      for (int i = 0; i < len; i += 4) {
        u32 cmd = Util::ReadAccess<u32>(rsp.dmem, (current + i) & 0xFFF);
        cmd_buf[remaining_cmds + (i >> 2)] = cmd;
      }
    } else {
      if (end > 0x7FFFFFF || current > 0x7FFFFFF) { // if (end > RDRAM_DSIZE || current > RDRAM_DSIZE)
        return;
      }
      for (int i = 0; i < len; i += 4) {
        u32 cmd = Util::ReadAccess<u32>(rdram, current + i);
        cmd_buf[remaining_cmds + (i >> 2)] = cmd;
      }
    }

    int word_len = (len >> 2) + remaining_cmds;
    int buf_index = 0;

    bool processed_all = true;

    while (buf_index < word_len) {
      u8 cmd = (cmd_buf[buf_index] >> 24) & 0x3F;

      int cmd_len = cmd_lens[cmd];
      if ((buf_index + cmd_len) * 4 > len + (remaining_cmds * 4)) {
        remaining_cmds = word_len - buf_index;

        u32 tmp[remaining_cmds];
        for (int i = 0; i < remaining_cmds; i++) {
          tmp[i] = cmd_buf[buf_index + i];
        }

        for (int i = 0; i < remaining_cmds; i++) {
          cmd_buf[i] = tmp[i];
        }

        processed_all = false;
        break;
      }

      if (cmd >= 8) {
        ParallelRdpEnqueueCommand(cmd_len, &cmd_buf[buf_index]);
      }

      if (cmd == 0x29) {
        OnFullSync(mi, regs);
      }

      buf_index += cmd_len;
    }

    if (processed_all) {
      remaining_cmds = 0;
    }

    dpc.current = end;
    dpc.end = end;
    dpc.status.freeze = false;
  }
  dpc.status.cbufReady = true;
}

void RDP::OnFullSync(MI& mi, Registers& regs) {
  ParallelRdpOnFullSync();

  dpc.status.pipeBusy = false;
  dpc.status.startGclk = false;
  dpc.status.cbufReady = false;
  InterruptRaise(mi, regs, Interrupt::DP);
}
}
