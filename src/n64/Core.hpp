#pragma once
#include <SDL2/SDL_events.h>
#include <Cpu.hpp>
#include <Mem.hpp>
#include <string>

struct Window;
namespace n64 {
struct Core {
  ~Core() = default;
  Core();
  void Reset();
  void LoadROM(const std::string&);
  void Run(Window&);
  void UpdateController(const u8*);
  VI& GetVI() { return mem.mmio.vi; }
  bool romLoaded = false;
  SDL_GameController* gamepad;
  bool gamepadConnected = false;
private:
  friend struct ::Window;
  Mem mem;
  Cpu cpu;
};
}
