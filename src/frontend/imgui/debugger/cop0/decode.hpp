#pragma once
#include <debugger_common.hpp>

inline std::string cop0decode(u32 instr) {
  u8 mask_cop = (instr >> 21) & 0x1F;
  u8 mask_cop2 = instr & 0x3F;
  switch(mask_cop) {
    case 0x00: return fmt::format("mfc0 {}, {}", gprStr[RT(instr)], cop0Str[RD(instr)]);
    case 0x01: return fmt::format("dmfc0 {}, {}", gprStr[RT(instr)], cop0Str[RD(instr)]);
    case 0x04: return fmt::format("mtc0 {}, {}", gprStr[RT(instr)], cop0Str[RD(instr)]);
    case 0x05: return fmt::format("dmtc0 {}, {}", gprStr[RT(instr)], cop0Str[RD(instr)]);
    case 0x10 ... 0x1F:
      switch(mask_cop2) {
        case 0x01: return fmt::format("tlbr");
        case 0x02: return fmt::format("tlbwi");
        case 0x08: return fmt::format("tlbp");
        case 0x18: return fmt::format("eret");
        default: return fmt::format("INVALID ({:08X})", instr);
      }
    default: return fmt::format("INVALID ({:08X})", instr);
  }
}