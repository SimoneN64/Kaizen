#pragma once
#include <common.hpp>

namespace n64 {

union MIIntr {
  struct {
    unsigned sp : 1;
    unsigned si : 1;
    unsigned ai : 1;
    unsigned vi : 1;
    unsigned pi : 1;
    unsigned dp : 1;
    unsigned : 26;
  };
  u32 raw;
};

struct Registers;

struct MI {
  enum class Interrupt : u8 { VI, SI, PI, AI, DP, SP };

  MI(Registers &);
  void Reset();
  [[nodiscard]] auto Read(u32) const -> u32;
  void Write(u32, u32);
  void InterruptRaise(Interrupt intr);
  void InterruptLower(Interrupt intr);
  void UpdateInterrupt();

  u32 miMode{};
  MIIntr miIntr{}, miIntrMask{};
  Registers &regs;
};
} // namespace n64
