#include <dynarec/cop/cop0instructions.hpp>
#include <log.hpp>
#include <Registers.hpp>

namespace n64::JIT {
void mtc0(n64::Registers& regs, u32 instr) {
  regs.cop0.SetReg32(RD(instr), regs.gpr[RT(instr)]);
}

void dmtc0(n64::Registers& regs, u32 instr) {
  regs.cop0.SetReg64(RD(instr), regs.gpr[RT(instr)]);
}

void mfc0(n64::Registers& regs, u32 instr) {
  regs.gpr[RT(instr)] = s32(regs.cop0.GetReg32(RD(instr)));
}

void dmfc0(n64::Registers& regs, u32 instr) {
  regs.gpr[RT(instr)] = s64(regs.cop0.GetReg64(RD(instr)));
}

void eret(n64::Registers& regs) {
  if(regs.cop0.status.erl) {
    regs.SetPC(regs.cop0.ErrorEPC);
    regs.cop0.status.erl = false;
  } else {
    regs.SetPC(regs.cop0.EPC);
    regs.cop0.status.exl = false;
  }
  regs.cop0.llbit = false;
}


void tlbr(n64::Registers& regs) {
  u8 Index = regs.cop0.index & 0b111111;
  if (Index >= 32) {
    Util::panic("TLBR with TLB index {}", Index);
  }

  TLBEntry entry = regs.cop0.tlb[Index];

  regs.cop0.entryHi.raw = entry.entryHi.raw;
  regs.cop0.entryLo0.raw = entry.entryLo0.raw & 0x3FFFFFFF;
  regs.cop0.entryLo1.raw = entry.entryLo1.raw & 0x3FFFFFFF;

  regs.cop0.entryLo0.g = entry.global;
  regs.cop0.entryLo1.g = entry.global;
  regs.cop0.pageMask.raw = entry.pageMask.raw;
}

void tlbw(n64::Registers& regs, int index_) {
  PageMask page_mask = regs.cop0.pageMask;
  u32 top = page_mask.mask & 0xAAA;
  page_mask.mask = top | (top >> 1);

  if(index_ >= 32) {
    Util::panic("TLBWI with TLB index {}", index_);
  }

  regs.cop0.tlb[index_].entryHi.raw  = regs.cop0.entryHi.raw;
  regs.cop0.tlb[index_].entryHi.vpn2 &= ~page_mask.mask;

  regs.cop0.tlb[index_].entryLo0.raw = regs.cop0.entryLo0.raw & 0x03FFFFFE;
  regs.cop0.tlb[index_].entryLo1.raw = regs.cop0.entryLo1.raw & 0x03FFFFFE;
  regs.cop0.tlb[index_].pageMask.raw = page_mask.raw;

  regs.cop0.tlb[index_].global = regs.cop0.entryLo0.g && regs.cop0.entryLo1.g;
  regs.cop0.tlb[index_].initialized = true;
}

void tlbp(n64::Registers& regs) {
  int match = -1;
  TLBEntry* entry = TLBTryMatch(regs, regs.cop0.entryHi.raw, &match);
  if(entry && match >= 0) {
    regs.cop0.index = match;
  } else {
    regs.cop0.index = 0x80000000;
  }
}

}