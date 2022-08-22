#include <n64/core/RDP.hpp>
#include <util.hpp>
#include <n64/core/RSP.hpp>
#include <parallel-rdp/ParallelRDPWrapper.hpp>
#include <n64/core/mmio/Interrupt.hpp>

namespace n64 {
RDP::RDP() {
  Reset();
}

void RDP::Reset() {
  dpc.status.raw = 0x80;
  dram.resize(RDRAM_SIZE);
  std::fill(dram.begin(), dram.end(), 0);
  memset(cmd_buf, 0, 0x100000);
}

static const int cmd_lens[64] = {
  2, 2, 2, 2, 2, 2, 2, 2, 8, 12, 24, 28, 24, 28, 40, 44,
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2,  2,  2,  2,  2,  2,  2,
  2, 2, 2, 2, 4, 4, 2, 2, 2, 2,  2,  2,  2,  2,  2,  2,
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2,  2,  2,  2,  2,  2,  2
};

auto RDP::Read(u32 addr) const -> u32{
  switch(addr) {
    case 0x04100000: return dpc.start;
    case 0x04100004: return dpc.end;
    case 0x04100008: return dpc.current;
    case 0x0410000C: return dpc.status.raw;
    case 0x04100010: return dpc.clock;
    case 0x04100014: return dpc.status.cmdBusy;
    case 0x04100018: return dpc.status.pipeBusy;
    case 0x0410001C: return dpc.tmem;
    default: util::panic("Unhandled DP Command Registers read (addr: {:08X})\n", addr);
  }
  return 0;
}

void RDP::Write(MI& mi, Registers& regs, RSP& rsp, u32 addr, u32 val) {
  switch(addr) {
    case 0x04100000:
      if(dpc.status.startValid) {
        dpc.start = val & 0xFFFFF8;
      }
      dpc.status.startValid = true;
      break;
    case 0x04100004:
      dpc.end = val & 0xFFFFF8;
      if(dpc.status.startValid) {
        dpc.current = dpc.start;
        dpc.status.startValid = false;
      }
      RunCommand(mi, regs, rsp);
      break;
    case 0x0410000C: StatusWrite(mi, regs, rsp, val); break;
    default: util::panic("Unhandled DP Command Registers write (addr: {:08X}, val: {:08X})\n", addr, val);
  }
}

void RDP::StatusWrite(MI& mi, Registers& regs, RSP& rsp, u32 val) {
  DPCStatusWrite temp{};
  temp.raw = val;
  bool rdpUnfrozen = false;
  CLEAR_SET(dpc.status.xbusDmemDma, temp.clearXbusDmemDma, temp.setXbusDmemDma);
  if(temp.clearFreeze) {
    dpc.status.freeze = false;
    rdpUnfrozen = true;
  }

  if(temp.setFreeze) {
    dpc.status.freeze = true;
  }
  CLEAR_SET(dpc.status.flush, temp.clearFlush, temp.setFlush);
  CLEAR_SET(dpc.status.cmdBusy, temp.clearCmd, false);
  CLEAR_SET(dpc.clock, temp.clearClock, false);
  CLEAR_SET(dpc.status.pipeBusy, temp.clearPipe, false);
  CLEAR_SET(dpc.status.tmemBusy, temp.clearTmem, false);

  if(rdpUnfrozen) {
    RunCommand(mi, regs, rsp);
  }
}

void RDP::RunCommand(MI& mi, Registers& regs, RSP& rsp) {
  static int remaining_cmds = 0;
  dpc.status.freeze = true;

  const u32 current = dpc.current & 0xFFFFF8;
  const u32 end = dpc.end & 0xFFFFF8;

  int len = end - current;
  if(len <= 0) return;

  if(len + (remaining_cmds * 4) <= 0xFFFFF) {
    if(dpc.status.xbusDmemDma) {
      for(int i = 0; i < len; i += 4) {
        u32 cmd = util::ReadAccess<u32>(rsp.dmem, current + i);
        cmd_buf[remaining_cmds + (i >> 2)] = cmd;
      }
    } else {
      if(end > 0x7FFFFF || current > 0x7FFFFF) {
        return;
      }
      for(int i = 0; i < len; i += 4) {
        u32 cmd = util::ReadAccess<u32>(rsp.dmem, current + i);
        cmd_buf[remaining_cmds + (i >> 2)] = cmd;
      }
    }

    int word_len = (len >> 2) + remaining_cmds;
    int buf_index = 0;

    bool processed_all = true;

    while(buf_index < word_len) {
      u8 cmd = (cmd_buf[buf_index] >> 24) & 0x3F;

      int cmd_len = cmd_lens[cmd];
      if((buf_index + cmd_len) * 4 > len + (remaining_cmds * 4)) {
        remaining_cmds = word_len - buf_index;

        u32 tmp[remaining_cmds];
        for(int i = 0; i < remaining_cmds; i++) {
          tmp[i] = cmd_buf[buf_index + i];
        }

        for(int i = 0; i < remaining_cmds; i++) {
          cmd_buf[buf_index + i] = tmp[i];
        }

        processed_all = false;
        break;
      }

      if(cmd >= 8) {
        ParallelRdpEnqueueCommand(cmd_len, &cmd_buf[buf_index]);
      }

      if (cmd == 0x29) {
        OnFullSync();
        InterruptRaise(mi, regs, Interrupt::DP);
      }

      buf_index += cmd_len;
    }

    if(processed_all) {
      remaining_cmds = 0;
    }

    dpc.current = end;
    dpc.status.freeze = false;
    dpc.status.cbufReady = true;
  }
}

void RDP::OnFullSync() {
  ParallelRdpOnFullSync();
}
}
