#pragma once
#include <common.hpp>
#include <type_traits>
#include <cassert>
#include <portable_endian_bswap.h>
#include <fstream>

namespace util {
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

template <typename T, bool HToBE = false>
auto GetSwapFunc(T num) -> T {
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
  } else {
    static_assert(sizeof(T) == 2 || sizeof(T) == 4 || sizeof(T) == 8);
    T result = 0;
    memcpy(&result, &data[index], sizeof(T));
    return GetSwapFunc<T>(result);
  }
}

template <typename T>
inline void WriteAccess(u8* data, u32 index, T val) {
  if constexpr(sizeof(T) == 1) {
    data[index] = val;
    return;
  } else {
    static_assert(sizeof(T) == 2 || sizeof(T) == 4 || sizeof(T) == 8);
    T temp = GetSwapFunc<T, true>(val);
    memcpy(&data[index], &temp, sizeof(T));
  }
}

#define Z64 0x80371240
#define N64 0x40123780
#define V64 0x37804012

inline void SwapN64Rom(size_t size, u8* data) {
  u32 endianness;
  memcpy(&endianness, data, 4);
  endianness = be32toh(endianness);
  switch(endianness) {
    case V64:
      for(int i = 0; i < size; i += 2) {
        u16 original = *(u16*)&data[i];
        *(u16*)&data[i] = bswap_16(original);
      }
      break;
    case N64:
      for(int i = 0; i < size; i += 4) {
        u32 original = *(u32*)&data[i];
        *(u32*)&data[i] = bswap_32(original);
      }
      break;
    case Z64: break;
    default:
      panic("Unrecognized rom format! Make sure this is a valid Nintendo 64 ROM dump!\n");
  }
}

template <size_t size, typename T = u8>
struct CircularBuffer {
  CircularBuffer() : head(0) {
    memset(raw, 0, size * sizeof(T));
  }

  void PushValue(T val) {
    raw[head & mask] = val;
    head &= mask;
    head++;
  }

  T PopValue() {
    head--;
    head &= mask;
    return raw[head & mask];
  }
  size_t GetHead() { return head; }
private:
  T raw[size];
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

inline void ReadFileBinary(const std::string& path, void* buf) {
  std::ifstream file("external/vert.spv", std::ios::binary);
  file.unsetf(std::ios::skipws);
  if(!file.is_open()) {
    util::panic("Could not load file!\n");
  }
  file.seekg(std::ios::end);
  auto size = file.tellg();
  file.seekg(std::ios::beg);
  if(buf)
    free(buf);

  buf = calloc(size, 1);
  file.read((char*)buf, size);
  file.close();
}
}