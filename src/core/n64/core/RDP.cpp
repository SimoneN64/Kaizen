#include <RDP.hpp>
#include <util.hpp>
#include <RSP.hpp>
#include <../../../frontend/ParallelRDPWrapper.hpp>
#include <Interrupt.hpp>

namespace natsukashii::n64::core {
static const int cmd_lens[64] = {
  2, 2, 2, 2, 2, 2, 2, 2, 8, 12, 24, 28, 24, 28, 40, 44,
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2,  2,  2,  2,  2,  2,  2,
  2, 2, 2, 2, 4, 4, 2, 2, 2, 2,  2,  2,  2,  2,  2,  2,
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2,  2,  2,  2,  2,  2,  2
};

u32 RDP::Read(u32 addr) {
  switch(addr) {
    case 0x0410000C: return dpc.status.raw;
    default: util::panic("Unhandled DP Command Registers read (addr: {:08X})\n", addr);
  }
}

void RDP::Write(u32 addr, u32 val) {
  switch(addr) {
    case 0x0410000C: StatusWrite(val); break;
    default: util::panic("Unhandled DP Command Registers read (addr: {:08X}, val: {:08X})\n", addr, val);
  }
}

void RDP::StatusWrite(u32 val) {
  DPCStatusWrite temp{};
  temp.raw = val;
  CLEAR_SET(dpc.status.xbusDmemDma, temp.clearXbusDmemDma, temp.setXbusDmemDma);
  CLEAR_SET(dpc.status.freeze, temp.clearFreeze, false); // Setting it seems to break games? Avoid for now (TODO)
  CLEAR_SET(dpc.status.flush, temp.clearFlush, temp.setFlush);
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
        InterruptRaise(mi, regs, InterruptType::DP);
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
