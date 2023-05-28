#pragma once
#include <core/registers/Registers.hpp>
#include <Mem.hpp>
#include <vector>
#include <BaseCPU.hpp>
#include <functional>

namespace n64 {
struct Core;
struct CachedInterpreter;
using CachedFn = void(*)(CachedInterpreter&, u32);

struct InstructionHandler {
  InstructionHandler() = default;
  CachedFn func;
  u32 instr;

  void operator()(CachedInterpreter& cpu) const { func(cpu, instr); }

  explicit operator bool() const noexcept { return func != nullptr; }
};

struct CachedInterpreter : BaseCPU {
  CachedInterpreter() = default;
  ~CachedInterpreter() override = default;
  int Run() override;
  void Reset() override;
  void InvalidateCache() {
    for(auto &i : instrCache) {
      if(i)
        free(i);
    }
  }

  u64 cop2Latch{};
  InstructionHandler* instrCache[0x80000]{};
  void AllocateOuter(u32 pc) {
    instrCache[pc >> 20] = (InstructionHandler *)calloc(0x1000, sizeof(InstructionHandler));
  }

  void InvalidatePage(u32 paddr) {
    if(instrCache[paddr >> 20])
      free(instrCache[paddr >> 20]);
  }

  CachedFn cop2GetFunc(u32);
  CachedFn special(u32);
  CachedFn regimm(u32);
  CachedFn GetInstrFunc(u32);
};
}
