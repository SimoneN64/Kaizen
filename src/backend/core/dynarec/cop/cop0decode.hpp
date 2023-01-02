#pragma once
#include <Dynarec.hpp>

namespace n64 {
struct Registers;
}

namespace n64::JIT {
void cop0Decode(n64::Registers& regs, u32 instr, Dynarec& cpu);
}