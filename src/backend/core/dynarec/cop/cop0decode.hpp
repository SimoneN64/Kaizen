#pragma once
#include <common.hpp>

namespace n64 {
struct Registers;
}

namespace n64::JIT {
void cop0Decode(n64::Registers& regs, u32 instr);
}