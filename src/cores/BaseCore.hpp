#pragma once
#include <common.hpp>

enum class System {
  GameBoy, Nintendo64
};

struct BaseCore {
  virtual ~BaseCore() = default;
  virtual void Run() {};
  virtual void PollInputs(u32) {};
  [[nodiscard]] bool& ShouldQuit() { return quit; }
  bool initialized = false;
  System system;
private:
  bool quit = false;
};