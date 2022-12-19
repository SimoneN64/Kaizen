#pragma once
#include <common.hpp>
#include <type_traits>
#include <cassert>
#include <portable_endian_bswap.h>
#include <fstream>
#include <vector>
#include <array>
#include <chrono>
#include <functional>
#include <discord_rpc.h>

namespace util {
using SteadyClock = std::chrono::steady_clock;
using DurationMillis = std::chrono::duration<double, std::milli>;
using TimePoint = std::chrono::time_point<SteadyClock, DurationMillis>;

enum MessageType : u8 {
  Info, Warn, Error
};

template <MessageType messageType = Info, typename ...Args>
constexpr void print(const std::string& fmt, Args... args) {
#ifndef _WIN32
  if constexpr(messageType == Error) {
    fmt::print(fmt::emphasis::bold | fg(fmt::color::red), fmt, args...);
    exit(-1);
  } else if constexpr(messageType == Warn) {
    fmt::print(fg(fmt::color::yellow), fmt, args...);
  } else if constexpr(messageType == Info) {
    fmt::print(fmt, args...);
  }
#else
  if constexpr(messageType == Error) {
    fmt::print(fmt, args...);
    exit(-1);
  } else {
    fmt::print(fmt, args...);
  }
#endif
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

enum RomTypes {
  Z64 = 0x80371240,
  N64 = 0x40123780,
  V64 = 0x37804012
};

inline void SwapN64RomJustCRC(size_t size, u8* rom, u32& crc) {
  RomTypes endianness;
  memcpy(&endianness, rom, 4);
  endianness = static_cast<RomTypes>(be32toh(endianness));

  switch(endianness) {
    case RomTypes::V64: {
      SwapBuffer16(size, rom);
      crc = crc32(0, rom, size);
    } break;
    case RomTypes::N64: {
      SwapBuffer32(size, rom);
      crc = crc32(0, rom, size);
    } break;
    case RomTypes::Z64:
      crc = crc32(0, rom, size);
      break;
    default:
      panic("Unrecognized rom format! Make sure this is a valid Nintendo 64 ROM dump!\n");
  }
}

inline void SwapN64Rom(size_t size, u8* rom, u32& crc, u32& cicChecksum) {
  RomTypes endianness;
  memcpy(&endianness, rom, 4);
  endianness = static_cast<RomTypes>(be32toh(endianness));

  switch(endianness) {
    case RomTypes::V64: {
      u8* temp = (u8*)calloc(size, 1);
      memcpy(temp, rom, size);
      SwapBuffer16(size, temp);
      crc = crc32(0, temp, size);
      cicChecksum = crc32(0, &temp[0x40], 0x9c0);
      free(temp);
      SwapBuffer32(size, rom);
      SwapBuffer16(size, rom);
    } break;
    case RomTypes::N64: {
      u8* temp = (u8*)calloc(size, 1);
      memcpy(temp, rom, size);
      SwapBuffer32(size, temp);
      crc = crc32(0, temp, size);
      cicChecksum = crc32(0, &temp[0x40], 0x9c0);
      free(temp);
    } break;
    case RomTypes::Z64:
      crc = crc32(0, rom, size);
      cicChecksum = crc32(0, &rom[0x40], 0x9c0);
      SwapBuffer32(size, rom);
      break;
    default:
      panic("Unrecognized rom format! Make sure this is a valid Nintendo 64 ROM dump!\n");
  }
}

template <size_t size, typename T = u8>
struct CircularBuffer {
  CircularBuffer() : head(0) {
    raw.fill(T());
  }
  // make it scroll
  void PushValue(T val) {
    raw[head] = val;
    head++;
    if(head == size) {
      head = 0;
    }
  }

  T PopValue() {
    head--;
    if(head == 0) {
      head = size - 1;
    }
    return raw[head];
  }

  T& operator[](const int index) {
    return raw[index];
  }
  auto begin() { return raw.begin(); }
  auto end() { return raw.end(); }
  auto fill(const T& u) { raw.fill(u); }
  size_t GetHead() { return head; }
private:
  std::array<T, size> raw;
  size_t head;
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
    panic("Could not load file '{}'!\n", path);
  }

  file.seekg(0, std::ios::end);
  size_t size = file.tellg();
  file.seekg(0, std::ios::beg);

  *buf = (u32*)calloc(size, 1);
  file.read(reinterpret_cast<char*>(*buf), size);
  file.close();
  return size;
}

enum State {
  Idling,
  Playing,
  Paused
};

inline void UpdateRPC(State state, const std::string& game = "") {
  DiscordRichPresence presence{};
  std::string textState, textDetails;

  switch(state) {
    case Idling:
      textDetails = "Idling";
      break;
    case Playing:
      textDetails = "In-game";
      textState = "Playing \"" + game + "\"";
      break;
    case Paused:
      textDetails = "In-game";
      textState = "Playing \"" + game + "\" (Paused)";
      break;
  }

  presence.details = textDetails.c_str();
  presence.state = textState.c_str();
  presence.startTimestamp = time(nullptr);
  presence.largeImageText = "Gadolinium";
  presence.largeImageKey = "logo";
  Discord_UpdatePresence(&presence);
}

inline void ClearRPC() {
  Discord_ClearPresence();
}