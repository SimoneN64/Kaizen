#pragma once
#include <Registers.hpp>

namespace n64 {
struct BaseCPU {
  virtual ~BaseCPU() {}
  virtual void Reset() {}
  virtual int Run() {}
  void RunRSP(int cpuCount) {
    mem.mmio.rsp.Run(cpuCount, regs, mem);
  }
  Registers regs;
  Mem mem;
};
}