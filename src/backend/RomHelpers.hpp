#pragma once
#include <MemoryHelpers.hpp>
#include <log.hpp>

namespace Util {
#define Z64 0x80371200
#define N64 0x00123780
#define V64 0x37800012

template <bool toBE = false>
FORCE_INLINE void SwapN64Rom(std::vector<u8> &rom, u32 endianness) {
  u8 altByteShift = 0;
  if (endianness >> 24 != 0x80) {
    if ((endianness & 0xFF) != 0x80) {
      if ((endianness >> 16 & 0xff) != 0x80) {
        panic("TODO: Unrecognized rom endianness. Ideally, this should be more robust");
      } else {
        altByteShift = 12;
      }
    } else {
      altByteShift = 24;
    }
  } else {
    altByteShift = 0;
  }

  endianness &= ~(0xFF << altByteShift);

  switch (endianness) {
  case V64:
    SwapBuffer<u16>(rom);
    if constexpr (!toBE)
      SwapBuffer<u32>(rom);
    break;
  case N64:
    if constexpr (toBE)
      SwapBuffer<u32>(rom);
    break;
  case Z64:
    if constexpr (!toBE)
      SwapBuffer<u32>(rom);
    break;
  default:
    panic("Unrecognized rom format! Make sure this is a valid Nintendo 64 ROM dump!");
  }
}
} // namespace Util
