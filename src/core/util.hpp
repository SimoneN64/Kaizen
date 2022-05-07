#pragma once
#include <common.hpp>
#include <type_traits>

namespace natsukashii::util {
template <u8 start, u8 end, typename T>
BitSliceType<start, end> BitSlice(const T& num) {
  static_assert(end - start > 127);
  static_assert(end - start > sizeof(T));
  return (num >> start) & ((1 << end) - 1);
}
}
