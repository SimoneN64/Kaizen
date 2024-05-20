#pragma once
#include <common.hpp>
#include <cstring>
#include <portable_endian_bswap.h>
#include <log.hpp>
#include <functional>

namespace Util {
template <class T, typename... Args>
struct AutoRelease {
  AutoRelease(void (*dtor)(T*), const char* name = "") : dtor(dtor), name(name) { }

  AutoRelease(T* (*ctor)(Args...), void (*dtor)(T*), const char* name = "", Args... args) : dtor(dtor), name(name) {
    thing = ctor(args...);
  }

  T* get() {
    if (!thing) {
      Util::panic("AutoRelease::{} is null!", name);
    }
    return thing;
  }

  void Construct(T* (*ctor)(Args...), Args... args) {
    if(thing) {
      dtor(thing);
    }

    thing = ctor(args...);
  }

  void Destroy() {
    if(thing) {
      dtor(thing);
    }
  }

  ~AutoRelease() {
    if(thing) {
      dtor(thing);
    }
  }
private:
  const char* name = "";
  T* thing = nullptr;
  void (*dtor)(T*) = nullptr;
};

template<typename T>
static FORCE_INLINE T ReadAccess(const u8* data, u32 index) {
  if constexpr (sizeof(T) == 8) {
    u32 hi = *reinterpret_cast<const u32*>(&data[index + 0]);
    u32 lo = *reinterpret_cast<const u32*>(&data[index + 4]);
    T result = ((T)hi << 32) | (T)lo;
    return result;
  } else {
    return *reinterpret_cast<const T*>(&data[index]);
  }
}

template<typename T>
static FORCE_INLINE T ReadAccess(const std::vector<u8>& data, u32 index) {
  if constexpr (sizeof(T) == 8) {
    u32 hi = *reinterpret_cast<const u32*>(&data[index + 0]);
    u32 lo = *reinterpret_cast<const u32*>(&data[index + 4]);
    T result = ((T)hi << 32) | (T)lo;
    return result;
  } else {
    return *reinterpret_cast<const T*>(&data[index]);
  }
}

template<typename T, size_t Size>
static FORCE_INLINE T ReadAccess(const std::array<u8, Size>& data, u32 index) {
  if constexpr (sizeof(T) == 8) {
    u32 hi = *reinterpret_cast<const u32*>(&data[index + 0]);
    u32 lo = *reinterpret_cast<const u32*>(&data[index + 4]);
    T result = ((T)hi << 32) | (T)lo;
    return result;
  } else {
    return *reinterpret_cast<const T*>(&data[index]);
  }
}

template<typename T, size_t Size>
static FORCE_INLINE void WriteAccess(std::array<u8, Size>& data, u32 index, T val) {
  if constexpr (sizeof(T) == 8) {
    u32 hi = val >> 32;
    u32 lo = val;

    *reinterpret_cast<u32*>(&data[index + 0]) = hi;
    *reinterpret_cast<u32*>(&data[index + 4]) = lo;
  } else {
    *reinterpret_cast<T*>(&data[index]) = val;
  }
}

template<typename T>
static FORCE_INLINE void WriteAccess(std::vector<u8>& data, u32 index, T val) {
  if constexpr (sizeof(T) == 8) {
    u32 hi = val >> 32;
    u32 lo = val;

    *reinterpret_cast<u32*>(&data[index + 0]) = hi;
    *reinterpret_cast<u32*>(&data[index + 4]) = lo;
  } else {
    *reinterpret_cast<T*>(&data[index]) = val;
  }
}

template<typename T>
static FORCE_INLINE void WriteAccess(u8 *data, u32 index, T val) {
  if constexpr (sizeof(T) == 8) {
    u32 hi = val >> 32;
    u32 lo = val;

    *reinterpret_cast<u32*>(&data[index + 0]) = hi;
    *reinterpret_cast<u32*>(&data[index + 4]) = lo;
  } else {
    *reinterpret_cast<T*>(&data[index]) = val;
  }
}

FORCE_INLINE void SwapBuffer32(std::vector<u8> &data) {
  for (size_t i = 0; i < data.size(); i += 4) {
    u32 original = *(u32 *) &data[i];
    *(u32 *) &data[i] = bswap_32(original);
  }
}

FORCE_INLINE void SwapBuffer16(std::vector<u8> &data) {
  for (size_t i = 0; i < data.size(); i += 2) {
    u16 original = *(u16 *) &data[i];
    *(u16 *) &data[i] = bswap_16(original);
  }
}

template <size_t Size>
FORCE_INLINE void SwapBuffer32(std::array<u8, Size> &data) {
  for (size_t i = 0; i < Size; i += 4) {
    u32 original = *(u32 *) &data[i];
    *(u32 *) &data[i] = bswap_32(original);
  }
}

template <size_t Size>
FORCE_INLINE void SwapBuffer16(std::array<u8, Size> &data) {
  for (size_t i = 0; i < Size; i += 2) {
    u16 original = *(u16 *) &data[i];
    *(u16 *) &data[i] = bswap_16(original);
  }
}

FORCE_INLINE u32 crc32(u32 crc, const u8 *buf, size_t len) {
  static u32 table[256];
  static int have_table = 0;
  u32 rem;
  u8 octet;

  if (have_table == 0) {
    for (int i = 0; i < 256; i++) {
      rem = i;
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
    octet = buf[i];  /* Cast to unsigned octet. */
    crc = (crc >> 8) ^ table[(crc & 0xff) ^ octet];
  }
  return ~crc;
}

#ifdef _WIN32
FORCE_INLINE void* aligned_alloc(size_t alignment, size_t size) {
  return _aligned_malloc(size, alignment);
}

FORCE_INLINE void aligned_free(void* ptr) {
  _aligned_free(ptr);
}
#else
FORCE_INLINE void* aligned_alloc(size_t alignment, size_t size) {
  return std::aligned_alloc(alignment, size);
}

FORCE_INLINE void aligned_free(void* ptr) {
  std::free(ptr);
}
#endif
}