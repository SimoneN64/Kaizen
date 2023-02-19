#pragma once
#include <Dynarec.hpp>

namespace n64 {
struct Registers;
void cop0Decode(Registers&, Dynarec& cpu, u32 instr);
}