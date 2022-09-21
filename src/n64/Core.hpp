#pragma once
#include <SDL_events.h>
#include <Cpu.hpp>
#include <Mem.hpp>
#include <string>

struct Window;
namespace n64 {
struct Core {
  ~Core() { Stop(); }
  Core();
  void Stop();
  u32 LoadROM(const std::string&);
  void Run(Window&, float volumeL, float volumeR);
  void UpdateController(const u8*);
  void TogglePause() { pause = !pause; }
  VI& GetVI() { return mem.mmio.vi; }

  u32 breakpoint = 0;

  bool pause = true;
  bool romLoaded = false;
  SDL_GameController* gamepad;
  bool gamepadConnected = false;
  bool done = false;
  std::string rom;
  Mem mem;
  Cpu cpu;
};
}
