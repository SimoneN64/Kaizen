#pragma once
#include <util.hpp>
#include <map>
#include <functional>

struct Core;

const std::string gprStr[32] = {
  "zero", "at", "v0", "v1",
  "a0", "a1", "a2", "a3",
  "t0", "t1", "t2", "t3",
  "t4", "t5", "t6", "t7",
  "s0", "s1", "s2", "s3",
  "s4", "s5", "s6", "s7",
  "t8", "t9", "k0", "k1",
  "gp", "sp", "s8", "ra"
};

std::string special(u32 instr) {
  u8 mask = (instr & 0x3F);
  u8 sa = (instr >> 6) & 0x1f;
  switch (mask) { // TODO: named constants for clearer code
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
      return fmt::format("INVALID ({:08X})\n", instr);
  }
}

std::string regimm(u32 instr) {
  u8 mask = ((instr >> 16) & 0x1F);
  // 000r_rccc
  switch (mask) { // TODO: named constants for clearer code
    case 0x00: return fmt::format("bltz {}", gprStr[RS(instr)]);
    case 0x01: return fmt::format("bgez {}", gprStr[RS(instr)]);
    case 0x02: return fmt::format("bltzl {}", gprStr[RS(instr)]);
    case 0x03: return fmt::format("bgezl {}", gprStr[RS(instr)]);
    case 0x10: return fmt::format("bltzal {}", gprStr[RS(instr)]);
    case 0x11: return fmt::format("bgezal {}", gprStr[RS(instr)]);
    case 0x12: return fmt::format("bltzall {}", gprStr[RS(instr)]);
    case 0x13: return fmt::format("bgezall {}", gprStr[RS(instr)]);
    default:   return fmt::format("INVALID {:08X}", instr);
  }
}

