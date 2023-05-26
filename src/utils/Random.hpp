#pragma once
#include <random>

namespace Util {
template <class T>
inline T GetRandomNumber() {
  std::random_device r;
  std::default_random_engine el(r());
  std::uniform_int_distribution<T> ud;
  return ud(el);
}
}