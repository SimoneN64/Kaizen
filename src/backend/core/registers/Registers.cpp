#include <core/registers/Registers.hpp>

namespace n64 {
Registers::Registers() : cop0(*this), cop1(*this) { Reset(); }

void Registers::Reset() {
  hi = 0;
  lo = 0;
  delaySlot = false;
  prevDelaySlot = false;
  gpr.fill(0);
  gprIsConstant.fill(false);

  cop0.Reset();
  cop1.Reset();

  steps = 0;
  extraCycles = 0;
}

void Registers::SetPC64(s64 val) {
  oldPC = pc;
  pc = val;
  nextPC = pc + 4;
}

void Registers::SetPC32(s32 val) {
  oldPC = pc;
  pc = s64(val);
  nextPC = pc + 4;
}

template <>
u64 Registers::Read<u64>(size_t idx) {
  return idx == 0 ? 0 : gpr[idx];
}

template <>
s64 Registers::Read<s64>(size_t idx) {
  return s64(Read<u64>(idx));
}

template <>
u32 Registers::Read<u32>(size_t idx) {
  return idx == 0 ? 0 : gpr[idx];
}

template <>
s32 Registers::Read<s32>(size_t idx) {
  return s32(Read<u32>(idx));
}

template <>
u16 Registers::Read<u16>(size_t idx) {
  return idx == 0 ? 0 : gpr[idx];
}

template <>
s16 Registers::Read<s16>(size_t idx) {
  return s16(Read<u16>(idx));
}

template <>
u8 Registers::Read<u8>(size_t idx) {
  return idx == 0 ? 0 : gpr[idx];
}

template <>
s8 Registers::Read<s8>(size_t idx) {
  return s8(Read<u8>(idx));
}

template <>
void Registers::Write<bool>(size_t idx, bool v) {
  if (idx == 0)
    return;
  gpr[idx] = v;
  gprIsConstant[idx] = true;
}

template <>
void Registers::Write<u64>(size_t idx, u64 v) {
  if (idx == 0)
    return;
  gpr[idx] = v;
  gprIsConstant[idx] = true;
}

template <>
void Registers::Write<s64>(size_t idx, s64 v) {
  Write<u64>(idx, v);
}

template <>
void Registers::Write<u32>(size_t idx, u32 v) {
  if (idx == 0)
    return;
  gpr[idx] = (u32)v;
  gprIsConstant[idx] = true;
}

template <>
void Registers::Write<s32>(size_t idx, s32 v) {
  if (idx == 0)
    return;
  gpr[idx] = v;
  gprIsConstant[idx] = true;
}

template <>
void Registers::Write<u16>(size_t idx, u16 v) {
  if (idx == 0)
    return;
  gpr[idx] = (u16)v;
  gprIsConstant[idx] = true;
}

template <>
void Registers::Write<s16>(size_t idx, s16 v) {
  if (idx == 0)
    return;
  gpr[idx] = v;
  gprIsConstant[idx] = true;
}

template <>
void Registers::Write<u8>(size_t idx, u8 v) {
  if (idx == 0)
    return;
  gpr[idx] = (u8)v;
  gprIsConstant[idx] = true;
}

template <>
void Registers::Write<s8>(size_t idx, s8 v) {
  if (idx == 0)
    return;
  gpr[idx] = v;
  gprIsConstant[idx] = true;
}
} // namespace n64
