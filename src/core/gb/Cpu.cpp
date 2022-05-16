#include <Cpu.hpp>
#include <util.hpp>

namespace natsukashii::core {
Cpu::Cpu() {
}

void Cpu::Step(Mem& mem) {
  u8 opcode = mem.Consume<u8>(pc);
  DecodeAndExecute(opcode);
}

void Cpu::DecodeAndExecute(u8 opcode) {
  switch(opcode) {
    default: util::panic("Unimplemented opcode %02X", opcode);
  }
}
}
