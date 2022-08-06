#pragma once
#include <common.hpp>
#include <n64/core/mmio/Interrupt.hpp>

namespace n64 {

struct Mem;
struct Registers;

struct PI {
  PI() = default;
  auto Read(MI&, u32) const -> u32;
  void Write(Mem&, Registers&, u32, u32);
  u32 dramAddr{}, cartAddr{};
  u32 rdLen{}, wrLen{};
  u32 status{};
  u32 stub[8]{};
};
}