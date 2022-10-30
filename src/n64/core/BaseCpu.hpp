#pragma
#include <core/registers/Registers.hpp>

namespace n64 {
struct BaseCpu {
  virtual ~BaseCpu() {}

  Registers regs;
  virtual void Step(Mem& mem) {}
  virtual void Reset() {}
};
}