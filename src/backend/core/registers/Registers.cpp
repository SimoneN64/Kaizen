#include <core/registers/Registers.hpp>

namespace n64 {
Registers::Registers() {
  Reset();
}

void Registers::Reset() {
  delaySlot = false;
  prevDelaySlot = false;
  memset(gpr, 0, 32*sizeof(s64));
}

void Registers::SetPC64(s64 val) {
  oldPC = pc;
  pc = val;
  nextPC = pc + 4;
}

void Registers::SetPC32(s32 val) {
  oldPC = pc;
  pc = s64(val);
  nextPC = pc + 4;
}
}
