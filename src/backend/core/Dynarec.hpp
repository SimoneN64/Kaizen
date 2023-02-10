#pragma once
#include <xbyak/xbyak.h>
#include <backend/core/Mem.hpp>
#include <fstream>
#include <Registers.hpp>

namespace n64 { struct Cop1; }

namespace n64::JIT {
using namespace Xbyak;
using namespace Xbyak::util;
using Fn = void (*)();

#define GPR_OFFSET(x) ((uintptr_t)&regs.gpr[(x)] - (uintptr_t)&regs)
#define REG_OFFSET(kind) ((uintptr_t)&regs.kind - (uintptr_t)&regs)
#define CODECACHE_SIZE (2 << 25)
#define CODECACHE_OVERHEAD (CODECACHE_SIZE - 1_kb)

struct Dynarec {
  Dynarec();
  ~Dynarec();
  int Step(Mem&);
  void Reset() {
    code.reset();
    regs.Reset();
    InvalidateCache();
  }
  u64 cop2Latch{};
  CodeGenerator code;
  n64::Registers regs;
  void InvalidatePage(u32);
  void InvalidateCache();
private:
  friend struct n64::Cop1;
  Fn* blockCache[0x80000]{};
  u8* codeCache;
  int instrInBlock = 0;
  bool enableBreakpoints = false;
  u64 sizeUsed = 0;
  std::ofstream dump;

  inline void emitBreakpoint() {
    #ifndef NDEBUG
    if(enableBreakpoints)
      code.int3();
    #endif
  }

  void* bumpAlloc(u64 size, u8 val = 0);
  void Recompile(Mem&, u32 pc);
  void AllocateOuter(u32 pc);
  void cop2Decode(u32);
  bool special(u32);
  bool regimm(u32);
  bool Exec(Mem&, u32);
};
}
