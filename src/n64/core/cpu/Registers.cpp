#include <Registers.hpp>

namespace n64 {
Registers::Registers() {
  Reset();
}

void Registers::Reset() {
  delaySlot = false;
  prevDelaySlot = false;
  memset(gpr, 0, 32*sizeof(s64));
  oldPC = (s64)0xFFFFFFFFA4000040;
  pc = oldPC;
  nextPC = pc + 4;
}

void Registers::SetPC(s64 val) {
  pc = val;
  nextPC = pc + 4;
}
}
