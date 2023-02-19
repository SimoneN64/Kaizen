#pragma once
#include <Dynarec.hpp>

namespace n64 {
void mtc0(Dynarec&, u32);
void dmtc0(Dynarec&, u32);
void mfc0(Dynarec&, u32);
void dmfc0(Dynarec&, u32);
void eret(Dynarec&);
void tlbr(Dynarec&);
void tlbw(Dynarec&, int);
void tlbp(Dynarec&);
}