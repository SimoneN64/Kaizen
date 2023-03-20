#pragma once
#include <JIT.hpp>

namespace n64 {
struct Registers;
void cop0Decode(JIT& cpu, u32 instr);
}