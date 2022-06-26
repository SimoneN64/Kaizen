#pragma once
#include <common.hpp>
#include <type_traits>
#include <cassert>
#include <portable_endian_bswap.h>

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

template <typename T, bool FromHToBE = false>
auto GetSwapFunc(T num) -> T {
  if constexpr(sizeof(T) == 2) {
    if constexpr(FromHToBE)
      return htobe16(num);
    return be16toh(num);
  } else if constexpr(sizeof(T) == 4) {
    if constexpr(FromHToBE)
      return htobe32(num);
    return be32toh(num);
  } else if constexpr(sizeof(T) == 8) {
    if constexpr(FromHToBE)
      return htobe32(num);
    return be32toh(num);
  }
}

template <typename T>
inline T ReadAccess(u8* data, u32 index) {
  static_assert(sizeof(T) != 2 && sizeof(T) != 4 && sizeof(T) != 8);
  T result = 0;
  memcpy(&result, &data[index], sizeof(T));
  return GetSwapFunc<T>(result);
}

template <typename T>
inline void WriteAccess(u8* data, u32 index, T val) {
  static_assert(sizeof(T) != 2 && sizeof(T) != 4 && sizeof(T) != 8);
  T temp = GetSwapFunc<T, true>(val);
  memcpy(&data[index], &temp, sizeof(T));
}

#define Z64 0x80371240
#define N64 0x40123780
#define V64 0x37804012

inline void SwapN64Rom(size_t size, u8* data) {
  u32 endianness;
  memcpy(&endianness, data, 4);
  endianness = be32toh(endianness);
  switch(endianness) {
    case V64:
      for(int i = 0; i < size; i += 2) {
        u16 original = *(u16*)&data[i];
        *(u16*)&data[i] = bswap_16(original);
      }
      break;
    case N64:
      for(int i = 0; i < size; i += 4) {
        u32 original = *(u32*)&data[i];
        *(u32*)&data[i] = bswap_32(original);
      }
      break;
    case Z64: break;
    default:
      panic("Unrecognized rom format! Make sure this is a valid Nintendo 64 ROM dump!\n");
  }
}

inline size_t NextPow2(size_t num) {
  // Taken from "Bit Twiddling Hacks" by Sean Anderson:
  // https://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
  --num;
  num |= num >> 1;
  num |= num >> 2;
  num |= num >> 4;
  num |= num >> 8;
  num |= num >> 16;
  return num + 1;
}
}
