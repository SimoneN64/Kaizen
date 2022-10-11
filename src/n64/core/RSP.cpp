#include <n64/core/RSP.hpp>
#include <util.hpp>
#include <n64/core/Mem.hpp>

namespace n64 {
RSP::RSP() {
  Reset();
}

void RSP::Reset() {
  spStatus.raw = 0x1;
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

inline void logRSP(const RSP& rsp, const u32 instr) {
  util::print("{:04X} {:08X} ", rsp.oldPC, instr);
  for (int i = 0; i < 32; i++) {
    util::print("{:08X} ", (u32)rsp.gpr[i]);
  }

  for (int i = 0; i < 32; i++) {
    for (int e = 0; e < 8; e++) {
      util::print("{:04X}", rsp.vpr[i].element[e]);
    }
    util::print(" ");
  }

  for (int e = 0; e < 8; e++) {
    util::print("{:04X}", rsp.acc.h.element[e]);
  }
  util::print(" ");

  for (int e = 0; e < 8; e++) {
    util::print("{:04X}", rsp.acc.m.element[e]);
  }
  util::print(" ");

  for (int e = 0; e < 8; e++) {
    util::print("{:04X}", rsp.acc.l.element[e]);
  }
  util::print(" ");

  util::print("{:04X} {:04X} {:02X}", rsp.GetVCC(), rsp.GetVCO(), rsp.GetVCE());

  util::print("\n");
}

void RSP::Step(Registers& regs, Mem& mem) {
  gpr[0] = 0;
  u32 instr = util::ReadAccess<u32>(imem, pc & IMEM_DSIZE);
  oldPC = pc & 0xFFC;
  pc = nextPC & 0xFFC;
  nextPC += 4;
  if(oldPC == 0x00E4 && gpr[1] == 0x18) {
    printf("\n");
  }
  Exec(regs, mem, instr);

  // logRSP(*this, instr);
}

template <bool crashOnUnimplemented>
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
    default:
      if constexpr (crashOnUnimplemented) {
        util::panic("Unimplemented SP register read {:08X}\n", addr);
      }
      return 0;
  }
}

template auto RSP::Read<true>(u32 addr) -> u32;
template auto RSP::Read<false>(u32 addr) -> u32;

template <bool crashOnUnimplemented>
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
      if constexpr (crashOnUnimplemented) {
        util::panic("Unimplemented SP register write {:08X}, val: {:08X}\n", addr, value);
      }
  }
}

template void RSP::Write<true>(n64::Mem &mem, n64::Registers &regs, u32 addr, u32 value);
template void RSP::Write<false>(n64::Mem &mem, n64::Registers &regs, u32 addr, u32 value);
}
