#include <RSP.hpp>
#include <util.hpp>
#include <n64/core/Mem.hpp>
#include "Interrupt.hpp"

namespace natsukashii::n64::core {
void RSP::StepRSP(MI& mi, Registers& regs, RDP& rdp) {
  if(!spStatus.halt) {
    gpr[0] = 0;
    u32 instr = util::ReadAccess<u32>(imem, pc & IMEM_DSIZE);
    oldPC = pc & 0xFFF;
    pc = nextPC & 0xFFF;
    nextPC += 4;
    Exec(mi, regs, rdp, instr);
  }
}

u32 RSP::Read(u32 addr) {
  switch (addr) {
    case 0x04040000: return spDMASPAddr.raw & 0xFFFFF8;
    case 0x04040004: return spDMADRAMAddr.raw & 0x1FF8;
    case 0x04040008: return spDMARDLen.raw;
    case 0x0404000C: return spDMAWRLen.raw;
    case 0x04040010: return spStatus.raw;
    case 0x04040018: return 0;
    case 0x04080000: return pc & 0xFFF;
    default: util::panic("Unimplemented SP register read %08X\n", addr);
  }
}

template <bool isDRAMdest>
inline void DMA(SPDMALen len, RSP& rsp, u8* dst, u8* src) {
  u32 length = len.len + 1;

  length = (length + 0x7) & ~0x7;

  u32 last_addr = rsp.spDMASPAddr.address + length;
  if (last_addr > 0x1000) {
    u32 overshoot = last_addr - 0x1000;
    length -= overshoot;
  }

  u32 dram_address = rsp.spDMADRAMAddr.address & 0xFFFFF8;
  u32 mem_address = rsp.spDMASPAddr.address & 0x1FF8;

  for (int i = 0; i < len.count + 1; i++) {
    if(isDRAMdest) {
      memcpy(&dst[dram_address], &src[mem_address], length);
    } else {
      memcpy(&dst[mem_address], &src[dram_address], length);
    }

    int skip = i == len.count ? 0 : len.skip;

    dram_address += (length + skip) & 0xFFFFF8;
    mem_address += length;
  }
}

void RSP::Write(Mem& mem, Registers& regs, u32 addr, u32 value) {
  MI& mi = mem.mmio.mi;
  switch (addr) {
    case 0x04040000: spDMASPAddr.raw = value & 0x1FF8; break;
    case 0x04040004: spDMADRAMAddr.raw = value & 0xFFFFF8; break;
    case 0x04040008: {
      spDMARDLen.raw = value;
      DMA<false>(spDMARDLen, *this, spDMASPAddr.bank ? imem : dmem, mem.GetRDRAM());
      spDMARDLen.raw = 0xFF8 | (spDMARDLen.skip << 20);
    } break;
    case 0x0404000C: {
      spDMAWRLen.raw = value;
      DMA<true>(spDMAWRLen, *this, mem.GetRDRAM(), spDMASPAddr.bank ? imem : dmem);
      spDMAWRLen.raw = 0xFF8 | (spDMAWRLen.skip << 20);
    } break;
    case 0x04040010: {
      SPStatusWrite write;
      write.raw = value;
      CLEAR_SET(spStatus.halt, write.clearHalt, write.setHalt);
      CLEAR_SET(spStatus.broke, write.clearBroke, false);
      if(write.clearIntr) InterruptLower(mi, regs, InterruptType::SP);
      if(write.setIntr) InterruptRaise(mi, regs, InterruptType::SP);
      CLEAR_SET(spStatus.singleStep, write.clearSstep, write.setSstep);
      CLEAR_SET(spStatus.interruptOnBreak, write.clearIntrOnBreak, write.setIntrOnBreak);
      CLEAR_SET(spStatus.signal0Set, write.clearSignal0, write.setSignal0);
      CLEAR_SET(spStatus.signal1Set, write.clearSignal1, write.setSignal1);
      CLEAR_SET(spStatus.signal2Set, write.clearSignal2, write.setSignal2);
      CLEAR_SET(spStatus.signal3Set, write.clearSignal3, write.setSignal3);
      CLEAR_SET(spStatus.signal4Set, write.clearSignal4, write.setSignal4);
      CLEAR_SET(spStatus.signal5Set, write.clearSignal5, write.setSignal5);
      CLEAR_SET(spStatus.signal6Set, write.clearSignal6, write.setSignal6);
      CLEAR_SET(spStatus.signal7Set, write.clearSignal7, write.setSignal7);
    } break;
    case 0x04080000:
      if(spStatus.halt) {
        oldPC = pc;
        pc = nextPC;
        nextPC = value & 0xFFF;
      } break;
    default: util::panic("Unimplemented SP register write {:08X}, val: {:08X}\n", addr, value);
  }
}

}
