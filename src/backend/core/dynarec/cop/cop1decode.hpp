#pragma once
#include <dynarec/cop/cop1instructions.hpp>

namespace n64 {
bool cop1Decode(Registers&, Dynarec& cpu, u32 instr);
}