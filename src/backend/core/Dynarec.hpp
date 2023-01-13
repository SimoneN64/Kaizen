#pragma once
#include <xbyak/xbyak.h>
#include <backend/core/Mem.hpp>
#include <fstream>

namespace n64::JIT {
using namespace Xbyak;
using namespace Xbyak::util;
using Fn = void (*)();

#define GPR_OFFSET(x) ((uintptr_t)&regs.gpr[(x)] - (uintptr_t)&regs)
#define REG_OFFSET(kind) ((uintptr_t)&regs.kind - (uintptr_t)&regs)
#define CODECACHE_SIZE (2 << 25)

struct Dynarec {
  Dynarec();
  ~Dynarec();
  int Step(Mem&, n64::Registers&);
  void Reset() {
    code.reset();
    InvalidateCache();
  }
  u64 cop2Latch{};
  CodeGenerator code;
  void InvalidatePage(u32);
  void InvalidateCache();
private:
  friend struct Cop1;
  Fn* blockCache[0x80000]{};
  u8* codeCache;
  int instrInBlock = 0;
  bool enableBreakpoints = false;
  u64 sizeUsed = 0;

  inline void emitBreakpoint() {
    #ifndef NDEBUG
    if(enableBreakpoints)
      code.int3();
    #endif
  }

  void* bumpAlloc(u64 size, u8 val = 0);
  void Recompile(n64::Registers&, Mem&, u32 pc);
  void AllocateOuter(u32 pc);
  void cop2Decode(n64::Registers&, u32);
  bool special(n64::Registers&, u32);
  bool regimm(n64::Registers&, u32);
  bool Exec(n64::Registers&, Mem&, u32);
};
}
