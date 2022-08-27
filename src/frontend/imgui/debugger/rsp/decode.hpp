#pragma once
#include <debugger_common.hpp>

inline std::string rspSpecial(u32 instr) {
  u8 mask = instr & 0x3f;
  u8 sa = (instr >> 6) & 0x1f;
  switch(mask) {
    case 0x00: return fmt::format("[rsp]: sll {}, {}, {:02X}", gprStr[RD(instr)], gprStr[RT(instr)], sa);
    case 0x04: return fmt::format("[rsp]: sllv {}, {}, {}", gprStr[RD(instr)], gprStr[RT(instr)], gprStr[RS(instr)]);
    case 0x08: return fmt::format("[rsp]: jr {}", gprStr[RS(instr)]);
    case 0x0C: case 0x0D:
      return fmt::format("[rsp]: break");
    case 0x20: case 0x21:
      return fmt::format("[rsp]: add {}, {}, {}", gprStr[RD(instr)], gprStr[RS(instr)], gprStr[RT(instr)]);
    case 0x24: return fmt::format("[rsp]: and {}, {}, {}", gprStr[RD(instr)], gprStr[RS(instr)], gprStr[RT(instr)]);
    case 0x25: return fmt::format("[rsp]: or {}, {}, {}", gprStr[RD(instr)], gprStr[RS(instr)], gprStr[RT(instr)]);
    case 0x27: return fmt::format("[rsp]: nor {}, {}, {}", gprStr[RD(instr)], gprStr[RS(instr)], gprStr[RT(instr)]);
    default: return fmt::format("INVALID ({:08X})", instr);
  }
}

inline std::string rspRegimm(u32 instr) {
  u8 mask = ((instr >> 16) & 0x1F);
  switch(mask) {
    case 0x00: return fmt::format("[rsp]: bltz {}, {:04X}", gprStr[RS(instr)], instr & 0xffff);
    case 0x01: return fmt::format("[rsp]: bgez {}, {:04X}", gprStr[RS(instr)], instr & 0xffff);
    default: return fmt::format("INVALID ({:08X})", instr);
  }
}

inline std::string lwc2(u32 instr) {
  u8 mask = (instr >> 11) & 0x1F;
  switch(mask) {
    case 0x04: return fmt::format("[rsp]: lqv {}[{}], {:02X}({})", vprStr[VT(instr)], vprByteStr[BYTE_INDEX(E(instr))], instr & 0x7F, gprStr[BASE(instr)]);
    default: return fmt::format("INVALID ({:08X})", instr);
  }
}

inline std::string swc2(u32 instr) {
  u8 mask = (instr >> 11) & 0x1F;
  switch(mask) {
    case 0x04: return fmt::format("[rsp]: sqv {}[{}], {:02X}({})", vprStr[VT(instr)], vprByteStr[BYTE_INDEX(E(instr))], instr & 0x7F, gprStr[BASE(instr)]);
    default: return fmt::format("INVALID ({:08X})", instr);
  }
}

inline std::string cop2decode(u32 instr) {
  u8 mask = instr & 0x3F;
  u8 mask_sub = (instr >> 21) & 0x1F;
  switch(mask) {
    case 0x00:
      switch(mask_sub) {
        case 0x02: return fmt::format("[rsp]: cfc2 {}, {}", gprStr[RT(instr)], rspControlStr[RD(instr) & 3]);
        default: return fmt::format("INVALID ({:08X})", instr);
      }
      break;
      case 0x13: return fmt::format("[rsp]: vabs {}, {}, {}", vprStr[VD(instr)], vprStr[VS(instr)], vprStr[VT(instr)]);
      case 0x1D: return fmt::format("[rsp]: vsar {}, {}, {}", vprStr[VD(instr)], vprStr[VS(instr)], vprStr[VT(instr)]);
      case 0x21: return fmt::format("[rsp]: veq {}, {}, {}", vprStr[VD(instr)], vprStr[VS(instr)], vprStr[VT(instr)]);
      case 0x22: return fmt::format("[rsp]: vne {}, {}, {}", vprStr[VD(instr)], vprStr[VS(instr)], vprStr[VT(instr)]);
      case 0x33: return fmt::format("[rsp]: vmov {}[{}], {}[{}]", vprStr[VD(instr)], vprElementStr[DE(instr)], vprStr[VT(instr)], vprElementStr[E(instr)]);
    default: return fmt::format("INVALID ({:08X})", instr);
  }
}

inline std::string rspCop0Decode(u32 instr) {
  u8 mask = (instr >> 21) & 0x1F;
  switch(mask) {
    case 0x00: return fmt::format("[rsp]: mfc0 {}, {}", gprStr[RT(instr)], cop0Str[RD(instr)]);
    case 0x04: return fmt::format("[rsp]: mtc0 {}, {}", gprStr[RT(instr)], cop0Str[RD(instr)]);
    default: return fmt::format("INVALID ({:08X})", instr);
  }
}

inline std::string rspDecode(u32 instr) {
  u8 mask = (instr >> 26) & 0x3F;
  switch(mask) {
    case 0x00: return special(instr);
    case 0x01: return regimm(instr);
    case 0x02: return fmt::format("[rsp]: j {:04X}", instr & 0xffff);
    case 0x03: return fmt::format("[rsp]: jal {:04X}", instr & 0xffff);
    case 0x04: return fmt::format("[rsp]: beq {}, {}, {:04X}", gprStr[RT(instr)], gprStr[RS(instr)], instr & 0xffff);
    case 0x05: return fmt::format("[rsp]: bne {}, {}, {:04X}", gprStr[RT(instr)], gprStr[RS(instr)], instr & 0xffff);
    case 0x07: return fmt::format("[rsp]: bgtz {}, {:04X}", gprStr[RS(instr)], instr & 0xffff);
    case 0x08: case 0x09:
      return fmt::format("[rsp]: addi {}, {}, {:04X}", gprStr[RT(instr)], gprStr[RS(instr)], instr & 0xffff);
    case 0x0C: return fmt::format("[rsp]: andi {}, {}, {:04X}", instr);
    case 0x0D: return fmt::format("[rsp]: ori {}, {}, {:04X}", instr);
    case 0x0F: return fmt::format("[rsp]: lui {}, {:04X}", gprStr[RT(instr)], instr & 0xffff);
    case 0x10: return rspCop0Decode(instr);
    case 0x12: return cop2decode(instr);
    case 0x21: return fmt::format("[rsp]: lh {}, {:04X}({})", gprStr[RT(instr)], instr & 0xffff, gprStr[BASE(instr)]);
    case 0x23: return fmt::format("[rsp]: lw {}, {:04X}({})", gprStr[RT(instr)], instr & 0xffff, gprStr[BASE(instr)]);
    case 0x28: return fmt::format("[rsp]: sb {}, {:04X}({})", gprStr[RT(instr)], instr & 0xffff, gprStr[BASE(instr)]);
    case 0x29: return fmt::format("[rsp]: sh {}, {:04X}({})", gprStr[RT(instr)], instr & 0xffff, gprStr[BASE(instr)]);
    case 0x2B: return fmt::format("[rsp]: sw {}, {:04X}({})", gprStr[RT(instr)], instr & 0xffff, gprStr[BASE(instr)]);
    case 0x32: return lwc2(instr);
    case 0x3A: return swc2(instr);
    default: return fmt::format("INVALID ({:08X})", instr);
  }
}