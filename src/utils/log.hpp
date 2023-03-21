#pragma once
#include <common.hpp>
#include <fmt/format.h>
#include <fmt/color.h>

namespace Util {
enum MessageType : u8 {
  Trace, Debug, Info, Warn, Panic
};

#ifdef NDEBUG
static constexpr MessageType globalLogLevel = Panic;
#else
static constexpr MessageType globalLogLevel = Trace;
#endif

template <MessageType logLevel, typename ...Args>
constexpr void print(const std::string& fmt, Args... args) {
  if(logLevel >= globalLogLevel) {
    fmt::print(fmt + "\n", args...);
  }
}

template <typename ...Args>
constexpr void panic(const std::string& fmt, Args... args) {
  print<Panic>(fmt, args...);
  exit(-1);
}

template <typename ...Args>
constexpr void warn(const std::string& fmt, Args... args) {
  print<Warn>(fmt, args...);
}

template <typename ...Args>
constexpr void info(const std::string& fmt, Args... args) {
  print<Info>(fmt, args...);
}

template <typename ...Args>
constexpr void trace(const std::string& fmt, Args... args) {
  print<Trace>(fmt, args...);
}

template <typename ...Args>
constexpr void debug(const std::string& fmt, Args... args) {
  print<Debug>(fmt, args...);
}
}