#pragma once
#include <jit/cop/cop1instructions.hpp>

namespace n64 {
void cop1Emit(JIT& cpu, u32 instr);
bool cop1IsEndBlock(u32 instr);
}