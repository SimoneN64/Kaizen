#pragma once
#include <core/mmio/VI.hpp>
#include <core/mmio/MI.hpp>
#include <core/mmio/AI.hpp>
#include <core/mmio/PI.hpp>
#include <core/mmio/RI.hpp>
#include <core/mmio/SI.hpp>
#include <core/RSP.hpp>
#include <core/RDP.hpp>

class ParallelRDP;

namespace n64 {
struct Mem;
struct Registers;

struct MMIO {
  MMIO(Mem& mem, Registers& regs, ParallelRDP& parallel) : vi(mem, regs), mi(regs), ai(mem, regs), pi(mem, regs), si(mem, regs), rsp(mem, regs), rdp(mem, regs, parallel) {}
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
  void Write(u32, u32);
  std::vector<u8> Serialize();
  void Deserialize(const std::vector<u8>&);
};
}
