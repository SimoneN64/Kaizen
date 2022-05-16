#pragma once
#include "Cpu.hpp"
#include "Ppu.hpp"
#include "Mem.hpp"

namespace natsukashii::core {
struct Core {
  Core();
  void Run();
private:
  Mem mem;
  Cpu cpu;
  // Ppu ppu;
};
}
