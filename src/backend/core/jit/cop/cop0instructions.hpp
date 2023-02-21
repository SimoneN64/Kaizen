#pragma once
#include <JIT.hpp>

namespace n64 {
void mtc0(JIT&, u32);
void dmtc0(JIT&, u32);
void mfc0(JIT&, u32);
void dmfc0(JIT&, u32);
void eret(JIT&);
void tlbr(JIT&);
void tlbw(JIT&, int);
void tlbp(JIT&);
}