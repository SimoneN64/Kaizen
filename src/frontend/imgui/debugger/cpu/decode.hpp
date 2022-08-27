#pragma once
#include <debugger_common.hpp>

inline std::string special(u32 instr) {
  u8 mask = (instr & 0x3F);
  u8 sa = (instr >> 6) & 0x1f;
  switch (mask) {
    case 0:
      if (mask != 0) {
        return fmt::format("sll {}, {}, {:02X}", gprStr[RD(instr)], gprStr[RT(instr)], sa);
      }
      return "nop";
    case 0x02: return fmt::format("srl {}, {}, {:02X}", gprStr[RD(instr)], gprStr[RT(instr)], sa);
    case 0x03: return fmt::format("sra {}, {}, {:02X}", gprStr[RD(instr)], gprStr[RT(instr)], sa);
    case 0x04: return fmt::format("sllv {}, {}, {}", gprStr[RD(instr)], gprStr[RT(instr)], gprStr[RS(instr)]);
    case 0x06: return fmt::format("srlv {}, {}, {}", gprStr[RD(instr)], gprStr[RT(instr)], gprStr[RS(instr)]);
    case 0x07: return fmt::format("srav {}, {}, {}", gprStr[RD(instr)], gprStr[RT(instr)], gprStr[RS(instr)]);
    case 0x08: return fmt::format("jr {}", gprStr[RS(instr)]);
    case 0x09: return fmt::format("jalr {}, {}", gprStr[RD(instr)], gprStr[RS(instr)]);
    case 0x0C: return fmt::format("syscall");
    case 0x0D: return fmt::format("break");
    case 0x0F: return fmt::format("sync");
    case 0x10: return fmt::format("mfhi {}", gprStr[RD(instr)]);
    case 0x11: return fmt::format("mthi {}", gprStr[RS(instr)]);
    case 0x12: return fmt::format("mflo {}", gprStr[RD(instr)]);
    case 0x13: return fmt::format("mtlo {}", gprStr[RS(instr)]);
    case 0x14: return fmt::format("dsllv {}, {}, {}", gprStr[RD(instr)], gprStr[RT(instr)], gprStr[RS(instr)]);
    case 0x16: return fmt::format("dsrlv {}, {}, {}", gprStr[RD(instr)], gprStr[RT(instr)], gprStr[RS(instr)]);
    case 0x17: return fmt::format("dsrav {}, {}, {}", gprStr[RD(instr)], gprStr[RT(instr)], gprStr[RS(instr)]);
    case 0x18: return fmt::format("mult {}, {}", gprStr[RS(instr)], gprStr[RT(instr)]);
    case 0x19: return fmt::format("multu {}, {}", gprStr[RS(instr)], gprStr[RT(instr)]);
    case 0x1A: return fmt::format("div {}, {}", gprStr[RS(instr)], gprStr[RT(instr)]);
    case 0x1B: return fmt::format("divu {}, {}", gprStr[RS(instr)], gprStr[RT(instr)]);
    case 0x1C: return fmt::format("dmult {}, {}", gprStr[RS(instr)], gprStr[RT(instr)]);
    case 0x1D: return fmt::format("dmultu {}, {}", gprStr[RS(instr)], gprStr[RT(instr)]);
    case 0x1E: return fmt::format("ddiv {}, {}", gprStr[RS(instr)], gprStr[RT(instr)]);
    case 0x1F: return fmt::format("ddivu {}, {}", gprStr[RS(instr)], gprStr[RT(instr)]);
    case 0x20: return fmt::format("add {}, {}, {}", gprStr[RD(instr)], gprStr[RS(instr)], gprStr[RT(instr)]);
    case 0x21: return fmt::format("addu {}, {}, {}", gprStr[RD(instr)], gprStr[RS(instr)], gprStr[RT(instr)]);
    case 0x22: return fmt::format("sub {}, {}, {}", gprStr[RD(instr)], gprStr[RS(instr)], gprStr[RT(instr)]);
    case 0x23: return fmt::format("subu {}, {}, {}", gprStr[RD(instr)], gprStr[RS(instr)], gprStr[RT(instr)]);
    case 0x24: return fmt::format("and {}, {}, {}", gprStr[RD(instr)], gprStr[RS(instr)], gprStr[RT(instr)]);
    case 0x25: return fmt::format("or {}, {}, {}", gprStr[RD(instr)], gprStr[RS(instr)], gprStr[RT(instr)]);
    case 0x26: return fmt::format("xor {}, {}, {}", gprStr[RD(instr)], gprStr[RS(instr)], gprStr[RT(instr)]);
    case 0x27: return fmt::format("nor {}, {}, {}", gprStr[RD(instr)], gprStr[RS(instr)], gprStr[RT(instr)]);
    case 0x2A: return fmt::format("slt {}, {}, {}", gprStr[RD(instr)], gprStr[RS(instr)], gprStr[RT(instr)]);
    case 0x2B: return fmt::format("sltu {}, {}, {}", gprStr[RD(instr)], gprStr[RS(instr)], gprStr[RT(instr)]);
    case 0x2C: return fmt::format("dadd {}, {}, {}", gprStr[RD(instr)], gprStr[RS(instr)], gprStr[RT(instr)]);
    case 0x2D: return fmt::format("daddu {}, {}, {}", gprStr[RD(instr)], gprStr[RS(instr)], gprStr[RT(instr)]);
    case 0x2E: return fmt::format("dsub {}, {}, {}", gprStr[RD(instr)], gprStr[RS(instr)], gprStr[RT(instr)]);
    case 0x2F: return fmt::format("dsubu {}, {}, {}", gprStr[RD(instr)], gprStr[RS(instr)], gprStr[RT(instr)]);
    case 0x34: return fmt::format("teq {}, {}", gprStr[RS(instr)], gprStr[RT(instr)]);
    case 0x38: return fmt::format("dsll {}, {}, {:02X}", gprStr[RD(instr)], gprStr[RS(instr)], sa);
    case 0x3A: return fmt::format("dsrl {}, {}, {:02X}", gprStr[RD(instr)], gprStr[RS(instr)], sa);
    case 0x3B: return fmt::format("dsra {}, {}, {:02X}", gprStr[RD(instr)], gprStr[RS(instr)], sa);
    case 0x3C: return fmt::format("dsll32 {}, {}, {:02X}", gprStr[RD(instr)], gprStr[RS(instr)], sa);
    case 0x3E: return fmt::format("dsrl32 {}, {}, {:02X}", gprStr[RD(instr)], gprStr[RS(instr)], sa);
    case 0x3F: return fmt::format("dsra32 {}, {}, {:02X}", gprStr[RD(instr)], gprStr[RS(instr)], sa);
    default:
      return fmt::format("INVALID ({:08X})", instr);
  }
}

inline std::string regimm(u32 instr) {
  u8 mask = ((instr >> 16) & 0x1F);
  // 000r_rccc
  switch (mask) { // TODO: named constants for clearer code
    case 0x00: return fmt::format("bltz {}, {:04X}", gprStr[RS(instr)], instr & 0xffff);
    case 0x01: return fmt::format("bgez {}, {:04X}", gprStr[RS(instr)], instr & 0xffff);
    case 0x02: return fmt::format("bltzl {}, {:04X}", gprStr[RS(instr)], instr & 0xffff);
    case 0x03: return fmt::format("bgezl {}, {:04X}", gprStr[RS(instr)], instr & 0xffff);
    case 0x10: return fmt::format("bltzal {}, {:04X}", gprStr[RS(instr)], instr & 0xffff);
    case 0x11: return fmt::format("bgezal {}, {:04X}", gprStr[RS(instr)], instr & 0xffff);
    case 0x12: return fmt::format("bltzall {}, {:04X}", gprStr[RS(instr)], instr & 0xffff);
    case 0x13: return fmt::format("bgezall {}, {:04X}", gprStr[RS(instr)], instr & 0xffff);
    default:   return fmt::format("INVALID ({:08X})", instr);
  }
}