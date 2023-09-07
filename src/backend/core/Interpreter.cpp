#include <Core.hpp>

namespace n64 {
int Interpreter::Step() {
  CheckCompareInterrupt();

  regs.prevDelaySlot = regs.delaySlot;
  regs.delaySlot = false;

  u32 paddr = 0;
  if(!MapVAddr(regs, LOAD, regs.pc, paddr)) {
    HandleTLBException(regs, regs.pc);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, LOAD), 0, false);
    return 1;
  }

  u32 instruction = mem.Read32(regs, paddr);

  if(ShouldServiceInterrupt()) {
    FireException(regs, ExceptionCode::Interrupt, 0, false);
    return 1;
  }

  regs.oldPC = regs.pc;
  regs.pc = regs.nextPC;
  regs.nextPC += 4;

  Exec(instruction);

  return 1;
}
}