#include <Core.hpp>

namespace n64 {
void Interpreter::Reset() {
  regs.Reset();
  mem.Reset();
}
}