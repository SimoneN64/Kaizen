#pragma once
#include <xbyak/xbyak.h>
#include <Mem.hpp>
#include <Registers.hpp>

namespace n64 {
struct Dynarec {
  void Step(Mem& mem, Registers& regs);
  void Reset();
  Registers regs;
private:
  Xbyak::CodeGenerator code;
};
}
