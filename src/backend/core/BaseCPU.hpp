#pragma once
#include <Registers.hpp>
#include <Mem.hpp>

namespace n64 {
struct BaseCPU {
  virtual ~BaseCPU() = default;
  virtual int Step() {return 0;}
  virtual void Reset() {}
  Registers regs;
  Mem mem;
};
}