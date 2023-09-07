#pragma once
#include <common.hpp>
#include <fmt/format.h>
#include <fmt/color.h>

namespace Util {
enum LogLevel : u8 {
  Trace, Debug, Info, Warn, Error
};

#ifndef NDEBUG
static constexpr auto globalLogLevel = Debug;
#else
static constexpr auto globalLogLevel = Info;
#endif

template <LogLevel messageType = Info, typename ...Args>
constexpr void print(const std::string& fmt, Args... args) {
  if constexpr(messageType >= globalLogLevel) {
#ifndef _WIN32
    if constexpr(messageType == Error) {
      fmt::print(fmt::emphasis::bold | fg(fmt::color::red), fmt, args...);
    } else if constexpr(messageType == Warn) {
      fmt::print(fg(fmt::color::yellow), fmt, args...);
    } else if constexpr(messageType == Info) {
      fmt::print(fmt, args...);
    } else if constexpr(messageType == Debug) {
#ifndef NDEBUG
      fmt::print(fmt, args...);
#endif
    }
#else
    if constexpr(messageType == Debug) {
#ifndef NDEBUG
      fmt::print(fmt, args...);
#endif
    } else {
      fmt::print(fmt, args...);
    }
#endif
  }
}

template <typename ...Args>
constexpr void panic(const std::string& fmt, Args... args) {
  print<Error>(fmt + "\n", args...);
  exit(-1);
}

template <typename ...Args>
constexpr void error(const std::string& fmt, Args... args) {
  print<Error>(fmt + "\n", args...);
}

template <typename ...Args>
constexpr void warn(const std::string& fmt, Args... args) {
  print<Warn>(fmt + "\n", args...);
}

template <typename ...Args>
constexpr void info(const std::string& fmt, Args... args) {
  print(fmt + "\n", args...);
}

template <typename ...Args>
constexpr void debug(const std::string& fmt, Args... args) {
  print<Debug>(fmt + "\n", args...);
}

template <typename ...Args>
constexpr void trace(const std::string& fmt, Args... args) {
  print<Trace>(fmt + "\n", args...);
}
}