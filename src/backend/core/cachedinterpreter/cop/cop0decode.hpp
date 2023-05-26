#pragma once
#include <CachedInterpreter.hpp>

namespace n64 {
struct Registers;
CachedFn cop0GetFunc(CachedInterpreter& cpu, u32 instr);
}