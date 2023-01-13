#pragma once
#include <common.hpp>
#include <fmt/format.h>
#include <fmt/color.h>

namespace Util {
enum MessageType : u8 {
  Info, Debug, Warn, Error
};

template <MessageType messageType = Info, typename ...Args>
constexpr void print(const std::string& fmt, Args... args) {
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

template <typename ...Args>
constexpr void panic(const std::string& fmt, Args... args) {
  print<Error>(fmt, args...);
  exit(-1);
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
constexpr void debug(const std::string& fmt, Args... args) {
  print<Debug>(fmt, args...);
}
}