#pragma once
#include <log.hpp>
#include <MemoryHelpers.hpp>

namespace util {
enum RomTypes {
  Z64 = 0x80371240,
  N64 = 0x40123780,
  V64 = 0x37804012
};

inline void GetRomCRC(size_t size, u8 *rom, u32 &crc) {
  RomTypes endianness;
  memcpy(&endianness, rom, 4);
  endianness = static_cast<RomTypes>(be32toh(endianness));

  switch (endianness) {
    case RomTypes::V64: {
      SwapBuffer16(size, rom);
      crc = crc32(0, rom, size);
    }
      break;
    case RomTypes::N64: {
      SwapBuffer32(size, rom);
      crc = crc32(0, rom, size);
    }
      break;
    case RomTypes::Z64:
      crc = crc32(0, rom, size);
      break;
    default:
      panic("Unrecognized rom format! Make sure this is a valid Nintendo 64 ROM dump!\n");
  }
}

inline void SwapN64Rom(size_t size, u8 *rom, u32 &crc, u32 &cicChecksum) {
  RomTypes endianness;
  memcpy(&endianness, rom, 4);
  endianness = static_cast<RomTypes>(be32toh(endianness));

  switch (endianness) {
    case RomTypes::V64: {
      u8 *temp = (u8 *) calloc(size, 1);
      memcpy(temp, rom, size);
      SwapBuffer16(size, temp);
      crc = crc32(0, temp, size);
      cicChecksum = crc32(0, &temp[0x40], 0x9c0);
      free(temp);
      SwapBuffer32(size, rom);
      SwapBuffer16(size, rom);
    }
      break;
    case RomTypes::N64: {
      u8 *temp = (u8 *) calloc(size, 1);
      memcpy(temp, rom, size);
      SwapBuffer32(size, temp);
      crc = crc32(0, temp, size);
      cicChecksum = crc32(0, &temp[0x40], 0x9c0);
      free(temp);
    }
      break;
    case RomTypes::Z64:
      crc = crc32(0, rom, size);
      cicChecksum = crc32(0, &rom[0x40], 0x9c0);
      SwapBuffer32(size, rom);
      break;
    default:
      panic("Unrecognized rom format! Make sure this is a valid Nintendo 64 ROM dump!\n");
  }
}
}