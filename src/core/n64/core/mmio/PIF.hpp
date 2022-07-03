#pragma once
#include <common.hpp>

namespace natsukashii::n64::core {

union Controller {
  struct {
    u8 b1, b2;
    s8 b3, b4;
  } __attribute__((__packed__));
  u32 raw;
};

static_assert(sizeof(Controller) == 4);

struct Mem;

void ProcessPIFCommands(u8*, Controller&, Mem&);
}