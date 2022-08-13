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
  void Reset();
  void LoadROM(const std::string&);
  void Run(Window&);
  void PollInputs(SDL_Event);
  VI& GetVI() { return mem.mmio.vi; }
  u8* GetRDRAM() { return mem.rdram.data(); }
  bool romLoaded = false;
private:
  Mem mem;
  Cpu cpu;
};
}
