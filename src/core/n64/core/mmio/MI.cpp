#include <n64/core/mmio/MI.hpp>
#include <n64/core/cpu/Registers.hpp>
#include <util.hpp>
#include <n64/core/mmio/Interrupt.hpp>

#define MI_VERSION_REG 0x02020102

namespace natsukashii::n64::core {
MI::MI() {
  miIntrMask.raw = 0;
  miIntr.raw = 0;
  miMode = 0;
}

auto MI::Read(u32 paddr) const -> u32 {
  switch(paddr & 0xF) {
    case 0x0: return miMode & 0x3FF;
    case 0x4: return MI_VERSION_REG;
    case 0x8: return miIntr.raw & 0x3F;
    case 0xC: return miIntrMask.raw & 0x3F;
    default: util::panic("Unhandled MI[{:08X}] read\n", paddr);
  }
  return 0;
}

void MI::Write(Registers& regs, u32 paddr, u32 val) {
  switch(paddr & 0xF) {
    case 0x0:
      miMode &= 0xFFFFFF80;
      miMode |= val & 0x7F;
      if (val & (1 << 7)) {
        miMode &= ~(1 << 7);
      }

      if (val & (1 << 8)) {
        miMode |= 1 << 7;
      }

      if (val & (1 << 9)) {
        miMode &= ~(1 << 8);
      }

      if (val & (1 << 10)) {
        miMode |= 1 << 8;
      }

      if (val & (1 << 11)) {
        InterruptLower(*this, regs, Interrupt::DP);
      }

      if (val & (1 << 12)) {
        miMode &= ~(1 << 9);
      }

      if (val & (1 << 13)) {
        miMode |= 1 << 9;
      }
      break;
    case 0x4: break;
    case 0xC:
      for (int bit = 0; bit < 6; bit++) {
        int clearbit = bit << 1;
        int setbit = (bit << 1) + 1;

        if (val & (1 << clearbit)) {
          miIntrMask.raw &= ~(1 << bit);
        }

        if (val & (1 << setbit)) {
          miIntrMask.raw |= 1 << bit;
        }
      }

      UpdateInterrupt(*this, regs);
      break;
    default:
      util::panic("Unhandled MI[{:08X}] write ({:08X})\n", val, paddr);
  }
}
}