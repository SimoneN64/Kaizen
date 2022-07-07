#pragma once
#include <common.hpp>
#include <n64/core/mmio/Interrupt.hpp>

namespace n64 {
struct Mem;
struct Registers;

struct AI {
  AI() = default;
  auto Read(u32) const -> u32;
  void Write(Mem&, Registers&, u32, u32);
  void Step(Mem&, Registers&, int);
  bool dmaEnable{};
  u16 dacRate{};
  u8 bitrate{};
  int dmaCount{};
  u32 dmaLen[2]{};
  u32 dmaAddr[2]{};
  bool dmaAddrCarry{};
  int cycles{};

  struct {
    u32 freq{44100};
    u32 period{N64_CPU_FREQ / freq};
    u32 precision{16};
  } dac;
};
}