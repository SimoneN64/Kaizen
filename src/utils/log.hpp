#pragma once
#include <common.hpp>
#include <fmt/format.h>
#include <fmt/color.h>

namespace Util {
enum MessageType : u8 {
  Trace, Debug, Info, Warn, Panic
};

#ifdef NDEBUG
#define GLOBAL_LOG_LEVEL Panic
#else
#define GLOBAL_LOG_LEVEL Debug
#endif

template <MessageType logLevel, typename ...Args>
constexpr void print(const std::string& fmt, Args... args) {
  if constexpr (logLevel >= GLOBAL_LOG_LEVEL) {
    fmt::print(fmt, args...);
  }
}

template <typename ...Args>
constexpr void panic(const std::string& fmt, Args... args) {
  print<Panic>(fmt + "\n", args...);
  exit(-1);
}

template <typename ...Args>
constexpr void warn(const std::string& fmt, Args... args) {
  print<Warn>(fmt + "\n", args...);
}

template <typename ...Args>
constexpr void info(const std::string& fmt, Args... args) {
  print<Info>(fmt + "\n", args...);
}

template <typename ...Args>
constexpr void trace(const std::string& fmt, Args... args) {
  print<Trace>(fmt + "\n", args...);
}

template <typename ...Args>
constexpr void debug(const std::string& fmt, Args... args) {
  print<Debug>(fmt + "\n", args...);
}
}