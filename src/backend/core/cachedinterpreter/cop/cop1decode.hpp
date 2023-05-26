#pragma once
#include <CachedInterpreter.hpp>

namespace n64 {
CachedFn cop1GetFunc(CachedInterpreter &cpu, u32 instr);
}
