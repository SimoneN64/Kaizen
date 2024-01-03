#pragma once
#include <SDL2/SDL_events.h>
#include <backend/core/Interpreter.hpp>
#include <backend/core/JIT.hpp>
#include <string>
#include <SDL2/SDL_timer.h>

struct Window;
struct Event;

namespace n64 {
struct Core {
  ~Core() { Stop(); }
  Core();
  void Stop();
  void LoadROM(const std::string&);
  void Run(float volumeL, float volumeR);
  void Serialize();
  void Deserialize();
  void TogglePause() { pause = !pause; }
  [[nodiscard]] VI& GetVI() const { return cpu->mem.mmio.vi; }

  u32 breakpoint = 0;

  bool pause = true;
  bool render = true;
  u32 cycles = 0;
  bool romLoaded = false;
  std::string rom;
  std::unique_ptr<BaseCPU> cpu;
  std::vector<u8> serialized[10]{};
  size_t memSize{}, cpuSize{}, verSize{};
  int slot = 0;
};

extern u32 extraCycles;
void CpuStall(u32 cycles);
u32 PopStalledCycles();
}
