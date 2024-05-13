#include <core/registers/Registers.hpp>

namespace n64 {
Registers::Registers() : cop0(*this), cop1(*this) {
  Reset();
}

void Registers::Reset() {
  hi = 0;
  lo = 0;
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
