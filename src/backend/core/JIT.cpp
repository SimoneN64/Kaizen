#include <JIT.hpp>

namespace n64 {
using namespace Xbyak;
JIT::JIT() : CodeGenerator(0x80000) { }

void JIT::Reset() {
  reset();
  regs.Reset();
  mem.Reset();
}

bool JIT::ShouldServiceInterrupt() {

}

void JIT::CheckCompareInterrupt() {

}

int JIT::Step() {
  CheckCompareInterrupt();

  regs.prevDelaySlot = regs.delaySlot;
  regs.delaySlot = false;

  u32 paddr = 0;
  if(!MapVAddr(regs, LOAD, regs.pc, paddr)) {
    HandleTLBException(regs, regs.pc);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, LOAD), 0, false);
    return 0;
  }

  u32 instruction = mem.Read32(regs, paddr);

  if(ShouldServiceInterrupt()) {
    FireException(regs, ExceptionCode::Interrupt, 0, false);
    return 0;
  }

  regs.oldPC = regs.pc;
  regs.pc = regs.nextPC;
  regs.nextPC += 4;

  return 1;
}
}