const std::map<u8, std::function<std::string(u32)>> cpuInstr = {
  {0b000000, special},
  {0b000001, regimm},
  {0b000010, [](u32 instr) {
    return fmt::format("j {:08X}", instr & 0x3FFFFFF);
  }}, {0b000011, [](u32 instr) {
    return fmt::format("jal {:08X}", instr & 0x3FFFFFF);
  }}, {0b000100, [](u32 instr) {
    return fmt::format("beq {}, {}", gprStr[RS(instr)], gprStr[RT(instr)]);
  }}, {0b000101, [](u32 instr) {
    return fmt::format("bne {}, {}", gprStr[RS(instr)], gprStr[RT(instr)]);
  }}, {0b000110, [](u32 instr) {
    return fmt::format("blez {}", gprStr[RS(instr)]);
  }}, {0b000111, [](u32 instr) {
    return fmt::format("bgtz {}", gprStr[RS(instr)]);
  }}, {0b001000, [](u32 instr) {
    return fmt::format("addi {}, {:04X}", gprStr[RS(instr)], instr & 0xFFFF);
  }}, {0b001001, [](u32 instr) {
    return fmt::format("addiu {}, {:04X}", gprStr[RS(instr)], instr & 0x3FFFFFF);
  }}, {0b001010, [](u32 instr) {
    return fmt::format("slti {}, {}, {:04X}", gprStr[RT(instr)], gprStr[RS(instr)], instr & 0xFFFF);
  }}, {0b001011, [](u32 instr) {
    return fmt::format("sltiu {}, {}, {:04X}", gprStr[RT(instr)], gprStr[RS(instr)], instr & 0xFFFF);
  }}, {0b001100, [](u32 instr) {
    return fmt::format("andi {}, {}", gprStr[RS(instr)], gprStr[RT(instr)]);
  }}, {0b001101, [](u32 instr) {
    return fmt::format("ori {}, {}", gprStr[RS(instr)], gprStr[RT(instr)]);
  }}, {0b001110, [](u32 instr) {
    return fmt::format("xori {}", gprStr[RS(instr)]);
  }}, {0b001111, [](u32 instr) {
    return fmt::format("lui {}", gprStr[RS(instr)]);
  }}, {0b010000, /*cop0decode*/ },
      {0b010001, [](u32 instr) {
    return fmt::format("j {:08X}", instr & 0x3FFFFFF);
  }}, {0b010010, [](u32 instr) {
    return fmt::format("j {:08X}", instr & 0x3FFFFFF);
  }}, {0b010011, [](u32 instr) {
    return fmt::format("jal {:08X}", instr & 0x3FFFFFF);
  }}, {0b010100, [](u32 instr) {
    return fmt::format("beq {}, {}", gprStr[RS(instr)], gprStr[RT(instr)]);
  }}, {0b010101, [](u32 instr) {
    return fmt::format("bne {}, {}", gprStr[RS(instr)], gprStr[RT(instr)]);
  }}, {0b010110, [](u32 instr) {
    return fmt::format("blez {}", gprStr[RS(instr)]);
  }}, {0b010111, [](u32 instr) {
    return fmt::format("bgtz {}", gprStr[RS(instr)]);
  }}, {0b011000, [](u32 instr) {
    return fmt::format("j {:08X}", instr & 0x3FFFFFF);
  }}, {0b011001, [](u32 instr) {
    return fmt::format("j {:08X}", instr & 0x3FFFFFF);
  }}, {0b011010, [](u32 instr) {
    return fmt::format("j {:08X}", instr & 0x3FFFFFF);
  }}, {0b011011, [](u32 instr) {
    return fmt::format("jal {:08X}", instr & 0x3FFFFFF);
  }}, {0b011100, [](u32 instr) {
    return fmt::format("beq {}, {}", gprStr[RS(instr)], gprStr[RT(instr)]);
  }}, {0b011101, [](u32 instr) {
    return fmt::format("bne {}, {}", gprStr[RS(instr)], gprStr[RT(instr)]);
  }}, {0b011110, [](u32 instr) {
    return fmt::format("blez {}", gprStr[RS(instr)]);
  }}, {0b011111, [](u32 instr) {
    return fmt::format("bgtz {}", gprStr[RS(instr)]);
  }}, {0b100000, [](u32 instr) {
    return fmt::format("j {:08X}", instr & 0x3FFFFFF);
  }}, {0b100001, [](u32 instr) {
    return fmt::format("j {:08X}", instr & 0x3FFFFFF);
  }}, {0b100010, [](u32 instr) {
    return fmt::format("j {:08X}", instr & 0x3FFFFFF);
  }}, {0b100011, [](u32 instr) {
    return fmt::format("jal {:08X}", instr & 0x3FFFFFF);
  }}, {0b100100, [](u32 instr) {
    return fmt::format("beq {}, {}", gprStr[RS(instr)], gprStr[RT(instr)]);
  }}, {0b100101, [](u32 instr) {
    return fmt::format("bne {}, {}", gprStr[RS(instr)], gprStr[RT(instr)]);
  }}, {0b100110, [](u32 instr) {
    return fmt::format("blez {}", gprStr[RS(instr)]);
  }}, {0b100111, [](u32 instr) {
    return fmt::format("bgtz {}", gprStr[RS(instr)]);
  }}, {0b101000, [](u32 instr) {
    return fmt::format("j {:08X}", instr & 0x3FFFFFF);
  }}, {0b101001, [](u32 instr) {
    return fmt::format("j {:08X}", instr & 0x3FFFFFF);
  }}, {0b101010, [](u32 instr) {
    return fmt::format("j {:08X}", instr & 0x3FFFFFF);
  }}, {0b101011, [](u32 instr) {
    return fmt::format("jal {:08X}", instr & 0x3FFFFFF);
  }}, {0b101100, [](u32 instr) {
    return fmt::format("beq {}, {}", gprStr[RS(instr)], gprStr[RT(instr)]);
  }}, {0b101101, [](u32 instr) {
    return fmt::format("bne {}, {}", gprStr[RS(instr)], gprStr[RT(instr)]);
  }}, {0b101110, [](u32 instr) {
    return fmt::format("blez {}", gprStr[RS(instr)]);
  }}, {0b101111, [](u32 instr) {
    return fmt::format("bgtz {}", gprStr[RS(instr)]);
  }}, {0b110000, [](u32 instr) {
    return fmt::format("j {:08X}", instr & 0x3FFFFFF);
  }}, {0b110001, [](u32 instr) {
    return fmt::format("j {:08X}", instr & 0x3FFFFFF);
  }}, {0b110010, [](u32 instr) {
    return fmt::format("j {:08X}", instr & 0x3FFFFFF);
  }}, {0b110011, [](u32 instr) {
    return fmt::format("jal {:08X}", instr & 0x3FFFFFF);
  }}, {0b110100, [](u32 instr) {
    return fmt::format("beq {}, {}", gprStr[RS(instr)], gprStr[RT(instr)]);
  }}, {0b110101, [](u32 instr) {
    return fmt::format("bne {}, {}", gprStr[RS(instr)], gprStr[RT(instr)]);
  }}, {0b110110, [](u32 instr) {
    return fmt::format("blez {}", gprStr[RS(instr)]);
  }}, {0b110111, [](u32 instr) {
    return fmt::format("bgtz {}", gprStr[RS(instr)]);
  }}, {0b111000, [](u32 instr) {
    return fmt::format("j {:08X}", instr & 0x3FFFFFF);
  }}, {0b111001, [](u32 instr) {
    return fmt::format("j {:08X}", instr & 0x3FFFFFF);
  }}, {0b111010, [](u32 instr) {
    return fmt::format("j {:08X}", instr & 0x3FFFFFF);
  }}, {0b111011, [](u32 instr) {
    return fmt::format("jal {:08X}", instr & 0x3FFFFFF);
  }}, {0b111100, [](u32 instr) {
    return fmt::format("beq {}, {}", gprStr[RS(instr)], gprStr[RT(instr)]);
  }}, {0b111101, [](u32 instr) {
    return fmt::format("bne {}, {}", gprStr[RS(instr)], gprStr[RT(instr)]);
  }}, {0b111110, [](u32 instr) {
    return fmt::format("blez {}", gprStr[RS(instr)]);
  }}, {0b111111, [](u32 instr) {
    return fmt::format("bgtz {}", gprStr[RS(instr)]);
  }}
};

struct Debugger {

};