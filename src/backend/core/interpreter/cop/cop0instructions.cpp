#include <core/registers/Cop0.hpp>
#include <core/registers/Registers.hpp>
#include <log.hpp>

namespace n64 {
void Cop0::mtc0(Registers& regs, u32 instr) {
  SetReg32(RD(instr), regs.gpr[RT(instr)]);
}

void Cop0::dmtc0(Registers& regs, u32 instr) {
  SetReg64(RD(instr), regs.gpr[RT(instr)]);
}

void Cop0::mfc0(Registers& regs, u32 instr) {
  regs.gpr[RT(instr)] = s32(GetReg32(RD(instr)));
}

void Cop0::dmfc0(Registers& regs, u32 instr) {
  regs.gpr[RT(instr)] = s64(GetReg64(RD(instr)));
}

void Cop0::eret(Registers& regs) {
  if(status.erl) {
    regs.SetPC64(ErrorEPC);
    status.erl = false;
  } else {
    regs.SetPC64(EPC);
    status.exl = false;
  }
  llbit = false;
}


void Cop0::tlbr() {
  u8 Index = index & 0b111111;
  if (Index >= 32) {
    Util::panic("TLBR with TLB index {}", index);
  }

  TLBEntry entry = tlb[Index];

  entryHi.raw = entry.entryHi.raw;
  entryLo0.raw = entry.entryLo0.raw & 0x3FFFFFFF;
  entryLo1.raw = entry.entryLo1.raw & 0x3FFFFFFF;

  entryLo0.g = entry.global;
  entryLo1.g = entry.global;
  pageMask.raw = entry.pageMask.raw;
}

void Cop0::tlbw(int index) {
  PageMask page_mask{};
  page_mask = pageMask;
  u32 top = page_mask.mask & 0xAAA;
  page_mask.mask = top | (top >> 1);

  if(index >= 32) {
    Util::panic("TLBWI with TLB index {}", index);
  }

  tlb[index].entryHi.raw  = entryHi.raw;
  tlb[index].entryHi.vpn2 &= ~page_mask.mask;

  tlb[index].entryLo0.raw = entryLo0.raw & 0x03FFFFFE;
  tlb[index].entryLo1.raw = entryLo1.raw & 0x03FFFFFE;
  tlb[index].pageMask.raw = page_mask.raw;

  tlb[index].global = entryLo0.g && entryLo1.g;
  tlb[index].initialized = true;
}

void Cop0::tlbp(Registers& regs) {
  int match = -1;
  TLBEntry* entry = TLBTryMatch(regs, entryHi.raw, &match);
  if(entry && match >= 0) {
    index = match;
  } else {
    index = 0x80000000;
  }
}

}