#pragma once
#include <common.hpp>
#include <cstring>
#include <functional>
#include <byteswap.hpp>

namespace Util {
template <typename T>
static FORCE_INLINE std::vector<u8> IntegralToBuffer(const T &val) {
  std::vector<u8> ret{};
  ret.resize(sizeof(T));

  memcpy(ret.data(), &val, sizeof(T));

  return ret;
}

template <typename T>
static constexpr FORCE_INLINE T ReadAccess(const u8 *data, const u32 index) {
  if constexpr (sizeof(T) == 8) {
    u32 hi = *reinterpret_cast<const u32 *>(&data[index + 0]);
    u32 lo = *reinterpret_cast<const u32 *>(&data[index + 4]);
    const T result = static_cast<T>(hi) << 32 | static_cast<T>(lo);
    return result;
  } else {
    return *reinterpret_cast<const T *>(&data[index]);
  }
}

template <typename T>
static constexpr FORCE_INLINE T ReadAccess(const std::vector<u8> &data, const u32 index) {
  if constexpr (sizeof(T) == 8) {
    u32 hi = *reinterpret_cast<const u32 *>(&data[index + 0]);
    u32 lo = *reinterpret_cast<const u32 *>(&data[index + 4]);
    const T result = (static_cast<T>(hi) << 32) | static_cast<T>(lo);
    return result;
  } else {
    return *reinterpret_cast<const T *>(&data[index]);
  }
}

template <typename T, size_t Size>
static constexpr FORCE_INLINE T ReadAccess(const std::array<u8, Size> &data, const u32 index) {
  if constexpr (sizeof(T) == 8) {
    u32 hi = *reinterpret_cast<const u32 *>(&data[index + 0]);
    u32 lo = *reinterpret_cast<const u32 *>(&data[index + 4]);
    const T result = static_cast<T>(hi) << 32 | static_cast<T>(lo);
    return result;
  } else {
    return *reinterpret_cast<const T *>(&data[index]);
  }
}

template <typename T, size_t Size>
static constexpr FORCE_INLINE void WriteAccess(std::array<u8, Size> &data, const u32 index, const T val) {
  if constexpr (sizeof(T) == 8) {
    const u32 hi = val >> 32;
    const u32 lo = val;

    *reinterpret_cast<u32 *>(&data[index + 0]) = hi;
    *reinterpret_cast<u32 *>(&data[index + 4]) = lo;
  } else {
    *reinterpret_cast<T *>(&data[index]) = val;
  }
}

template <typename T>
static constexpr FORCE_INLINE void WriteAccess(std::vector<u8> &data, const u32 index, const T val) {
  if constexpr (sizeof(T) == 8) {
    const u32 hi = val >> 32;
    const u32 lo = val;

    *reinterpret_cast<u32 *>(&data[index + 0]) = hi;
    *reinterpret_cast<u32 *>(&data[index + 4]) = lo;
  } else {
    *reinterpret_cast<T *>(&data[index]) = val;
  }
}

template <typename T>
static constexpr FORCE_INLINE void WriteAccess(u8 *data, const u32 index, const T val) {
  if constexpr (sizeof(T) == 8) {
    const u32 hi = val >> 32;
    const u32 lo = val;

    *reinterpret_cast<u32 *>(&data[index + 0]) = hi;
    *reinterpret_cast<u32 *>(&data[index + 4]) = lo;
  } else {
    *reinterpret_cast<T *>(&data[index]) = val;
  }
}

template <typename T>
static constexpr FORCE_INLINE void SwapBuffer(std::vector<u8> &data) {
  for (size_t i = 0; i < data.size(); i += sizeof(T)) {
    const T original = *reinterpret_cast<T *>(&data[i]);
    *reinterpret_cast<T *>(&data[i]) = bswap(original);
  }
}

template <typename T, size_t Size>
static constexpr FORCE_INLINE void SwapBuffer(std::array<u8, Size> &data) {
  for (size_t i = 0; i < data.size(); i += sizeof(T)) {
    const T original = *reinterpret_cast<T *>(&data[i]);
    *reinterpret_cast<T *>(&data[i]) = bswap(original);
  }
}

static FORCE_INLINE u32 crc32(u32 crc, const u8 *buf, const size_t len) {
  static u32 table[256];
  static int have_table = 0;

  if (have_table == 0) {
    for (int i = 0; i < 256; i++) {
      u32 rem = i;
      for (int j = 0; j < 8; j++) {
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
  for (int i = 0; i < len; i++) {
    const u8 octet = buf[i]; /* Cast to unsigned octet. */
    crc = (crc >> 8) ^ table[(crc & 0xff) ^ octet];
  }
  return ~crc;
}

#ifdef _WIN32
FORCE_INLINE void *aligned_alloc(const size_t alignment, const size_t size) { return _aligned_malloc(size, alignment); }

FORCE_INLINE void aligned_free(void *ptr) { _aligned_free(ptr); }
#else
FORCE_INLINE void *aligned_alloc(const size_t alignment, const size_t size) {
  return std::aligned_alloc(alignment, size);
}

FORCE_INLINE void aligned_free(void *ptr) { std::free(ptr); }
#endif
} // namespace Util
