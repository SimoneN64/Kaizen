#pragma once
#include <core/mmio/VI.hpp>
#include <core/mmio/MI.hpp>
#include <core/mmio/AI.hpp>
#include <core/mmio/PI.hpp>
#include <core/mmio/RI.hpp>
#include <core/mmio/SI.hpp>
#include <core/RSP.hpp>
#include <core/RDP.hpp>

namespace n64 {
struct Mem;
struct Registers;

struct MMIO {
  MMIO() = default;
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
  void Write(Mem&, Registers&, u32, u32);
};
}
