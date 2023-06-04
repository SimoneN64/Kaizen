#pragma once
#include <SDL2/SDL_events.h>
#include <backend/core/Interpreter.hpp>
#include <backend/core/Mem.hpp>
#include <string>
#include <backend/core/registers/Registers.hpp>
#include <SDL2/SDL_timer.h>

struct Window;

namespace n64 {
struct Core {
  ~Core() { Stop(); }
  Core();
  void Stop();
  void LoadROM(const std::string&);
  void Run(float volumeL, float volumeR);
  void TogglePause() { pause = !pause; }
  VI& GetVI() { return cpu.mem.mmio.vi; }

  u32 breakpoint = 0;

  bool pause = true;
  int cycles = 0;
  bool isPAL = false;
  bool romLoaded = false;
  bool done = false;
  std::string rom;
  Interpreter cpu;
};
}
