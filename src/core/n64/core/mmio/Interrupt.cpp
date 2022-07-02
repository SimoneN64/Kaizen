#include <n64/core/mmio/Interrupt.hpp>
#include <n64/core/mmio/MI.hpp>
#include <n64/core/cpu/Registers.hpp>

namespace natsukashii::n64::core {
void InterruptRaise(MI &mi, Registers &regs, Interrupt intr) {
  switch(intr) {
    case Interrupt::VI:
      mi.miIntr.vi = true;
      break;
    case Interrupt::SI:
      mi.miIntr.si = true;
      break;
    case Interrupt::PI:
      mi.miIntr.pi = true;
      break;
    case Interrupt::AI:
      mi.miIntr.ai = true;
      break;
    case Interrupt::DP:
      mi.miIntr.dp = true;
      break;
    case Interrupt::SP:
      mi.miIntr.sp = true;
      break;
  }

  UpdateInterrupt(mi, regs);
}
void InterruptLower(MI &mi, Registers &regs, Interrupt intr) {
  switch(intr) {
    case Interrupt::VI:
      mi.miIntr.vi = false;
      break;
    case Interrupt::SI:
      mi.miIntr.si = false;
      break;
    case Interrupt::PI:
      mi.miIntr.pi = false;
      break;
    case Interrupt::AI:
      mi.miIntr.ai = false;
      break;
    case Interrupt::DP:
      mi.miIntr.dp = false;
      break;
    case Interrupt::SP:
      mi.miIntr.sp = false;
      break;
  }

  UpdateInterrupt(mi, regs);
}
void UpdateInterrupt(MI &mi, Registers &regs) {
  bool interrupt = mi.miIntr.raw & mi.miIntrMask.raw;
  regs.cop0.cause.ip2 = interrupt;
}
}