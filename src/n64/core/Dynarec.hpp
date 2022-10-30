#pragma once
#include <BaseCpu.hpp>
#include <xbyak/xbyak.h>

namespace n64 {
struct Dynarec : BaseCpu {
  void Step(Mem& mem) override;
  void Reset() override;
private:
  Xbyak::CodeGenerator code;
};
}
