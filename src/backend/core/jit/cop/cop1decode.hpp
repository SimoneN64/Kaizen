#pragma once
#include <jit/cop/cop1instructions.hpp>

namespace n64 {
bool cop1Decode(Registers&, JIT& cpu, u32 instr);
}