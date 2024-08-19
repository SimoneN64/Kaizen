#pragma once
#include <backend/core/registers/Cop1.hpp>

namespace n64 {
struct Registers {
  Registers();
  void Reset();
  void SetPC64(s64);
  void SetPC32(s32);

  bool IsRegConstant(u32 index) {
    if(index == 0) return true;
    return gprIsConstant[index];
  }

  bool IsRegConstant(u32 first, u32 second) {
    return IsRegConstant(first) && IsRegConstant(second);
  }

  bool gprIsConstant[32]{};
  bool loIsConstant = false, hiIsConstant = false;
  Cop0 cop0;
  Cop1 cop1;
  s64 oldPC{}, pc{}, nextPC{};
  s64 hi{}, lo{};
  bool prevDelaySlot{}, delaySlot{};
  u32 steps = 0;
  u32 extraCycles = 0;

  void CpuStall(u32 cycles) {
    extraCycles += cycles;
  }

  u32 PopStalledCycles() {
    u32 ret = extraCycles;
    extraCycles = 0;
    return ret;
  }

  template <typename T>
  T Read(size_t);
  template <typename T>
  void Write(size_t, T);
private:
  s64 gpr[32]{};
};
}
