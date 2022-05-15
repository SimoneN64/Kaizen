#pragma once
#include "Cpu.hpp"
#include "Ppu.hpp"

namespace natsukashii::core {
struct Core {
  Core();
  void Run();
private:
  Cpu cpu;
  // Ppu ppu;
};
}
