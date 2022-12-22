#pragma once
#include <common.hpp>
#include <core/mmio/Interrupt.hpp>

namespace n64 {

struct Mem;
struct Registers;

struct PI {
  PI();
  void Reset();
  auto Read(MI&, u32) const -> u32;
  void Write(Mem&, Registers&, u32, u32);
  u32 dramAddr{}, cartAddr{};
  u32 rdLen{}, wrLen{};
  u32 stub[8]{};
};
}