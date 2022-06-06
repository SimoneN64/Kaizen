#pragma once
#include <Cpu.hpp>
#include <Ppu.hpp>
#include <Mem.hpp>
#include <BaseCore.hpp>

namespace natsukashii::gb::core {
struct Core : natsukashii::core::BaseCore {
  Core(const std::string&);
  void Run() override;
private:
  Mem mem;
  Cpu cpu;
  // Ppu ppu;
};
}
