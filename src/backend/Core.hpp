#pragma once
#include <SDL2/SDL_events.h>
#include <backend/core/Interpreter.hpp>
#include <backend/core/Mem.hpp>
#include <string>
#include <backend/core/Dynarec.hpp>
#include <backend/core/registers/Registers.hpp>
#include <Debugger.hpp>
#include <SDL2/SDL_timer.h>

struct Window;

namespace n64 {
enum class CpuType {
  Dynarec, Interpreter, NONE
};

struct Core {
  ~Core() { Stop(); }
  Core();
  void Stop();
  CartInfo LoadROM(const std::string&);
  void Run(Window&, float volumeL, float volumeR);
  void TogglePause() { pause = !pause; }
  VI& GetVI() { return cpu->mem.mmio.vi; }

  u32 breakpoint = 0;

  bool pause = true;
  bool isPAL = false;
  bool romLoaded = false;
  bool done = false;
  std::string rom;
  CpuType cpuType = CpuType::NONE;
  std::unique_ptr<BaseCPU> cpu;
  Debugger debugger{*this};
};
}
