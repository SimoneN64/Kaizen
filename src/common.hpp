#pragma once
#include <cstdint>
#include <bitset>

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

#define N64_CPU_FREQ 93750000
#define N64_CYCLES_PER_FRAME(pal) ((N64_CPU_FREQ) / (pal ? 50 : 60))
#define HALF_ADDRESS(addr) ((addr) ^ 2)
#define BYTE_ADDRESS(addr) ((addr) ^ 3)

#define RD(x) (((x) >> 11) & 0x1F)
#define RT(x) (((x) >> 16) & 0x1F)
#define RS(x) (((x) >> 21) & 0x1F)
#define FD(x) (((x) >>  6) & 0x1F)
#define FT(x) RT(x)
#define FS(x) RD(x)
#define BASE(x) RS(x)
#define VT(x) (((x) >> 16) & 0x1F)
#define VS(x) (((x) >> 11) & 0x1F)
#define VD(x) (((x) >>  6) & 0x1F)
#define E1(x) (((x) >>  7) & 0x0F)
#define E2(x) (((x) >> 21) & 0x0F)
#define ELEMENT_INDEX(i) (7 - (i))
#define BYTE_INDEX(i)   (15 - (i))

#define SI_DMA_DELAY (65536 * 2)
#define ARCH_TARGET(...) __attribute__ ((target_clones (__VA_ARGS__)))
#define unlikely(exp) __builtin_expect(exp, 0)
#define likely(exp) __builtin_expect(exp, 1)