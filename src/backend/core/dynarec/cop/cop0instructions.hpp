#pragma once
#include <Dynarec.hpp>

namespace n64::JIT {
void mtc0(n64::Registers&, u32);
void dmtc0(n64::Registers&, u32);
void mfc0(n64::Registers&, u32);
void dmfc0(n64::Registers&, u32);
void eret(n64::Registers&);
void tlbr(n64::Registers&);
void tlbw(int, n64::Registers&);
void tlbp(n64::Registers&);
}