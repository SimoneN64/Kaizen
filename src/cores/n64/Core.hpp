#pragma once
#include <BaseCore.hpp>
#include <n64/core/Cpu.hpp>
#include <n64/core/Mem.hpp>
#include <string>

namespace n64 {
struct Core : BaseCore {
  ~Core() override = default;
  explicit Core(const std::string&);
  void Run() override;
  void PollInputs(u32) override;
  VI& GetVI() { return mem.mmio.vi; }
private:
  Mem mem;
  Cpu cpu;
};
}
