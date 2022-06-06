#include <Cpu.hpp>
#include <util.hpp>

namespace natsukashii::gb::core {
Cpu::Cpu() {}

void Cpu::Step(Mem& mem) {
  FetchDecodeExecute(mem);
}

void Cpu::FetchDecodeExecute(Mem& mem) {
  u8 opcode = mem.Read8(regs.pc);
  switch(opcode) {
    case 0x01: case 0x11: case 0x21: case 0x31: // LD r16, u16
      SetR16<1>((opcode >> 4) & 3, mem.Consume16(regs.pc));
      break;
    case 0xA8 ... 0xAF: // XOR A, r8
      regs.a() ^= GetR8(opcode & 7, mem);
      regs.f().set(regs.a() == 0, false, false, false);
      break;
    case 0x02: case 0x12: case 0x22: case 0x32: {
      u8 bits = (opcode >> 4) & 3;
      mem.Write8(GetR16<2>(bits), regs.a());
    } break;
    default: util::panic("Unimplemented opcode {:02X}, pc: {:04X}", opcode, regs.pc);
  }

  regs.pc++;
}
}
