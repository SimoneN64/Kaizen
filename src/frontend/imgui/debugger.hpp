#pragma once
#include <util.hpp>
#include <map>
#include <functional>
#include <cpu/decode.hpp>
#include <fpu/decode.hpp>
#include <rsp/decode.hpp>
#include <cop0/decode.hpp>

struct Core;

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
  }}, {0b010000, cop0decode
  }, {0b010001, cop1decode
  }, {0b010010, [](u32 instr) {
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