#pragma once
#include <common.hpp>
#include <core/mmio/Interrupt.hpp>

namespace n64 {
struct Mem;
struct Registers;

struct AI {
  AI() = default;
  void Reset();
  auto Read(u32) const -> u32;
  void Write(Mem&, Registers&, u32, u32);
  void Step(Mem&, Registers&, u32, float, float);
  bool dmaEnable{};
  u16 dacRate{};
  u8 bitrate{};
  int dmaCount{};
  u32 dmaLen[2]{};
  u32 dmaAddr[2]{};
  bool dmaAddrCarry{};
  u32 cycles{};

  struct {
    u32 freq{44100};
    u32 period{N64_CPU_FREQ / freq};
    u32 precision{16};
  } dac;
};
}