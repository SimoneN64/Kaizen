#pragma once
#include <JIT.hpp>

namespace n64 {
struct Registers;
void cop0Emit(JIT& cpu, u32 instr);
bool cop0IsEndBlock(u32 instr);
}