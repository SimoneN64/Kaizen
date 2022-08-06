#pragma once
#include <Cpu.hpp>
#include <Mem.hpp>
#include <string>

namespace n64 {
struct Core {
  ~Core() = default;
  explicit Core(const std::string&);
  void Run();
  void PollInputs(u32);
  VI& GetVI() { return mem.mmio.vi; }
  const u8* GetRDRAM() { return mem.rdram.data(); }
private:
  Mem mem;
  Cpu cpu;
};
}
