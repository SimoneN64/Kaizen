#pragma once
#include <SDL2/SDL_events.h>
#include <backend/core/Interpreter.hpp>
#include <backend/core/Mem.hpp>
#include <string>
#include <backend/core/Dynarec.hpp>
#include <backend/core/registers/Registers.hpp>

struct Window;

namespace n64 {
enum class CpuType {
  Dynarec, Interpreter
};

struct Core {
  ~Core() { Stop(); }
  Core();
  void Stop();
  CartInfo LoadROM(const std::string&);
  void Run(Window&, float volumeL, float volumeR);
  void UpdateController(const u8*);
  void TogglePause() { pause = !pause; }
  VI& GetVI() { return mem.mmio.vi; }

  void CpuReset() {
    switch(cpuType) {
      case CpuType::Dynarec: break;
      case CpuType::Interpreter: break;
    }
  }

  int CpuStep() {
    switch(cpuType) {
      case CpuType::Dynarec: return cpuDynarec.Step(mem, regs);
      case CpuType::Interpreter: cpuInterp.Step(mem, regs); return 1;
    }
  }

  u32 breakpoint = 0;
  int cycles = 0;

  bool pause = true;
  bool romLoaded = false;
  SDL_GameController* gamepad;
  bool gamepadConnected = false;
  bool done = false;
  std::string rom;
  Mem mem;
  CpuType cpuType = CpuType::Dynarec;
  Interpreter cpuInterp;
  JIT::Dynarec cpuDynarec;
  Registers regs;
};
}
