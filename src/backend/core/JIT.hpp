#pragma once
#include <xbyak/xbyak.h>
#include <backend/core/Mem.hpp>
#include <fstream>
#include <BaseCPU.hpp>
#include <capstone/capstone.h>

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
#define JIT_THIS rcx
#define INSTR rdx
#define regArg0 JIT_THIS
#define regArg1 INSTR
#define regArg2 r8
#define regArg3 r9
#else
#define JIT_THIS rdi
#define INSTR rsi
#define regArg0 JIT_THIS
#define regArg1 INSTR
#define regArg2 rdx
#define regArg3 rcx
#define regArg4 r8
#define regArg5 r9
#endif
#define regScratch0 rax
#define regScratch1 r10
#define regScratch2 r11

#define GPR_OFFSET(x, jit) ((uintptr_t)&regs.gpr[(x)] - (uintptr_t)jit)
#define REG_OFFSET(kind, jit) ((uintptr_t)&regs.kind - (uintptr_t)jit)
#define THIS_OFFSET(kind, jit) ((uintptr_t)&kind - (uintptr_t)jit)
#define CODECACHE_SIZE (2 << 25)
#define CODECACHE_OVERHEAD (CODECACHE_SIZE - 1_kb)

struct JIT : BaseCPU {
  JIT();
  ~JIT() override;
  int Run() override;
  void Reset() override;
  u64 cop2Latch{};
  std::unique_ptr<CodeGenerator> code{};
  void InvalidatePage(u32);
  void InvalidateCache();
private:
  friend struct n64::Cop1;
  csh capstoneHandle{};
  Fn* blockCache[0x80000]{};
  u8* codeCache;
  int instrInBlock = 0;
  u64 sizeUsed = 0;
  u64 prevSize = 0;

  inline void epilogue() {
#ifdef ABI_MSVC
    code->add(rsp, 8);
#else
    code->pop(rbp);
#endif
    code->ret();
  }

  inline void prologue() {
#ifdef ABI_MSVC
    code->sub(rsp, 8);
#else
    code->push(rbp);
    code->mov(rbp, rsp);
#endif
  }

  void* bumpAlloc(u64 size, u8 val = 0);
  void Recompile(Mem&, u32 pc);
  void AllocateOuter(u32 pc);
  void cop2Decode(u32);
  void special(u32);
  void regimm(u32);
  void Emit(u32);
  bool isEndBlock(u32 instr);
};
}
