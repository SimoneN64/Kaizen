#include <Core.hpp>

namespace natsukashii::gb::core {
Core::Core(const std::string& rom) {
  mem.LoadROM(rom);
}

void Core::Run() {
  while(true) {
    cpu.Step(mem);
  }
}
}
