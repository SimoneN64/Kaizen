#pragma once
#include <Cpu.hpp>
#include <Ppu.hpp>
#include <Mem.hpp>
#include <BaseCore.hpp>

namespace natsukashii::gb::core {
using namespace natsukashii::core;
struct Core : BaseCore {
  ~Core() override = default;
  explicit Core(const std::string&);
  void Run() override;
  void PollInputs(u32) override;
private:
  Mem mem;
  Cpu cpu;
  // Ppu ppu;
};
}
