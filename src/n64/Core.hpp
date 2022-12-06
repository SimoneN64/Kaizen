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
      case CpuType::Dynarec: cpuDynarec.Reset(); break;
      case CpuType::Interpreter: cpuInterp.Reset(); break;
    }
  }

  void CpuStep(Mem& mem) {
    switch(cpuType) {
      case CpuType::Dynarec: cpuDynarec.Step(mem); break;
      case CpuType::Interpreter: cpuInterp.Step(mem); break;
    }
  }

  Registers& CpuGetRegs() {
    switch(cpuType) {
      case CpuType::Dynarec: return cpuDynarec.regs;
      case CpuType::Interpreter: return cpuInterp.regs;
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
};
}
