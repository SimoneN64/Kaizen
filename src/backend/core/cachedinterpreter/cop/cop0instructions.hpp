#pragma once
#include <CachedInterpreter.hpp>

namespace n64 {
void mtc0(CachedInterpreter&, u32);
void dmtc0(CachedInterpreter&, u32);
void mfc0(CachedInterpreter&, u32);
void dmfc0(CachedInterpreter&, u32);
void eret(CachedInterpreter&);
void tlbr(CachedInterpreter&);
void tlbw(CachedInterpreter&, int);
void tlbp(CachedInterpreter&);
}