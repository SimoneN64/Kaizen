#pragma once
#include <common.hpp>

namespace natsukashii::core {
struct Scheduler {
  Scheduler();
private:
  u128 currentTime = 0;
};
}