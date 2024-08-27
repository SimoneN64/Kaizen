#pragma once
#include <common.hpp>

namespace n64 {

struct RI {
  RI();
  void Reset();
  auto Read(u32) const -> u32;
  void Write(u32, u32);
  u32 mode{0xE}, config{0x40}, select{0x14}, refresh{0x63634};
};

} // namespace n64
