#pragma once
#include <array>
#include <xbyak.h>
#include <backend/core/registers/Cop1.hpp>

namespace n64 {
struct JIT;
struct Registers {
  Registers(JIT *jit = nullptr);
  void Reset();
  void SetPC64(s64);
  void SetPC32(s32);

  [[nodiscard]] bool IsRegConstant(const u32 index) const {
    if (index == 0)
      return true;
    return gprIsConstant[index];
  }

  [[nodiscard]] bool IsRegConstant(const u32 first, const u32 second) const {
    return IsRegConstant(first) && IsRegConstant(second);
  }

  JIT *jit = nullptr;

  std::array<bool, 32> gprIsConstant{};
  bool loIsConstant = false, hiIsConstant = false;
  Cop0 cop0;
  Cop1 cop1;
  s64 oldPC{}, pc{}, nextPC{};
  s64 hi{}, lo{};
  bool prevDelaySlot{}, delaySlot{}, block_delaySlot{};
  u32 steps = 0;
  u32 extraCycles = 0;

  void CpuStall(u32 cycles) { extraCycles += cycles; }

  u32 PopStalledCycles() {
    u32 ret = extraCycles;
    extraCycles = 0;
    return ret;
  }

  template <typename T>
  T Read(size_t);
  template <typename T>
  void Read(size_t, Xbyak::Reg);
  template <typename T>
  void Write(size_t, T, bool = false);
  template <typename T>
  void Write(size_t, Xbyak::Reg);

  std::array<s64, 32> gpr{};

private:
  template <typename T>
  void WriteJIT(size_t, T);
};
} // namespace n64
