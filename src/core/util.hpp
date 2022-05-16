#pragma once
#include <common.hpp>
#include <type_traits>
#include <cassert>

namespace natsukashii::util {
template <typename ...Args>
constexpr void panic(const std::string& fmt, Args... args) {
  fmt::print(fmt, args...);
  exit(-1);
}

template <u8 start, u8 end>
using BitSliceType =
typename std::conditional<(end - start) <= 7, u8,
  typename std::conditional<(end - start) <= 15, u16,
    typename std::conditional<(end - start) <= 31, u32,
      typename std::conditional<(end - start) <= 63, u64,
        typename std::conditional<(end - start) <= 127, u128, void>::type
      >::type
    >::type
  >::type
>::type;

template <u8 start, u8 end, typename T>
BitSliceType<start, end> BitSlice(const T& num) {
  static_assert(end < (sizeof(T) * 8) && start < (sizeof(T) * 8));
  constexpr auto correctedEnd = end == (sizeof(T) * 8) - 1 ? end : end + 1;
  return (num >> start) & ((1 << correctedEnd) - 1);
}

template <typename T>
T BitSlice(const T& num, int start, int end) {
  assert(end < (sizeof(T) * 8) && start < (sizeof(T) * 8));
  auto correctedEnd = end == (sizeof(T) * 8) - 1 ? end : end + 1;
  return (num >> start) & ((1 << correctedEnd) - 1);
}

}
