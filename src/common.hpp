#pragma once
#include <cstdint>
#include <bitset>
#include <emmintrin.h>
#include <fmt/format.h>
#include <fmt/color.h>

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

#define UINT128_MAX (((u128)0xFFFF'FFFF'FFFF'FFFF << 64) | 0xFFFF'FFFF'FFFF'FFFF)
#define UINT128_MIN 0
#define INT128_MAX (((u128)0x7FFF'FFFF'FFFF'FFFF << 64) | 0xFFFF'FFFF'FFFF'FFFF)
#define INT128_MIN (-(INT128_MAX) - 1LL)
#define KiB * 1024
#define MiB ((KiB) * 1024)
#define GiB ((MiB) * 1024)
#define N64_CPU_FREQ 93750000
#define N64_CYCLES_PER_FRAME ((N64_CPU_FREQ) / 60)
#define HALF_ADDRESS(addr) ((addr) ^ 2)
#define BYTE_ADDRESS(addr) ((addr) ^ 3)
#define ASPECT_RATIO ((float)4/3)

#define RD(x) (((x) >> 11) & 0x1F)
#define RT(x) (((x) >> 16) & 0x1F)
#define RS(x) (((x) >> 21) & 0x1F)
#define FD(x) (((x) >>  6) & 0x1F)
#define FT(x) RT(x)
#define FS(x) RD(x)
#define BASE(x) RS(x)