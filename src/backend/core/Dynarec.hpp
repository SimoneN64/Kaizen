#pragma once
#include <xbyak/xbyak.h>
#include <backend/core/Mem.hpp>

namespace n64::JIT {
using Fn = void (*)();

struct Dynarec {
  Dynarec();
  int Step(Mem&, n64::Registers&);
  void Reset() {
    code.reset();
  }
  u64 cop2Latch{};
  Xbyak::CodeGenerator code;
private:
  friend struct Cop1;
  Fn* codeCache[0x80000]{};
  int instrInBlock = 0;

  void Recompile(n64::Registers&, Mem&);
  void AllocateOuter(n64::Registers&);
  void cop2Decode(n64::Registers&, u32);
  bool special(n64::Registers&, u32);
  bool regimm(n64::Registers&, u32);
  bool Exec(n64::Registers&, Mem&, u32);
};
}
