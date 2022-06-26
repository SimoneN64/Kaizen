#pragma once
#include <Cop0.hpp>

namespace natsukashii::n64::core {
struct Registers {
  s64 gpr[32];
  Cop0 cop0;
};
}
