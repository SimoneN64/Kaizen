#pragma once
#include <n64/core/mmio/VI.hpp>
#include <n64/core/mmio/MI.hpp>
#include <n64/core/mmio/AI.hpp>
#include <n64/core/mmio/PI.hpp>
#include <n64/core/mmio/RI.hpp>
#include <n64/core/mmio/SI.hpp>
#include <n64/core/RSP.hpp>
#include <n64/core/RDP.hpp>

namespace n64 {
struct Mem;
struct Registers;

struct MMIO {
  MMIO();
  void Reset();
  VI vi;
  MI mi;
  AI ai;
  PI pi;
  RI ri;
  SI si;
  RSP rsp;
  RDP rdp;

  u32 Read(u32);
  void Write(Mem&, Registers& regs, u32, u32);
};
}
