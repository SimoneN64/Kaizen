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
  case BEQL:
  case BNEL:
  case BLEZL:
  case BGTZL:
    return true;
  default:
    return false;
  }
}
} // namespace n64
