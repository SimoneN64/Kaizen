#include <CpuDefinitions.hpp>

namespace n64 {
static inline bool SpecialEndsBlock(u32 instr) {
  u8 mask = instr & 0x3F;
  switch (mask) {
  case JR: case JALR: case SYSCALL: case BREAK:
  case TGE: case TGEU: case TLT: case TLTU: 
  case TEQ: case TNE: return true;
  default: return false;
  }
}

static inline bool InstrEndsBlock(u32 instr) {
  u8 mask = (instr >> 26) & 0x3f;
  switch (mask) {
  case SPECIAL: return SpecialEndsBlock(instr);
  case REGIMM: case J: case JAL: case BEQ:
  case BNE: case BLEZ: case BGTZ: case BEQL:
  case BNEL: case BLEZL: case BGTZL: return true;
  default: return false;
  }
}
}