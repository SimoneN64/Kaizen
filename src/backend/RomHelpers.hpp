#pragma once
#include <log.hpp>
#include <MemoryHelpers.hpp>

namespace Util {
#define Z64 0x80371240
#define N64 0x40123780
#define V64 0x37804012
#define Z64_ALT 0x80371241
#define N64_ALT 0x41123780
#define V64_ALT 0x37804112

template <bool toBE = false>
FORCE_INLINE void SwapN64Rom(size_t size, u8 *rom, u32 endianness) {
  switch (endianness) {
    case V64: case V64_ALT:
      SwapBuffer16(size, rom);
      if constexpr(!toBE)
        SwapBuffer32(size, rom);
      break;
    case N64: case N64_ALT:
      if constexpr(toBE)
        SwapBuffer32(size, rom);
      break;
    case Z64: case Z64_ALT:
      if constexpr(!toBE)
        SwapBuffer32(size, rom);
      break;
    default:
      panic("Unrecognized rom format! Make sure this is a valid Nintendo 64 ROM dump!");
  }
}
}