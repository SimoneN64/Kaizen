#include <Core.hpp>

namespace natsukashii::core {
Core::Core() {}

void Core::Run() {
  while(true) {
    cpu.Step(mem);
  }
}
}
