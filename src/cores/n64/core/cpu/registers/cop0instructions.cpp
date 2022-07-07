#include <n64/core/cpu/registers/Cop0.hpp>
#include <n64/core/cpu/Registers.hpp>
#include <util.hpp>

namespace n64 {
void Cop0::mtc0(Registers& regs, u32 instr) {
  SetReg<u32>(RD(instr), regs.gpr[RT(instr)]);
}

void Cop0::dmtc0(Registers& regs, u32 instr) {
  SetReg(RD(instr), regs.gpr[RT(instr)]);
}

void Cop0::mfc0(Registers& regs, u32 instr) {
  regs.gpr[RT(instr)] = GetReg<s32>(RD(instr));
}

void Cop0::dmfc0(Registers& regs, u32 instr) {
  regs.gpr[RT(instr)] = GetReg<s64>(RD(instr));
}

void Cop0::eret(Registers& regs) {
  if(status.erl) {
    regs.SetPC((s64)ErrorEPC);
    status.erl = false;
  } else {
    regs.SetPC((s64)EPC);
    status.exl = false;
  }
}


void Cop0::tlbr(Registers& regs) {
  int Index = index & 0b111111;
  if (Index >= 32) {
    util::panic("TLBR with TLB index {}", index);
  }

  TLBEntry entry = tlb[Index];

  entryHi.raw = entry.entryHi.raw;
  entryLo0.raw = entry.entryLo0.raw & 0x3FFFFFFF;
  entryLo1.raw = entry.entryLo1.raw & 0x3FFFFFFF;

  entryLo0.g = entry.global;
  entryLo1.g = entry.global;
  pageMask.raw = entry.pageMask.raw;
}

void Cop0::tlbwi(Registers& regs) {
  PageMask page_mask;
  page_mask = pageMask;
  u32 top = page_mask.mask & 0xAAA;
  page_mask.mask = top | (top >> 1);

  int Index = index & 0x3F;
  if(Index >= 32) {
    util::panic("TLBWI with TLB index {}", Index);
  }

  tlb[Index].entryHi.raw  = entryHi.raw;
  tlb[Index].entryHi.vpn2 &= ~page_mask.mask;

  tlb[Index].entryLo0.raw = entryLo0.raw & 0x03FFFFFE;
  tlb[Index].entryLo1.raw = entryLo1.raw & 0x03FFFFFE;
  tlb[Index].pageMask.raw = page_mask.raw;

  tlb[Index].global = entryLo0.g && entryLo1.g;
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