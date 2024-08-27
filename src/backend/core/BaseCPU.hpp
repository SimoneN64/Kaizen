#pragma once
#include <Mem.hpp>
#include <Registers.hpp>

namespace n64 {
struct BaseCPU {
  virtual ~BaseCPU() = default;
  virtual int Step() = 0;
  virtual void Reset() = 0;
  virtual bool ShouldServiceInterrupt() = 0;
  virtual void CheckCompareInterrupt() = 0;
  virtual std::vector<u8> Serialize() = 0;
  virtual void Deserialize(const std::vector<u8> &) = 0;
  virtual Mem &GetMem() = 0;
  virtual Registers &GetRegs() = 0;
};
} // namespace n64
