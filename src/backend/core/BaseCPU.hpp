#pragma once
#include <Registers.hpp>
#include <Mem.hpp>

namespace n64 {
struct BaseCPU {
  virtual ~BaseCPU() = default;
  virtual int Step() {return 0;}
  virtual void Reset() {
    regs.Reset();
    mem.Reset();
  }
  virtual bool ShouldServiceInterrupt() {return false;}
  virtual void CheckCompareInterrupt() {}
  Registers regs;
  Mem mem;
};
}