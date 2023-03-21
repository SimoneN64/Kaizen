#pragma once
#include <xbyak/xbyak.h>
#include <backend/core/Mem.hpp>
#include <fstream>
#include <BaseCPU.hpp>

namespace n64 {
using namespace Xbyak;
using namespace Xbyak::util;
using Fn = void (*)();

#ifdef _WIN64
#define ABI_MSVC
#else
#define ABI_SYSV
#endif

#ifdef ABI_MSVC
static constexpr Xbyak::Reg64 regArg0 = rcx;
static constexpr Xbyak::Reg64 regArg1 = rdx;
static constexpr Xbyak::Reg64 regArg2 = r8;
static constexpr Xbyak::Reg64 regArg3 = r9;
#else
static constexpr Xbyak::Reg64 regArg0 = rdi;
static constexpr Xbyak::Reg64 regArg1 = rsi;
static constexpr Xbyak::Reg64 regArg2 = rdx;
static constexpr Xbyak::Reg64 regArg3 = rcx;
static constexpr Xbyak::Reg64 regArg4 = r8;
static constexpr Xbyak::Reg64 regArg5 = r9;
#endif

#define GPR_OFFSET(x, jit) ((uintptr_t)&regs.gpr[(x)] - (uintptr_t)jit)
#define REG_OFFSET(kind, jit) ((uintptr_t)&regs.kind - (uintptr_t)jit)
#define CODECACHE_SIZE (2 << 25)
#define CODECACHE_OVERHEAD (CODECACHE_SIZE - 1_kb)

struct JIT : BaseCPU {
  JIT();
  ~JIT() override;
  int Run() override;
  void Reset() override;
  u64 cop2Latch{};
  CodeGenerator code;
  void InvalidatePage(u32);
  void InvalidateCache();
private:
  friend struct n64::Cop1;
  Fn* blockCache[0x80000]{};
  u8* codeCache;
  int instrInBlock = 0;
  u64 sizeUsed = 0;
  std::ofstream dump;

  void* bumpAlloc(u64 size, u8 val = 0);
  void Recompile(Mem&, u32 pc);
  void AllocateOuter(u32 pc);
  void cop2Decode(u32);
  bool special(u32);
  bool regimm(u32);
  bool Exec(Mem&, u32);
};
}
