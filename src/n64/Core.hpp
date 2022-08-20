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
  void Stop();
  void Reset();
  void LoadROM(const std::string&);
  void Run(Window&);
  void UpdateController(const u8*);
  void TogglePause() { pause = !pause; }
  VI& GetVI() { return mem.mmio.vi; }
  bool pause = true;
  bool romLoaded = false;
  SDL_GameController* gamepad;
  bool gamepadConnected = false;
  bool done = false;
  std::string rom;
private:
  friend struct ::Window;
  Mem mem;
  Cpu cpu;
};
}
