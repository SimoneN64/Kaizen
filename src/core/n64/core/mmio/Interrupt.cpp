#include <Interrupt.hpp>
#include <MI.hpp>
#include <n64/core/cpu/Registers.hpp>

namespace natsukashii::n64::core {
void InterruptRaise(MI &mi, Registers &regs, InterruptType intr) {
  switch(intr) {
    case InterruptType::VI:
      mi.miIntr.vi = true;
      break;
    case InterruptType::SI:
      mi.miIntr.si = true;
      break;
    case InterruptType::PI:
      mi.miIntr.pi = true;
      break;
    case InterruptType::AI:
      mi.miIntr.ai = true;
      break;
    case InterruptType::DP:
      mi.miIntr.dp = true;
      break;
    case InterruptType::SP:
      mi.miIntr.sp = true;
      break;
  }

  UpdateInterrupt(mi, regs);
}
void InterruptLower(MI &mi, Registers &regs, InterruptType intr) {
  switch(intr) {
    case InterruptType::VI:
      mi.miIntr.vi = false;
      break;
    case InterruptType::SI:
      mi.miIntr.si = false;
      break;
    case InterruptType::PI:
      mi.miIntr.pi = false;
      break;
    case InterruptType::AI:
      mi.miIntr.ai = false;
      break;
    case InterruptType::DP:
      mi.miIntr.dp = false;
      break;
    case InterruptType::SP:
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