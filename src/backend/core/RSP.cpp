#include <core/RSP.hpp>
#include <log.hpp>
#include <core/Mem.hpp>
#include <core/registers/Registers.hpp>

namespace n64 {
RSP::RSP() {
  Reset();
}

void RSP::Reset() {
  lastSuccessfulSPAddr.raw = 0;
  lastSuccessfulDRAMAddr.raw = 0;
  spStatus.raw = 0;
  spStatus.halt = true;
  oldPC = 0;
  pc = 0;
  nextPC = 4;
  spDMASPAddr.raw = 0;
  spDMADRAMAddr.raw = 0;
  spDMALen.raw = 0;
  memset(dmem, 0, DMEM_SIZE);
  memset(imem, 0, IMEM_SIZE);
  memset(vpr, 0, 32 * sizeof(VPR));
  memset(gpr, 0, 32 * sizeof(u32));
  memset(&vce, 0, sizeof(VPR));
  memset(&acc, 0, 3 * sizeof(VPR));
  memset(&vcc, 0, 2 * sizeof(VPR));
  memset(&vco, 0, 2 * sizeof(VPR));
  semaphore = false;
  divIn = 0;
  divOut = 0;
  divInLoaded = false;
}

/*
FORCE_INLINE void logRSP(const RSP& rsp, const u32 instr) {
  Util::debug("{:04X} {:08X} ", rsp.oldPC, instr);
  for (auto gpr : rsp.gpr) {
    Util::debug("{:08X} ", gpr);
  }

  for (auto vpr : rsp.vpr) {
    for (int i = 0; i < 8; i++) {
      Util::debug("{:04X}", vpr.element[i]);
    }
    Util::debug(" ");
  }

  for (int i = 0; i < 8; i++) {
    Util::debug("{:04X}", rsp.acc.h.element[i]);
  }
  Util::debug(" ");

  for (int i = 0; i < 8; i++) {
    Util::debug("{:04X}", rsp.acc.m.element[i]);
  }
  Util::debug(" ");

  for (int i = 0; i < 8; i++) {
    Util::debug("{:04X}", rsp.acc.l.element[i]);
  }

  Util::debug(" {:04X} {:04X} {:02X}", rsp.GetVCC(), rsp.GetVCO(), rsp.GetVCE());
  Util::debug("DMEM: {:02X}{:02X}", rsp.dmem[0x3c4], rsp.dmem[0x3c5]);
}
*/

auto RSP::Read(u32 addr) -> u32{
  switch (addr) {
    case 0x04040000: return lastSuccessfulSPAddr.raw & 0x1FF8;
    case 0x04040004: return lastSuccessfulDRAMAddr.raw & 0xFFFFF8;
    case 0x04040008:
    case 0x0404000C: return spDMALen.raw;
    case 0x04040010: return spStatus.raw;
    case 0x04040014: return spStatus.dmaFull;
    case 0x04040018: return 0;
    case 0x0404001C:
      return AcquireSemaphore();
    case 0x04080000: return pc & 0xFFC;
    default:
      Util::panic("Unimplemented SP register read {:08X}", addr);
  }
}

void RSP::WriteStatus(MI& mi, Registers& regs, u32 value) {
  auto write = SPStatusWrite{.raw = value};
  if(write.clearHalt && !write.setHalt) {
    spStatus.halt = false;
  }
  if(write.setHalt && !write.clearHalt) {
    regs.steps = 0;
    spStatus.halt = true;
  }
  if(write.clearBroke) spStatus.broke = false;
  if(write.clearIntr && !write.setIntr)
    InterruptLower(mi, regs, Interrupt::SP);
  if(write.setIntr && !write.clearIntr)
    InterruptRaise(mi, regs, Interrupt::SP);
  CLEAR_SET(spStatus.singleStep, write.clearSstep, write.setSstep);
  CLEAR_SET(spStatus.interruptOnBreak, write.clearIntrOnBreak, write.setIntrOnBreak);
  CLEAR_SET(spStatus.signal0, write.clearSignal0, write.setSignal0);
  CLEAR_SET(spStatus.signal1, write.clearSignal1, write.setSignal1);
  CLEAR_SET(spStatus.signal2, write.clearSignal2, write.setSignal2);
  CLEAR_SET(spStatus.signal3, write.clearSignal3, write.setSignal3);
  CLEAR_SET(spStatus.signal4, write.clearSignal4, write.setSignal4);
  CLEAR_SET(spStatus.signal5, write.clearSignal5, write.setSignal5);
  CLEAR_SET(spStatus.signal6, write.clearSignal6, write.setSignal6);
  CLEAR_SET(spStatus.signal7, write.clearSignal7, write.setSignal7);
}

void RSP::Write(Mem& mem, Registers& regs, u32 addr, u32 value) {
  MI& mi = mem.mmio.mi;
  switch (addr) {
    case 0x04040000: spDMASPAddr.raw = value & 0x1FF8; break;
    case 0x04040004: spDMADRAMAddr.raw = value & 0xFFFFF8; break;
    case 0x04040008: {
      spDMALen.raw = value;
      DMA<false>(spDMALen, mem.GetRDRAM(), *this, spDMASPAddr.bank);
    } break;
    case 0x0404000C: {
      spDMALen.raw = value;
      DMA<true>(spDMALen, mem.GetRDRAM(), *this, spDMASPAddr.bank);
    } break;
    case 0x04040010: WriteStatus(mi, regs, value); break;
    case 0x0404001C: ReleaseSemaphore(); break;
    case 0x04080000:
      if(spStatus.halt) {
        SetPC(value);
      } break;
    default:
      Util::panic("Unimplemented SP register write {:08X}, val: {:08X}", addr, value);
  }
}
}
