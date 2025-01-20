#include "jit/helpers.hpp"


#include <core/registers/Registers.hpp>
#include <core/JIT.hpp>

namespace n64 {
Registers::Registers(JIT *jit) : jit(jit), cop0(*this), cop1(*this) { Reset(); }

void Registers::Reset() {
  hi = 0;
  lo = 0;
  delaySlot = false;
  prevDelaySlot = false;
  gpr.fill(0);
  gprIsConstant.fill(false);
  gprIsConstant[0] = true;

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
s64 Registers::Read<s64>(const size_t idx) {
  return static_cast<s64>(Read<u64>(idx));
}

template <>
u32 Registers::Read<u32>(size_t idx) {
  return idx == 0 ? 0 : gpr[idx];
}

template <>
s32 Registers::Read<s32>(size_t idx) {
  return static_cast<s32>(Read<u32>(idx));
}

template <>
u16 Registers::Read<u16>(size_t idx) {
  return idx == 0 ? 0 : gpr[idx];
}

template <>
s16 Registers::Read<s16>(size_t idx) {
  return static_cast<s16>(Read<u16>(idx));
}

template <>
u8 Registers::Read<u8>(size_t idx) {
  return idx == 0 ? 0 : gpr[idx];
}

template <>
s8 Registers::Read<s8>(size_t idx) {
  return static_cast<s8>(Read<u8>(idx));
}

template <>
void Registers::Read<u64>(size_t idx, Xbyak::Reg reg) {
  jit->code.mov(reg.cvt64(), jit->GPR<u64>(idx));
}

template <>
void Registers::Read<s64>(size_t idx, Xbyak::Reg reg) {
  jit->code.mov(reg.cvt64(), jit->GPR<u64>(idx));
}

template <>
void Registers::Read<u32>(size_t idx, Xbyak::Reg reg) {
  jit->code.mov(reg.cvt32(), jit->GPR<u32>(idx));
}

template <>
void Registers::Read<s32>(size_t idx, Xbyak::Reg reg) {
  jit->code.mov(reg.cvt32(), jit->GPR<s32>(idx));
}

template <>
void Registers::Read<u16>(size_t idx, Xbyak::Reg reg) {
  jit->code.mov(reg.cvt16(), jit->GPR<u16>(idx));
}

template <>
void Registers::Read<s16>(size_t idx, Xbyak::Reg reg) {
  jit->code.mov(reg.cvt16(), jit->GPR<u16>(idx));
}

template <>
void Registers::Read<u8>(size_t idx, Xbyak::Reg reg) {
  jit->code.mov(reg.cvt8(), jit->GPR<u8>(idx));
}

template <>
void Registers::Read<s8>(size_t idx, Xbyak::Reg reg) {
  jit->code.mov(reg.cvt8(), jit->GPR<s8>(idx));
}

template <>
void Registers::WriteJIT<bool>(size_t idx, bool v) {
  jit->code.mov(jit->code.al, v);
  jit->code.mov(jit->GPR<u8>(idx), jit->code.al);
}

template <>
void Registers::Write<bool>(size_t idx, bool v, bool isConstant) {
  if (idx == 0)
    return;

  bool oldIsConstant = gprIsConstant[idx];
  gprIsConstant[idx] = isConstant;

  if (jit) {
    if (oldIsConstant) {
      gpr[idx] = v;
      return;
    }

    WriteJIT<bool>(idx, v);
    return;
  }

  gpr[idx] = v;
}

template <>
void Registers::WriteJIT<u64>(size_t idx, u64 v) {
  jit->code.mov(jit->code.rax, v);
  jit->code.mov(jit->GPR<u64>(idx), jit->code.rax);
}

template <>
void Registers::Write<u64>(size_t idx, u64 v, bool isConstant) {
  if (idx == 0)
    return;

  bool oldIsConstant = gprIsConstant[idx];
  gprIsConstant[idx] = isConstant;

  if (jit) {
    if (oldIsConstant) {
      gpr[idx] = v;
      return;
    }

    WriteJIT<u64>(idx, v);
    return;
  }

  gpr[idx] = v;
}

template <>
void Registers::Write<s64>(size_t idx, s64 v, bool isConstant) {
  Write<u64>(idx, v, isConstant);
}

template <>
void Registers::WriteJIT<u32>(size_t idx, u32 v) {
  jit->code.mov(jit->code.eax, v);
  jit->code.mov(jit->GPR<u32>(idx), jit->code.eax);
}

template <>
void Registers::Write<u32>(size_t idx, u32 v, bool isConstant) {
  if (idx == 0)
    return;

  bool oldIsConstant = gprIsConstant[idx];
  gprIsConstant[idx] = isConstant;

  if (jit) {
    if (oldIsConstant) {
      gpr[idx] = v;
      return;
    }

    WriteJIT<u32>(idx, v);
    return;
  }

  gpr[idx] = v;
}

template <>
void Registers::WriteJIT<s32>(size_t idx, s32 v) {
  jit->code.mov(jit->code.eax, v);
  jit->code.movsxd(jit->code.rax, jit->code.eax);
  jit->code.mov(jit->GPR<u64>(idx), jit->code.rax);
}

template <>
void Registers::Write<s32>(size_t idx, s32 v, bool isConstant) {
  if (idx == 0)
    return;

  bool oldIsConstant = gprIsConstant[idx];
  gprIsConstant[idx] = isConstant;

  if (jit) {
    if (oldIsConstant) {
      gpr[idx] = v;
      return;
    }

    WriteJIT<s32>(idx, v);
    return;
  }

  gpr[idx] = v;
}

template <>
void Registers::WriteJIT<u16>(size_t idx, u16 v) {
  jit->code.mov(jit->code.ax, v);
  jit->code.mov(jit->GPR<u16>(idx), jit->code.ax);
}

template <>
void Registers::Write<u16>(size_t idx, u16 v, bool isConstant) {
  if (idx == 0)
    return;

  bool oldIsConstant = gprIsConstant[idx];
  gprIsConstant[idx] = isConstant;

  if (jit) {
    if (oldIsConstant) {
      gpr[idx] = v;
      return;
    }

    WriteJIT<u16>(idx, v);
    return;
  }

  gpr[idx] = v;
}

template <>
void Registers::WriteJIT<s16>(size_t idx, s16 v) {
  jit->code.mov(jit->code.ax, v);
  jit->code.movsx(jit->code.rax, jit->code.ax);
  jit->code.mov(jit->GPR<u64>(idx), jit->code.rax);
}

template <>
void Registers::Write<s16>(size_t idx, s16 v, bool isConstant) {
  if (idx == 0)
    return;

  bool oldIsConstant = gprIsConstant[idx];
  gprIsConstant[idx] = isConstant;

  if (jit) {
    if (oldIsConstant) {
      gpr[idx] = v;
      return;
    }

    WriteJIT<s16>(idx, v);
    return;
  }

  gpr[idx] = v;
}

template <>
void Registers::WriteJIT<u8>(size_t idx, u8 v) {
  jit->code.mov(jit->code.al, v);
  jit->code.mov(jit->GPR<u8>(idx), jit->code.al);
}

template <>
void Registers::Write<u8>(size_t idx, u8 v, bool isConstant) {
  if (idx == 0)
    return;

  bool oldIsConstant = gprIsConstant[idx];
  gprIsConstant[idx] = isConstant;

  if (jit) {
    if (oldIsConstant) {
      gpr[idx] = v;
      return;
    }

    WriteJIT<u8>(idx, v);
    return;
  }

  gpr[idx] = v;
}

template <>
void Registers::WriteJIT<s8>(size_t idx, s8 v) {
  jit->code.mov(jit->code.al, v);
  jit->code.movsx(jit->code.rax, jit->code.al);
  jit->code.mov(jit->GPR<u64>(idx), jit->code.rax);
}

template <>
void Registers::Write<s8>(size_t idx, s8 v, bool isConstant) {
  if (idx == 0)
    return;

  bool oldIsConstant = gprIsConstant[idx];
  gprIsConstant[idx] = isConstant;

  if (jit) {
    if (oldIsConstant) {
      gpr[idx] = v;
      return;
    }

    WriteJIT<s8>(idx, v);
    return;
  }

  gpr[idx] = v;
}

template <>
void Registers::Write<s8>(size_t idx, Xbyak::Reg v) {
  if (idx == 0)
    return;

  gprIsConstant[idx] = false;

  if (!jit)
    Util::panic("Did you try to call Registers::Write(size_t, *Xbyak::Reg*) from the interpreter?");

  jit->code.movsx(v.cvt64(), v.cvt8());
  jit->code.mov(jit->GPR<u64>(idx), v);
}

template <>
void Registers::Write<u8>(size_t idx, Xbyak::Reg v) {
  if (idx == 0)
    return;

  gprIsConstant[idx] = false;

  if (!jit)
    Util::panic("Did you try to call Registers::Write(size_t, *Xbyak::Reg*) from the interpreter?");

  jit->code.mov(jit->GPR<u8>(idx), v.cvt8());
}

template <>
void Registers::Write<s16>(size_t idx, Xbyak::Reg v) {
  if (idx == 0)
    return;

  gprIsConstant[idx] = false;

  if (!jit)
    Util::panic("Did you try to call Registers::Write(size_t, *Xbyak::Reg*) from the interpreter?");

  jit->code.movsx(v.cvt64(), v.cvt16());
  jit->code.mov(jit->GPR<u64>(idx), v.cvt64());
}

template <>
void Registers::Write<u16>(size_t idx, Xbyak::Reg v) {
  if (idx == 0)
    return;

  gprIsConstant[idx] = false;

  if (!jit)
    Util::panic("Did you try to call Registers::Write(size_t, *Xbyak::Reg*) from the interpreter?");

  jit->code.mov(jit->GPR<u16>(idx), v.cvt16());
}

template <>
void Registers::Write<s32>(size_t idx, Xbyak::Reg v) {
  if (idx == 0)
    return;

  gprIsConstant[idx] = false;

  if (!jit)
    Util::panic("Did you try to call Registers::Write(size_t, *Xbyak::Reg*) from the interpreter?");

  jit->code.movsxd(v.cvt64(), v.cvt32());
  jit->code.mov(jit->GPR<u64>(idx), v.cvt64());
}

template <>
void Registers::Write<u32>(size_t idx, Xbyak::Reg v) {
  if (idx == 0)
    return;

  gprIsConstant[idx] = false;

  if (!jit)
    Util::panic("Did you try to call Registers::Write(size_t, *Xbyak::Reg*) from the interpreter?");

  jit->code.mov(jit->GPR<u32>(idx), v.cvt32());
}

template <>
void Registers::Write<u64>(size_t idx, Xbyak::Reg v) {
  if (idx == 0)
    return;

  gprIsConstant[idx] = false;

  if (!jit)
    Util::panic("Did you try to call Registers::Write(size_t, *Xbyak::Reg*) from the interpreter?");

  jit->code.mov(jit->GPR<u64>(idx), v.cvt64());
}

template <>
void Registers::Write<s64>(size_t idx, Xbyak::Reg v) {
  Write<u64>(idx, v);
}
} // namespace n64
