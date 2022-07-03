#pragma once
#include <common.hpp>

namespace natsukashii::n64::core {

struct RI {
  RI() = default;
  u32 mode{0xE}, config{0x40}, select{0x14}, refresh{0x63634};
  auto Read(u32) const -> u32;
  void Write(u32, u32);
};

}