#pragma once
#include <common.hpp>

namespace natsukashii::core {
struct BaseCore {
  virtual ~BaseCore() = default;
  virtual void Run();
  virtual void PollInputs(u32);
  [[nodiscard]] bool& ShouldQuit() { return quit; }
private:
  bool quit = false;
};
}
