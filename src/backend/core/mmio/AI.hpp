#pragma once
#include <common.hpp>
#include <core/mmio/Interrupt.hpp>
#include <core/mmio/Audio.hpp>

namespace n64 {
struct Mem;
struct Registers;

struct AI {
  AI(Mem&, Registers& regs);
  void Reset();
  auto Read(u32) const -> u32;
  void Write(u32, u32);
  void Step(u32, float, float);
  bool dmaEnable{};
  u16 dacRate{};
  u8 bitrate{};
  int dmaCount{};
  u32 dmaLen[2]{};
  u32 dmaAddr[2]{};
  bool dmaAddrCarry{};
  u32 cycles{};
  AudioDevice device;

  struct {
    u32 freq{44100};
    u32 period{N64_CPU_FREQ / freq};
    u32 precision{16};
  } dac;
private:
  Mem& mem;
  Registers& regs;
};
}