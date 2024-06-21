#include <core/mmio/MI.hpp>
#include <core/registers/Registers.hpp>

namespace n64 {
void MI::InterruptRaise(Interrupt intr) {
  switch(intr) {
    case Interrupt::VI:
      miIntr.vi = true;
      break;
    case Interrupt::SI:
      miIntr.si = true;
      break;
    case Interrupt::PI:
      miIntr.pi = true;
      break;
    case Interrupt::AI:
      miIntr.ai = true;
      break;
    case Interrupt::DP:
      miIntr.dp = true;
      break;
    case Interrupt::SP:
      miIntr.sp = true;
      break;
  }

  UpdateInterrupt();
}

void MI::InterruptLower(Interrupt intr) {
  switch(intr) {
    case Interrupt::VI:
      miIntr.vi = false;
      break;
    case Interrupt::SI:
      miIntr.si = false;
      break;
    case Interrupt::PI:
      miIntr.pi = false;
      break;
    case Interrupt::AI:
      miIntr.ai = false;
      break;
    case Interrupt::DP:
      miIntr.dp = false;
      break;
    case Interrupt::SP:
      miIntr.sp = false;
      break;
  }

  UpdateInterrupt();
}

void MI::UpdateInterrupt() {
  bool interrupt = miIntr.raw & miIntrMask.raw;
  regs.cop0.cause.ip2 = interrupt;
}
}