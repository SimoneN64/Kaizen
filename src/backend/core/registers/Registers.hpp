#pragma once
#include <backend/core/registers/Cop0.hpp>
#include <backend/core/registers/Cop1.hpp>

namespace n64 {
struct Registers {
  Registers();
  void Reset();
  void SetPC64(s64);
  void SetPC32(s32);
  s64 gpr[32]{};
  Cop0 cop0{};
  Cop1 cop1{};
  s64 oldPC{}, pc{}, nextPC{};
  s64 hi{}, lo{};
  bool prevDelaySlot{}, delaySlot{};
  int steps{};
};
}
