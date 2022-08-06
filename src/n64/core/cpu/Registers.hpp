#pragma once
#include <Cop0.hpp>
#include <Cop1.hpp>

namespace n64 {
struct Registers {
  Registers();
  void SetPC(s64);
  s64 gpr[32];
  Cop0 cop0;
  Cop1 cop1;
  s64 oldPC, pc, nextPC;
  s64 hi, lo;
  bool LLBit;
  bool prevDelaySlot, delaySlot;
};

#define RD(x) (((x) >> 11) & 0x1F)
#define RT(x) (((x) >> 16) & 0x1F)
#define RS(x) (((x) >> 21) & 0x1F)
#define FD(x) (((x) >>  6) & 0x1F)
#define FT(x) RT(x)
#define FS(x) RD(x)
#define BASE(x) RS(x)
}
