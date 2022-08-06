#pragma once
#include <Cpu.hpp>
#include <Mem.hpp>
#include <string>

namespace n64 {
struct Core {
  ~Core() = default;
  Core() = default;
  void LoadROM(const std::string&);
  void Run();
  void PollInputs(u32);
  VI& GetVI() { return mem.mmio.vi; }
  const u8* GetRDRAM() const { return mem.rdram.data(); }
  bool initialized = false;
private:
  Mem mem;
  Cpu cpu;
};
}
