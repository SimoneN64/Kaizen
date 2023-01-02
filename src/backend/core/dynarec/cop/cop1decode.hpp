#pragma once
#include <dynarec/cop/cop1instructions.hpp>

namespace n64::JIT {
bool cop1Decode(n64::Registers& regs, u32 instr, Dynarec& cpu);
}