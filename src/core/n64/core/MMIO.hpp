#pragma once
#include <n64/core/mmio/VI.hpp>
#include <n64/core/mmio/MI.hpp>
#include <n64/core/RSP.hpp>
#include <n64/core/RDP.hpp>

namespace natsukashii::n64::core {
struct Mem;
struct Registers;

struct MMIO {
  MMIO() = default;
  VI vi;
  MI mi;
  RSP rsp;
  RDP rdp;

  u32 Read(u32);
  void Write(Mem&, Registers& regs, u32, u32);
};
}
