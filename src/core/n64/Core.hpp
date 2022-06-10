#pragma once
#include <BaseCore.hpp>
#include <Cpu.hpp>
#include <Mem.hpp>
#include <string>

namespace natsukashii::n64::core {
using namespace natsukashii::core;
struct Core : BaseCore {
  ~Core() override = default;
  explicit Core(const std::string&);
  void Run() override;
};
}
