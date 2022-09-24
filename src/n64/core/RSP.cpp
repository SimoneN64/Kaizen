#include <n64/core/RSP.hpp>
#include <util.hpp>
#include <n64/core/Mem.hpp>
#include <n64/core/mmio/Interrupt.hpp>

namespace n64 {
RSP::RSP() {
  Reset();
}

void RSP::Reset() {
  spStatus.raw = 0x1;
  spStatus.halt = true;
  oldPC = 0;
  pc = 0;
  nextPC = 0;
  spDMASPAddr.raw = 0;
  spDMADRAMAddr.raw = 0;
  spDMALen.raw = 0;
  memset(dmem, 0, DMEM_SIZE);
  memset(imem, 0, IMEM_SIZE);
  memset(vpr, 0, 32 * sizeof(VPR));
  memset(gpr, 0, 32);
  vce = 0;
  acc = {.h={}, .m={}, .l={}};
  vcc = {.l = {}, .h = {}};
  vco = {.l = {}, .h = {}};
  semaphore = false;
}

void RSP::Step(MI& mi, Registers& regs, RDP& rdp) {
  if(!spStatus.halt) {
   gpr[0] = 0;
   u32 instr = util::ReadAccess<u32>(imem, pc & IMEM_DSIZE);
   oldPC = pc & 0xFFC;
   pc = nextPC & 0xFFC;
   nextPC += 4;
   Exec(mi, regs, rdp, instr);
  }
}

auto RSP::Read(u32 addr) -> u32{
  switch (addr) {
    case 0x04040000: return lastSuccessfulSPAddr.raw & 0x1FF8;
    case 0x04040004: return lastSuccessfulDRAMAddr.raw & 0xFFFFF8;
    case 0x04040008:
    case 0x0404000C: return spDMALen.raw;
    case 0x04040010: return spStatus.raw;
    case 0x04040014: return spStatus.dmaFull;
    case 0x04040018: return 0;
    case 0x0404001C: return AcquireSemaphore();
    case 0x04080000: return pc & 0xFFC;
    default: util::panic("Unimplemented SP register read {:08X}\n", addr);
  }
}

template <bool isDRAMdest>
inline void DMA(SPDMALen len, Mem& mem, RSP& rsp, bool bank) {
  u32 length = len.len + 1;

  length = (length + 0x7) & ~0x7;

  u8* dst, *src;
  if constexpr (isDRAMdest) {
    dst = mem.GetRDRAM();
    src = bank ? rsp.imem : rsp.dmem;
  } else {
    src = mem.GetRDRAM();
    dst = bank ? rsp.imem : rsp.dmem;
  }

  u32 mem_address = rsp.spDMASPAddr.address & 0xFF8;
  u32 dram_address = rsp.spDMADRAMAddr.address & 0xFFFFF8;

  for (int i = 0; i < len.count + 1; i++) {
    for(int j = 0; j < length; j++) {
      if constexpr (isDRAMdest) {
        dst[dram_address + j] = src[(mem_address + j) & 0xFFF];
      } else {
        dst[(mem_address + j) & 0xFFF] = src[dram_address + j];
      }
    }

    int skip = i == len.count ? 0 : len.skip;

    dram_address += (length + skip) & 0xFFFFF8;
    mem_address += length;
  }

  rsp.lastSuccessfulSPAddr.address = mem_address & 0xFF8;
  rsp.lastSuccessfulSPAddr.bank = bank;
  rsp.lastSuccessfulDRAMAddr.address = dram_address & 0xFFFFF8;
}

void RSP::Write(Mem& mem, Registers& regs, u32 addr, u32 value) {
  MI& mi = mem.mmio.mi;
  switch (addr) {
    case 0x04040000: spDMASPAddr.raw = value & 0x1FF8; break;
    case 0x04040004: spDMADRAMAddr.raw = value & 0xFFFFF8; break;
    case 0x04040008: {
      spDMALen.raw = value;
      DMA<false>(spDMALen, mem, *this, spDMASPAddr.bank);
      spDMALen.raw = 0xFF8 | (spDMALen.skip << 20);
    } break;
    case 0x0404000C: {
      spDMALen.raw = value;
      DMA<true>(spDMALen, mem, *this, spDMASPAddr.bank);
      spDMALen.raw = 0xFF8 | (spDMALen.skip << 20);
    } break;
    case 0x04040010: {
      auto write = SPStatusWrite{.raw = value};
      CLEAR_SET(spStatus.halt, write.clearHalt, write.setHalt);
      if(write.clearBroke) spStatus.broke = false;
      if(write.clearIntr && !write.setIntr)
        InterruptLower(mi, regs, Interrupt::SP);
      if(write.setIntr && !write.clearIntr)
        InterruptRaise(mi, regs, Interrupt::SP);
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
    case 0x0404001C: ReleaseSemaphore(); break;
    case 0x04080000:
      if(spStatus.halt) {
        oldPC = pc & 0xFFC;
        pc = value & 0xFFC;
        nextPC = value & 0xFFC;
      } break;
    default:
      util::panic("Unimplemented SP register write {:08X}, val: {:08X}\n", addr, value);
  }
}

}
