#pragma once
#include <SDL_events.h>
#include <Interpreter.hpp>
#include <Mem.hpp>
#include <string>
#include <Dynarec.hpp>

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

  void CpuStep() {
    switch(cpuType) {
      case CpuType::Dynarec: cpuDynarec.Step(mem, regs); break;
      case CpuType::Interpreter: cpuInterp.Step(regs, mem); break;
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
  Dynarec cpuDynarec;
  Registers regs;
};
}
