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
  Dynarec, Interpreter, NONE
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
      case CpuType::Dynarec: cpuDynarec->Reset(); break;
      case CpuType::Interpreter: cpuInterp->Reset(); break;
      case CpuType::NONE: break;
    }
  }

  Registers& CpuGetRegs() {
    switch(cpuType) {
      case CpuType::Dynarec: return cpuDynarec->regs;
      case CpuType::Interpreter: return cpuInterp->regs;
      case CpuType::NONE:
        Util::panic("BRUH\n");
    }
  }

  int CpuStep() {
    switch(cpuType) {
      case CpuType::Dynarec: return cpuDynarec->Step(mem);
      case CpuType::Interpreter: cpuInterp->Step(mem); return 1;
      case CpuType::NONE: return 0;
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
  CpuType cpuType = CpuType::NONE;
  Interpreter* cpuInterp = nullptr;
  JIT::Dynarec* cpuDynarec = nullptr;
};
}
