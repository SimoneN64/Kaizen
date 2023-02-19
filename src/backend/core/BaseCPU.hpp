#pragma once
#include <Registers.hpp>

namespace n64 {
struct BaseCPU {
  virtual ~BaseCPU() {}
  virtual void Reset() {}
  virtual int Run() {}
  Registers regs;
  Mem mem;
};
}