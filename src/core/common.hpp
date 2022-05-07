#pragma once
#include <cstdint>
#include <bitset>
#include <emmintrin.h>

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using s8 = int8_t;
using s16 = int16_t;
using s32 = int32_t;
using s64 = int64_t;
using u128 = __uint128_t;
using s128 = __int128_t;
using m128 = __m128i;

#define UINT128_MAX ((u128)0xFFFF'FFFF'FFFF'FFFF << 64) | 0xFFFF'FFFF'FFFF'FFFF
#define UINT128_MIN 0
#define INT128_MAX ((u128)0x7FFF'FFFF'FFFF'FFFF << 64) | 0xFFFF'FFFF'FFFF'FFFF
#define INT128_MIN (-INT128_MAX - 1LL)

template <u8 start, u8 end>
using BitSliceType =
std::conditional<(end - start) <= 7, u8,
  std::conditional<(end - start) <= 15, u16,
    std::conditional<(end - start) <= 31, u32,
      std::conditional<(end - start) <= 63, u64,
        std::conditional<(end - start) <= 127, u128, u128>
      >
    >
  >
>;
