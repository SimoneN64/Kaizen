#pragma once
#include <n64/core/cpu/Registers.hpp>
#include <n64/core/Mem.hpp>

namespace natsukashii::n64::core {
struct Cpu {
  Cpu() = default;
  void Step(Mem&);
  Registers regs;
};

enum class ExceptionCode : u8 {
  Interrupt,
  TLBModification,
  TLBLoad,
  TLBStore,
  AddressErrorLoad,
  AddressErrorStore,
  InstructionBusError,
  DataBusError,
  Syscall,
  Breakpoint,
  ReservedInstruction,
  CoprocessorUnusable,
  Overflow,
  Trap,
  FloatingPointError = 15,
  Watch = 23
};

void FireException(Registers& regs, ExceptionCode code, int cop, s64 pc);
}
