#include <Registers.hpp>

namespace natsukashii::n64::core {
Registers::Registers() {
  delaySlot = false;
  prevDelaySlot = false;
  memset(gpr, 0, 32*sizeof(s64));
  oldPC = (s64)0xFFFFFFFFA4000040;
  pc = oldPC;
  nextPC = pc + 4;
  lo = 0;
  hi = 0;
  gpr[11] = (s64)0xFFFFFFFFA4000040;
  gpr[20] = 0x0000000000000001;
  gpr[22] = 0x000000000000003F;
  gpr[29] = (s64)0xFFFFFFFFA4001FF0;
}

void Registers::SetPC(s64 val) {
  pc = val;
  nextPC = pc + 4;
}
}
