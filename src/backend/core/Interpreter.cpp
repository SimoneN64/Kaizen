#include <Core.hpp>

namespace n64 {
Interpreter::Interpreter(ParallelRDP &parallel) : mem(regs, parallel) {}

bool Interpreter::ShouldServiceInterrupt() {
  bool interrupts_pending = (regs.cop0.status.im & regs.cop0.cause.interruptPending) != 0;
  bool interrupts_enabled = regs.cop0.status.ie == 1;
  bool currently_handling_exception = regs.cop0.status.exl == 1;
  bool currently_handling_error = regs.cop0.status.erl == 1;

  return interrupts_pending && interrupts_enabled && !currently_handling_exception && !currently_handling_error;
}

void Interpreter::CheckCompareInterrupt() {
  regs.cop0.count++;
  regs.cop0.count &= 0x1FFFFFFFF;
  if (regs.cop0.count == (u64)regs.cop0.compare << 1) {
    regs.cop0.cause.ip7 = 1;
    mem.mmio.mi.UpdateInterrupt();
  }
}

Disassembler::DisassemblyResult Interpreter::Disassemble(u32 address, u32 instruction) const {
  return Disassembler::instance().Disassemble(address, instruction);
}

int Interpreter::Step() {
  CheckCompareInterrupt();

  regs.prevDelaySlot = regs.delaySlot;
  regs.delaySlot = false;

  if (check_address_error(0b11, u64(regs.pc))) [[unlikely]] {
    regs.cop0.HandleTLBException(regs.pc);
    regs.cop0.FireException(ExceptionCode::AddressErrorLoad, 0, regs.pc);
    return 1;
  }

  u32 paddr = 0;
  if (!regs.cop0.MapVAddr(Cop0::LOAD, regs.pc, paddr)) {
    regs.cop0.HandleTLBException(regs.pc);
    regs.cop0.FireException(regs.cop0.GetTLBExceptionCode(regs.cop0.tlbError, Cop0::LOAD), 0, regs.pc);
    return 1;
  }

  u32 instruction = mem.Read<u32>(regs, paddr);

  if (ShouldServiceInterrupt()) {
    regs.cop0.FireException(ExceptionCode::Interrupt, 0, regs.pc);
    return 1;
  }

  regs.oldPC = regs.pc;
  regs.pc = regs.nextPC;
  regs.nextPC += 4;

  Exec(instruction);

  return 1;
}

std::vector<u8> Interpreter::Serialize() {
  std::vector<u8> res{};

  res.resize(sizeof(Registers));

  memcpy(res.data(), &regs, sizeof(Registers));

  return res;
}

void Interpreter::Deserialize(const std::vector<u8> &data) { memcpy(&regs, data.data(), sizeof(Registers)); }
} // namespace n64
