#pragma once
#include <log.hpp>
#include <MemoryHelpers.hpp>

namespace Util {
#define Z64 0x80371240
#define N64 0x40123780
#define V64 0x37804012

template <bool toBE = false>
inline void SwapN64Rom(size_t size, u8 *rom, u32 endianness) {
  switch (endianness) {
    case V64:
      SwapBuffer16(size, rom);
      if constexpr(!toBE)
        SwapBuffer32(size, rom);
      break;
    case N64:
      if constexpr(toBE)
        SwapBuffer32(size, rom);
      break;
    case Z64:
      if constexpr(!toBE)
        SwapBuffer32(size, rom);
      break;
    default:
      panic("Unrecognized rom format! Make sure this is a valid Nintendo 64 ROM dump!\n");
  }
}
}