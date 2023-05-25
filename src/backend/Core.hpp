#pragma once
#include <SDL2/SDL_events.h>
#include <backend/core/Interpreter.hpp>
#include <backend/core/CachedInterpreter.hpp>
#include <backend/core/JIT.hpp>
#include <backend/core/Mem.hpp>
#include <string>
#include <backend/core/registers/Registers.hpp>
#include <Debugger.hpp>
#include <SDL2/SDL_timer.h>

struct Window;

namespace n64 {
enum class CpuType {
  Interpreter, CachedInterpreter, JIT, COUNT
};

struct Core {
  ~Core() { Stop(); }
  Core();
  void Stop();
  void LoadROM(const std::string&);
  void Run(float volumeL, float volumeR) const;
  void TogglePause() { pause = !pause; }
  [[nodiscard]] VI& GetVI() const { return cpu->mem.mmio.vi; }

  u32 breakpoint = 0;

  bool pause = true;
  bool isPAL = false;
  bool romLoaded = false;
  bool done = false;
  std::string rom;
  CpuType cpuType = CpuType::COUNT;
  std::unique_ptr<BaseCPU> cpu;
  Debugger debugger{*this};
};
}
