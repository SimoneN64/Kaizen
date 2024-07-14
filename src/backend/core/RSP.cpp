#include <core/RSP.hpp>
#include <log.hpp>
#include <core/Mem.hpp>
#include <core/registers/Registers.hpp>

namespace n64 {
RSP::RSP(Mem& mem, Registers& regs) : mem(mem), regs(regs) {
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
  dmem = {};
  imem = {};
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
  steps = 0;
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

auto RSP::Read(u32 addr) -> u32 {
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

void RSP::WriteStatus(u32 value) {
  MI& mi = mem.mmio.mi;
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
    mi.InterruptLower(MI::Interrupt::SP);
  if(write.setIntr && !write.clearIntr)
    mi.InterruptRaise(MI::Interrupt::SP);
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

template <> void RSP::DMA<true>() {
  u32 length = spDMALen.len + 1;

  length = (length + 0x7) & ~0x7;

  std::array<u8, DMEM_SIZE>& src = spDMASPAddr.bank ? imem : dmem;

  u32 mem_address = spDMASPAddr.address & 0xFF8;
  u32 dram_address = spDMADRAMAddr.address & 0xFFFFFC;
  Util::trace("SP DMA from RSP to RDRAM (size: {} B, {:08X} to {:08X})", length, mem_address, dram_address);

  for (u32 i = 0; i < spDMALen.count + 1; i++) {
    for(u32 j = 0; j < length; j++) {
      mem.mmio.rdp.WriteRDRAM<u8>(BYTE_ADDRESS(dram_address + j), src[(mem_address + j) & DMEM_DSIZE]);
    }

    int skip = i == spDMALen.count ? 0 : spDMALen.skip;

    dram_address += (length + skip);
    dram_address &= 0xFFFFFE;
    mem_address += length;
    mem_address &= 0xFF8;
  }
  Util::trace("Addresses after: RSP: 0x{:08X}, Dram: 0x{:08X}", mem_address, dram_address);

  lastSuccessfulSPAddr.address = mem_address;
  lastSuccessfulSPAddr.bank = spDMASPAddr.bank;
  lastSuccessfulDRAMAddr.address = dram_address;
  spDMALen.raw = 0xFF8 | (spDMALen.skip << 20);
}

template <> void RSP::DMA<false>() {
  u32 length = spDMALen.len + 1;

  length = (length + 0x7) & ~0x7;

  std::array<u8, DMEM_SIZE>& dst = spDMASPAddr.bank ? imem : dmem;

  u32 mem_address = spDMASPAddr.address & 0xFF8;
  u32 dram_address = spDMADRAMAddr.address & 0xFFFFFE;
  Util::trace("SP DMA from RDRAM to RSP (size: {} B, {:08X} to {:08X})", length, dram_address, mem_address);

  for (u32 i = 0; i < spDMALen.count + 1; i++) {
    for(u32 j = 0; j < length; j++) {
      dst[(mem_address + j) & DMEM_DSIZE] = mem.mmio.rdp.ReadRDRAM<u8>(BYTE_ADDRESS(dram_address + j));
    }

    int skip = i == spDMALen.count ? 0 : spDMALen.skip;

    dram_address += (length + skip);
    dram_address &= 0xFFFFFE;
    mem_address += length;
    mem_address &= 0xFF8;
  }
  Util::trace("Addresses after: RSP: 0x{:08X}, Dram: 0x{:08X}", mem_address, dram_address);

  lastSuccessfulSPAddr.address = mem_address;
  lastSuccessfulSPAddr.bank = spDMASPAddr.bank;
  lastSuccessfulDRAMAddr.address = dram_address;
  spDMALen.raw = 0xFF8 | (spDMALen.skip << 20);
}

void RSP::Write(u32 addr, u32 val) {
  switch (addr) {
    case 0x04040000: spDMASPAddr.raw = val & 0x1FF8; break;
    case 0x04040004: spDMADRAMAddr.raw = val & 0xFFFFFE; break;
    case 0x04040008: {
      spDMALen.raw = val;
      DMA<false>();
    } break;
    case 0x0404000C: {
      spDMALen.raw = val;
      DMA<true>();
    } break;
    case 0x04040010: WriteStatus(val); break;
    case 0x0404001C: ReleaseSemaphore(); break;
    case 0x04080000:
      if(spStatus.halt) {
        SetPC(val);
      } break;
    default:
      Util::panic("Unimplemented SP register write {:08X}, val: {:08X}", addr, val);
  }
}
}
