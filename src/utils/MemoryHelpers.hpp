#pragma once
#include <common.hpp>
#include <cstring>
#include <portable_endian_bswap.h>
#include <log.hpp>

namespace Util {
template<typename T>
inline T ReadAccess(u8 *data, u32 index) {
  if constexpr (sizeof(T) == 1) {
    return data[index];
  } else if constexpr (sizeof(T) == 2 || sizeof(T) == 4) {
    T result = 0;
    memcpy(&result, &data[index], sizeof(T));
    return result;
  } else {
    static_assert(sizeof(T) == 8);
    u32 hi = 0;
    u32 lo = 0;
    memcpy(&hi, &data[index + 0], sizeof(u32));
    memcpy(&lo, &data[index + 4], sizeof(u32));
    T result = ((T) hi << 32) | (T) lo;
    return result;
  }
}

template<typename T>
inline void WriteAccess(u8 *data, u32 index, T val) {
  if constexpr (sizeof(T) == 1) {
    data[index] = val;
    return;
  } else if constexpr (sizeof(T) == 2 || sizeof(T) == 4) {
    memcpy(&data[index], &val, sizeof(T));
  } else {
    static_assert(sizeof(T) == 8);
    u32 hi = val >> 32;
    u32 lo = val;
    memcpy(&data[index + 0], &hi, sizeof(u32));
    memcpy(&data[index + 4], &lo, sizeof(u32));
  }
}

inline void SwapBuffer32(size_t size, u8 *data) {
  for (int i = 0; i < size; i += 4) {
    u32 original = *(u32 *) &data[i];
    *(u32 *) &data[i] = bswap_32(original);
  }
}

inline void SwapBuffer16(size_t size, u8 *data) {
  for (int i = 0; i < size; i += 2) {
    u16 original = *(u16 *) &data[i];
    *(u16 *) &data[i] = bswap_16(original);
  }
}

inline u32 crc32(u32 crc, const u8 *buf, size_t len) {
  static u32 table[256];
  static int have_table = 0;
  u32 rem;
  u8 octet;
  int i, j;
  const u8 *p, *q;

  if (have_table == 0) {
    for (i = 0; i < 256; i++) {
      rem = i;
      for (j = 0; j < 8; j++) {
        if (rem & 1) {
          rem >>= 1;
          rem ^= 0xedb88320;
        } else
          rem >>= 1;
      }
      table[i] = rem;
    }
    have_table = 1;
  }

  crc = ~crc;
  q = buf + len;
  for (p = buf; p < q; p++) {
    octet = *p;  /* Cast to unsigned octet. */
    crc = (crc >> 8) ^ table[(crc & 0xff) ^ octet];
  }
  return ~crc;
}

#ifdef _WIN32
inline void* aligned_alloc(size_t alignment, size_t size) {
  return _aligned_malloc(size, alignment);
}

inline void free(void* ptr) {
  _aligned_free(ptr);
}
#else
inline void* aligned_alloc(size_t alignment, size_t size) {
  return std::aligned_alloc(alignment, size);
}

inline void free(void* ptr) {
  std::free(ptr);
}
#endif
}