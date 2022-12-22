#pragma once
#include <common.hpp>
#include <core/mmio/MI.hpp>

namespace n64 {

struct Registers;

enum class Interrupt : u8 {
  VI, SI, PI, AI, DP, SP
};

void InterruptRaise(MI &mi, Registers &regs, Interrupt intr);
void InterruptLower(MI &mi, Registers &regs, Interrupt intr);
void UpdateInterrupt(MI &mi, Registers &regs);
}