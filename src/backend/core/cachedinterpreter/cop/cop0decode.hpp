#pragma once
#include <CachedInterpreter.hpp>

namespace n64 {
struct Registers;
auto cop0GetFunc(CachedInterpreter& cpu, u32 instr);
}