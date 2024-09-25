#pragma once
#include <common.hpp>

static FORCE_INLINE u16 bswap(const u16 x) { return (x & 0xFF00u) >> 8 | (x & 0x00FFu) << 8; }

static FORCE_INLINE u32 bswap(const u32 x) {
  return (x & 0xFF000000u) >> 24u | (x & 0x00FF0000u) >> 8u | (x & 0x0000FF00u) << 8u | (x & 0x000000FFu) << 24u;
}

static FORCE_INLINE u64 bswap(const u64 x) {
  return (x & 0xFF00000000000000u) >> 56u | (x & 0x00FF000000000000u) >> 40u | (x & 0x0000FF0000000000u) >> 24u |
    (x & 0x000000FF00000000u) >> 8u | (x & 0x00000000FF000000u) << 8u | (x & 0x0000000000FF0000u) << 24u |
    (x & 0x000000000000FF00u) << 40u | (x & 0x00000000000000FFu) << 56u;
}
