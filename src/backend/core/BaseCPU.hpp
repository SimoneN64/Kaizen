#pragma once
#include <Registers.hpp>
#include <Mem.hpp>

namespace n64 {
struct BaseCPU {
  virtual ~BaseCPU() = default;
  virtual int Step() = 0;
  virtual void Reset() {
    regs.Reset();
    mem.Reset();
  }
  virtual bool ShouldServiceInterrupt() = 0;
  virtual void CheckCompareInterrupt() = 0;
  virtual std::vector<u8> Serialize() = 0;
  virtual void Deserialize(const std::vector<u8>&) = 0;
  Registers regs;
  Mem mem{regs};
};
}