#pragma once
#include <common.hpp>
#include <MI.hpp>

namespace natsukashii::n64::core {

struct Registers;

enum class InterruptType : u8 {
  VI, SI, PI, AI, DP, SP
};

void InterruptRaise(MI &mi, Registers &regs, InterruptType intr);
void InterruptLower(MI &mi, Registers &regs, InterruptType intr);
void UpdateInterrupt(MI &mi, Registers &regs);
}