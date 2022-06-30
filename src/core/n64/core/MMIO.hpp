#pragma once
#include <mmio/VI.hpp>
#include <mmio/MI.hpp>

namespace natsukashii::n64::core {
struct Mem;
struct Registers;

struct MMIO {
  MMIO() = default;
  VI vi;
  MI mi;

  u32 Read(u32);
  void Write(Mem&, Registers& regs, u32, u32);
};
}
