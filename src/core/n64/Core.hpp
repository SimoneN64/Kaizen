#pragma once
#include <BaseCore.hpp>
#include <n64/core/Cpu.hpp>
#include <n64/core/Mem.hpp>
#include <string>

namespace natsukashii::n64::core {
using namespace natsukashii::core;
struct Core : BaseCore {
  ~Core() override = default;
  explicit Core(const std::string&);
  void Run() override;
  void PollInputs(u32) override;
private:
  Mem mem;
  Cpu cpu;
};
}
