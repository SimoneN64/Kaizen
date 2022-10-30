#pragma once
#include <core/registers/Cop0.hpp>
#include <core/registers/Cop1.hpp>

namespace n64 {
struct Registers {
  Registers();
  void Reset();
  void SetPC(s64);
  s64 gpr[32];
  Cop0 cop0;
  Cop1 cop1;
  s64 oldPC, pc, nextPC;
  s64 hi, lo;
  bool prevDelaySlot, delaySlot;
};
}
