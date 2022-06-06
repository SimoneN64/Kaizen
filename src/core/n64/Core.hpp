#pragma once
#include <BaseCore.hpp>
#include <Cpu.hpp>
#include <Mem.hpp>
#include <string>

namespace natsukashii::n64::core {
struct Core : natsukashii::core::BaseCore {
  Core(const std::string&);
  virtual void Run() override;
};
}
