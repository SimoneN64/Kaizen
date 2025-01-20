#pragma once
#include <CpuDefinitions.hpp>

namespace n64 {
static bool SpecialEndsBlock(const u32 instr) {
  switch (instr & 0x3F) {
  case JR:
  case JALR:
  case SYSCALL:
  case BREAK:
  case TGE:
  case TGEU:
  case TLT:
  case TLTU:
  case TEQ:
  case TNE:
    return true;
  default:
    return false;
  }
}

static bool InstrEndsBlock(const u32 instr) {
  switch (instr >> 26 & 0x3f) {
  case SPECIAL:
    return SpecialEndsBlock(instr);
  case REGIMM:
  case J:
  case JAL:
  case BEQ:
  case BNE:
  case BLEZ:
  case BGTZ:
    return true;
  default:
    return false;
  }
}

static bool IsBranchLikely(const u32 instr) {
  switch (instr >> 26 & 0x1F) {
  case BEQL:
  case BNEL:
  case BLEZL:
  case BGTZL:
    return true;
  case COP1:
    {
      const u8 mask_sub = (instr >> 21) & 0x1F;
      const u8 mask_branch = (instr >> 16) & 0x1F;
      if (mask_sub == 0x08) {
        if (mask_branch == 2 || mask_branch == 3)
          return true;

        return false;
      }

      return false;
    }
  default:
    switch (instr >> 16 & 0x1F) {
    case BLTZL:
    case BGEZL:
    case BLTZALL:
    case BGEZALL:
      return true;
    default:
      return false;
    }
  }
}
} // namespace n64
