#pragma once
#include <common.hpp>
#include <type_traits>
#include <cassert>
#include <portable_endian_bswap.h>
#include <fstream>
#include <vector>
#include <array>
#include <chrono>

namespace util {
using SteadyClock = std::chrono::steady_clock;
using DurationMillis = std::chrono::duration<double, std::milli>;
using TimePoint = std::chrono::time_point<SteadyClock, DurationMillis>;

enum MessageType : u8 {
  Info, Warn, Error
};

template <MessageType messageType = Info, typename ...Args>
constexpr void print(const std::string& fmt, Args... args) {
  if constexpr(messageType == Error) {
    fmt::print(fmt::emphasis::bold | fg(fmt::color::red), fmt, args...);
    exit(-1);
  } else if constexpr(messageType == Warn) {
    fmt::print(fg(fmt::color::yellow), fmt, args...);
  } else if constexpr(messageType == Info) {
    fmt::print(fmt, args...);
  }
}

template <typename ...Args>
constexpr void panic(const std::string& fmt, Args... args) {
  print<Error>(fmt, args...);
}

template <typename ...Args>
constexpr void warn(const std::string& fmt, Args... args) {
  print<Warn>(fmt, args...);
}

template <typename ...Args>
constexpr void info(const std::string& fmt, Args... args) {
  print(fmt, args...);
}

template <typename ...Args>
constexpr void logdebug(const std::string& fmt, Args... args) {
#ifndef NDEBUG
  print(fmt, args...);
#endif
}

template <typename T, bool HToBE = false>
[[maybe_unused]] auto GetSwapFunc(T num) -> T {
  static_assert(sizeof(T) == 2 || sizeof(T) == 4 || sizeof(T) == 8, "GetSwapFunc used with invalid size!");
  if constexpr(sizeof(T) == 2) {
    if constexpr(HToBE) {
      return htobe16(num);
    }
    return be16toh(num);
  } else if constexpr(sizeof(T) == 4) {
    if constexpr(HToBE) {
      return htobe32(num);
    }
    return be32toh(num);
  } else if constexpr(sizeof(T) == 8) {
    if constexpr(HToBE) {
      return htobe64(num);
    }
    return be64toh(num);
  }
}

template <typename T>
inline T ReadAccess(u8* data, u32 index) {
  if constexpr(sizeof(T) == 1) {
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
    T result = ((T)hi << 32) | (T)lo;
    return result;
  }
}

template <typename T>
inline void WriteAccess(u8* data, u32 index, T val) {
  if constexpr(sizeof(T) == 1) {
    data[index] = val;
    return;
  } else if constexpr (sizeof(T) == 2 || sizeof(T) == 4){
    memcpy(&data[index], &val, sizeof(T));
  } else {
    static_assert(sizeof(T) == 8);
    u32 hi = val >> 32;
    u32 lo = val;
    memcpy(&data[index + 0], &hi, sizeof(u32));
    memcpy(&data[index + 4], &lo, sizeof(u32));
  }
}

inline void SwapBuffer32(size_t size, u8* data) {
  for(int i = 0; i < size; i += 4) {
    u32 original = *(u32*)&data[i];
    *(u32*)&data[i] = bswap_32(original);
  }
}

inline void SwapBuffer16(size_t size, u8* data) {
  for(int i = 0; i < size; i += 2) {
    u16 original = *(u16*)&data[i];
    *(u16*)&data[i] = bswap_16(original);
  }
}

enum RomTypes {
  Z64 = 0x80371240,
  N64 = 0x40123780,
  V64 = 0x37804012
};

inline void SwapN64Rom(size_t size, u8* data) {
  RomTypes endianness;
  memcpy(&endianness, data, 4);
  endianness = static_cast<RomTypes>(be32toh(endianness));
  switch(endianness) {
    case V64: {
      u8* tmp = (u8*)calloc(size, 1);
      for(int i = 0; i < size; i++) {
        data[i] = tmp[i ^ 2];
      }
      free(tmp);
    } break;
    case N64: break;
    case Z64:
      SwapBuffer32(size, data); break;
    default:
      panic("Unrecognized rom format! Make sure this is a valid Nintendo 64 ROM dump!\n");
  }
}

template <size_t size, typename T = u8>
struct CircularBuffer {
  CircularBuffer() : head(0) {
    memset(raw.data(), 0, size * sizeof(T));
  }
  // make it scroll
  void PushValue(T val) {
    raw[head] = val;
    head++;
    head &= mask;
  }

  T PopValue() {
    head--;
    head &= mask;
    return raw[head];
  }

  T& operator[](const int index) {
    return raw[index];
  }
  auto begin() { return raw.begin(); }
  auto end() { return raw.end(); }
  size_t GetHead() { return head; }
private:
  std::array<T, size> raw;
  size_t head;
  static constexpr size_t mask = size - 1;
};

inline size_t NextPow2(size_t num) {
  // Taken from "Bit Twiddling Hacks" by Sean Anderson:
  // https://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
  --num;
  num |= num >> 1;
  num |= num >> 2;
  num |= num >> 4;
  num |= num >> 8;
  num |= num >> 16;
  return num + 1;
}

inline auto ReadFileBinary(const std::string& path, u32** buf) {
  std::ifstream file(path, std::ios::binary);
  file.unsetf(std::ios::skipws);
  if(!file.is_open()) {
    panic("Could not load file!\n");
  }

  file.seekg(0, std::ios::end);
  size_t size = file.tellg();
  file.seekg(0, std::ios::beg);

  *buf = (u32*)calloc(size, 1);
  file.read(reinterpret_cast<char*>(*buf), size);
  file.close();
  return size;
}
}