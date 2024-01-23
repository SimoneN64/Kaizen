#pragma once
#include <log.hpp>

namespace n64 {
struct IRGuestReg {
  IRGuestReg(u8 reg) : reg(reg) {}

  /// The ARM general purpose register
  const u8 reg;
};
}