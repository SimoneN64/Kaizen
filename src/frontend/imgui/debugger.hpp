#pragma once
#include <util.hpp>
#include <map>
#include <functional>
#include <cpu/decode.hpp>
#include <fpu/decode.hpp>
#include <rsp/decode.hpp>
#include <cop0/decode.hpp>

struct Core;
// TODO: Fix syntax
static std::map<u8, std::function<std::string(u32)>> cpuInstr = {
  {0b000000, special},
  {0b000001, regimm},
  {0b000010, [](u32 instr) { return fmt::format("j {:08X}", instr & 0x3FFFFFF); }},
  {0b000011, [](u32 instr) { return fmt::format("jal {:08X}", instr & 0x3FFFFFF); }},
  {0b000100, [](u32 instr) { return fmt::format("beq {}, {}, {:04X}", gprStr[RS(instr)], gprStr[RT(instr)], instr & 0xffff); }},
  {0b000101, [](u32 instr) { return fmt::format("bne {}, {}, {:04X}", gprStr[RS(instr)], gprStr[RT(instr)], instr & 0xffff); }},
  {0b000110, [](u32 instr) { return fmt::format("blez {}, {:04X}", gprStr[RS(instr)], instr & 0xffff); }},
  {0b000111, [](u32 instr) { return fmt::format("bgtz {}, {:04X}", gprStr[RS(instr)], instr & 0xffff); }},
  {0b001000, [](u32 instr) { return fmt::format("addi {}, {:04X}", gprStr[RS(instr)], instr & 0xFFFF); }},
  {0b001001, [](u32 instr) { return fmt::format("addiu {}, {:04X}", gprStr[RS(instr)], instr & 0x3FFFFFF); }},
  {0b001010, [](u32 instr) { return fmt::format("slti {}, {}, {:04X}", gprStr[RT(instr)], gprStr[RS(instr)], instr & 0xFFFF); }},
  {0b001011, [](u32 instr) { return fmt::format("sltiu {}, {}, {:04X}", gprStr[RT(instr)], gprStr[RS(instr)], instr & 0xFFFF); }},
  {0b001100, [](u32 instr) { return fmt::format("andi {}, {}", gprStr[RS(instr)], gprStr[RT(instr)]); }},
  {0b001101, [](u32 instr) { return fmt::format("ori {}, {}", gprStr[RS(instr)], gprStr[RT(instr)]); }},
  {0b001110, [](u32 instr) { return fmt::format("xori {}", gprStr[RS(instr)]); }},
  {0b001111, [](u32 instr) { return fmt::format("lui {}", gprStr[RS(instr)]); }},
  {0b010000, cop0decode},
  {0b010001, cop1decode},
  {0b010010, rspDecode},
  {0b010011, [](u32 instr) { return fmt::format("RESERVED"); }},
  {0b010100, [](u32 instr) { return fmt::format("beql {}, {}, {:04X}", gprStr[RS(instr)], gprStr[RT(instr)], instr & 0xffff); }},
  {0b010101, [](u32 instr) { return fmt::format("bnel {}, {}, {:04X}", gprStr[RS(instr)], gprStr[RT(instr)], instr & 0xffff); }},
  {0b010110, [](u32 instr) { return fmt::format("blezl {}, {:04X}", gprStr[RS(instr)], instr & 0xffff); }},
  {0b010111, [](u32 instr) { return fmt::format("bgtzl {}, {:04X}", gprStr[RS(instr)], instr & 0xffff); }},
  {0b011000, [](u32 instr) { return fmt::format("daddi {}, {}, {}", gprStr[RT(instr)], gprStr[RS(instr)], instr & 0xFFFF);}},
  {0b011001, [](u32 instr) { return fmt::format("daddiu {}, {}, {}", gprStr[RT(instr)], gprStr[RS(instr)], instr & 0xFFFF); }},
  {0b011010, [](u32 instr) { return fmt::format("ldl {:08X}", instr & 0x3FFFFFF); }},
  {0b011011, [](u32 instr) { return fmt::format("ldr {:08X}", instr & 0x3FFFFFF); }},
  {0b011100, [](u32 instr) { return fmt::format("RESERVED"); }},
  {0b011101, [](u32 instr) { return fmt::format("RESERVED"); }},
  {0b011110, [](u32 instr) { return fmt::format("RESERVED"); }},
  {0b011111, [](u32 instr) { return fmt::format("RESERVED"); }},
  {0b100000, [](u32 instr) { return fmt::format("lb {:08X}", instr & 0x3FFFFFF); }},
  {0b100001, [](u32 instr) { return fmt::format("lh {:08X}", instr & 0x3FFFFFF); }},
  {0b100010, [](u32 instr) { return fmt::format("lwl {:08X}", instr & 0x3FFFFFF); }},
  {0b100011, [](u32 instr) { return fmt::format("lw {:08X}", instr & 0x3FFFFFF); }},
  {0b100100, [](u32 instr) { return fmt::format("lbu {}, {}", gprStr[RS(instr)], gprStr[RT(instr)]); }},
  {0b100101, [](u32 instr) { return fmt::format("lhu {}, {}", gprStr[RS(instr)], gprStr[RT(instr)]); }},
  {0b100110, [](u32 instr) { return fmt::format("lwr {}", gprStr[RS(instr)]); }},
  {0b100111, [](u32 instr) { return fmt::format("lwu {}", gprStr[RS(instr)]); }},
  {0b101000, [](u32 instr) { return fmt::format("sb {:08X}", instr & 0x3FFFFFF); }},
  {0b101001, [](u32 instr) { return fmt::format("sh {:08X}", instr & 0x3FFFFFF); }},
  {0b101010, [](u32 instr) { return fmt::format("swl {:08X}", instr & 0x3FFFFFF); }},
  {0b101011, [](u32 instr) { return fmt::format("sw {:08X}", instr & 0x3FFFFFF); }},
  {0b101100, [](u32 instr) { return fmt::format("sdl {}, {}", gprStr[RS(instr)], gprStr[RT(instr)]); }},
  {0b101101, [](u32 instr) { return fmt::format("sdr {}, {}", gprStr[RS(instr)], gprStr[RT(instr)]); }},
  {0b101110, [](u32 instr) { return fmt::format("swr {}", gprStr[RS(instr)]); }},
  {0b101111, [](u32 instr) { return fmt::format("cache", gprStr[RS(instr)]); }},
  {0b110000, [](u32 instr) { return fmt::format("ll {:08X}", instr & 0x3FFFFFF); }},
  {0b110001, [](u32 instr) { return fmt::format("lwc1 {:08X}", instr & 0x3FFFFFF); }},
  {0b110010, [](u32 instr) { return fmt::format("lwc2 {:08X}", instr & 0x3FFFFFF); }},
  {0b110011, [](u32 instr) { return fmt::format("RESERVED"); }},
  {0b110100, [](u32 instr) { return fmt::format("lld {}, {}", gprStr[RS(instr)], gprStr[RT(instr)]); }},
  {0b110101, [](u32 instr) { return fmt::format("ldc1 {}, {}", gprStr[RS(instr)], gprStr[RT(instr)]); }},
  {0b110110, [](u32 instr) { return fmt::format("ldc2 {}", gprStr[RS(instr)]); }},
  {0b110111, [](u32 instr) { return fmt::format("ld {}", gprStr[RS(instr)]); }},
  {0b111000, [](u32 instr) { return fmt::format("sc {:08X}", instr & 0x3FFFFFF); }},
  {0b111001, [](u32 instr) { return fmt::format("swc1 {:08X}", instr & 0x3FFFFFF); }},
  {0b111010, [](u32 instr) { return fmt::format("swc2 {:08X}", instr & 0x3FFFFFF); }},
  {0b111011, [](u32 instr) { return fmt::format("RESERVED"); }},
  {0b111100, [](u32 instr) { return fmt::format("scd {}, {}", gprStr[RS(instr)], gprStr[RT(instr)]); }},
  {0b111101, [](u32 instr) { return fmt::format("sdc1 {}, {}", gprStr[RS(instr)], gprStr[RT(instr)]); }},
  {0b111110, [](u32 instr) { return fmt::format("sdc2 {}", gprStr[RS(instr)]); }},
  {0b111111, [](u32 instr) { return fmt::format("sd {}", gprStr[RS(instr)]); }}
};

struct Debugger {
  util::CircularBuffer<20, std::string> disassembled;

  inline void Decode(u32 instr) {
    const u8 mask = (instr >> 26) & 0x3f;
    disassembled.PushValue(cpuInstr[mask](instr));
  }
};