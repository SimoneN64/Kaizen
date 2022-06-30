#pragma once
#include <BaseCore.hpp>
#include <n64/core/Cpu.hpp>
#include <n64/core/Mem.hpp>
#include <string>

enum class Platform : bool;

namespace natsukashii::n64::core {

struct Core : natsukashii::core::BaseCore {
  ~Core() override = default;
  explicit Core(Platform platform, const std::string&);
  void Run() override;
  void PollInputs(u32) override;
private:
  Mem mem;
  Cpu cpu;
};
}
