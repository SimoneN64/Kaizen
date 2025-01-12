#include <registers/Cop0.hpp>
#include <registers/Registers.hpp>
#include <log.hpp>
#include <ranges>

namespace n64 {
void Cop0::mtc0(const u32 instr) { SetReg32(RD(instr), regs.Read<u32>(RT(instr))); }

void Cop0::dmtc0(const u32 instr) { SetReg64(RD(instr), regs.Read<u64>(RT(instr))); }

void Cop0::mfc0(const u32 instr) { regs.Write(RT(instr), s32(GetReg32(RD(instr)))); }

void Cop0::dmfc0(const u32 instr) const { regs.Write(RT(instr), s64(GetReg64(RD(instr)))); }

void Cop0::eret() {
  if (status.erl) {
    regs.SetPC64(ErrorEPC);
    status.erl = false;
  } else {
    regs.SetPC64(EPC);
    status.exl = false;
  }
  regs.cop0.Update();
  llbit = false;
}


void Cop0::tlbr() {
  if (index.i >= 32) {
    Util::panic("TLBR with TLB index {}", index.i);
  }

  const TLBEntry entry = tlb[index.i];

  entryHi.raw = entry.entryHi.raw;
  entryLo0.raw = entry.entryLo0.raw & 0x3FFFFFFF;
  entryLo1.raw = entry.entryLo1.raw & 0x3FFFFFFF;

  entryLo0.g = entry.global;
  entryLo1.g = entry.global;
  pageMask.raw = entry.pageMask.raw;
}

void Cop0::tlbw(const int index_) {
  PageMask page_mask{};
  page_mask = pageMask;
  const u32 top = page_mask.mask & 0xAAA;
  page_mask.mask = top | (top >> 1);

  if (index_ >= 32) {
    Util::panic("TLBWI with TLB index {}", index_);
  }

  tlb[index_].entryHi.raw = entryHi.raw;
  tlb[index_].entryHi.vpn2 &= ~page_mask.mask;

  tlb[index_].entryLo0.raw = entryLo0.raw & 0x03FFFFFE;
  tlb[index_].entryLo1.raw = entryLo1.raw & 0x03FFFFFE;
  tlb[index_].pageMask.raw = page_mask.raw;

  tlb[index_].global = entryLo0.g && entryLo1.g;
  tlb[index_].initialized = true;
}

void Cop0::tlbp() {
  int match = -1;
  if (const TLBEntry *entry = TLBTryMatch(entryHi.raw, match); entry && match >= 0) {
    index.raw = match;
  } else {
    index.raw = 0;
    index.p = 1;
  }
}

} // namespace n64
