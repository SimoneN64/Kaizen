#include <core/Interpreter.hpp>
#include <log.hpp>

namespace n64 {
inline bool ShouldServiceInterrupt(Registers& regs) {
  bool interrupts_pending = (regs.cop0.status.im & regs.cop0.cause.interruptPending) != 0;
  bool interrupts_enabled = regs.cop0.status.ie == 1;
  bool currently_handling_exception = regs.cop0.status.exl == 1;
  bool currently_handling_error = regs.cop0.status.erl == 1;

  return interrupts_pending && interrupts_enabled &&
         !currently_handling_exception && !currently_handling_error;
}

inline void CheckCompareInterrupt(MI& mi, Registers& regs) {
  regs.cop0.count++;
  regs.cop0.count &= 0x1FFFFFFFF;
  if(regs.cop0.count == (u64)regs.cop0.compare << 1) {
    regs.cop0.cause.ip7 = 1;
    UpdateInterrupt(mi, regs);
  }
}

void Interpreter::Step(Mem& mem) {
  regs.gpr[0] = 0;

  CheckCompareInterrupt(mem.mmio.mi, regs);

  regs.prevDelaySlot = regs.delaySlot;
  regs.delaySlot = false;

  u32 paddr = 0;
  if(!MapVAddr(regs, LOAD, regs.pc, paddr)) {
    HandleTLBException(regs, regs.pc);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, LOAD), 0, false);
    return;
  }

  u32 instruction = mem.Read32(regs, paddr);

  if(ShouldServiceInterrupt(regs)) {
    FireException(regs, ExceptionCode::Interrupt, 0, false);
    return;
  }

  regs.oldPC = regs.pc;
  regs.pc = regs.nextPC;
  regs.nextPC += 4;

  Exec(mem, instruction);
}
}