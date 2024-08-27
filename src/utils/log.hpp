#pragma once
#include <common.hpp>
#include <fmt/color.h>
#include <fmt/format.h>
#include <string>
#if !defined(NDEBUG) && !defined(_WIN32)
#include <dlfcn.h>
#endif

namespace Util {
enum LogLevel : u8 { Trace, Debug, Warn, Info, Error, Always };

#ifndef NDEBUG
static constexpr auto globalLogLevel = Debug;
#else
static constexpr auto globalLogLevel = Info;
#endif

template <LogLevel messageType = Info, typename... Args>
constexpr void print(const std::string &fmt, Args... args) {
  if constexpr (messageType >= globalLogLevel) {
#ifndef _WIN32
    if constexpr (messageType == Error) {
      fmt::print(fmt::emphasis::bold | fg(fmt::color::red), fmt, args...);
    } else if constexpr (messageType == Warn) {
      fmt::print(fg(fmt::color::yellow), fmt, args...);
    } else if constexpr (messageType == Info || messageType == Trace || messageType == Always) {
      fmt::print(fmt, args...);
    } else if constexpr (messageType == Debug) {
#ifndef NDEBUG
      fmt::print(fmt, args...);
#endif
    }
#else
    if constexpr (messageType == Debug) {
#ifndef NDEBUG
      fmt::print(fmt, args...);
#endif
    } else {
      fmt::print(fmt, args...);
    }
#endif
  }
}

template <typename... Args>
constexpr void panic(const std::string &fmt, Args... args) {
  print<Error>("[FATAL] " + fmt + "\n", args...);
  exit(-1);
}

template <typename... Args>
constexpr void error(const std::string &fmt, Args... args) {
  print<Error>("[ERROR] " + fmt + "\n", args...);
}

template <typename... Args>
constexpr void warn(const std::string &fmt, Args... args) {
  print<Warn>("[WARN] " + fmt + "\n", args...);
}

template <typename... Args>
constexpr void info(const std::string &fmt, Args... args) {
  print("[INFO] " + fmt + "\n", args...);
}

template <typename... Args>
constexpr void debug(const std::string &fmt, Args... args) {
  print<Debug>("[DEBUG] " + fmt + "\n", args...);
}

template <typename... Args>
constexpr void trace(const std::string &fmt, Args... args) {
  print<Trace>("[TRACE] " + fmt + "\n", args...);
}

template <typename... Args>
constexpr void always(const std::string &fmt, Args... args) {
  print<Always>(fmt + "\n", args...);
}

template <typename... Args>
constexpr void panic_trace(const std::string &fmt, Args... args) {
#if !defined(NDEBUG) && !defined(_WIN32)
  Dl_info info;
  auto tmp = fmt::format(fmt + '\n', args...);
  tmp += "Called by:\n";
  if (dladdr(__builtin_return_address(0), &info))
    tmp += fmt::format("\t-> {}\n", info.dli_sname);
  if (dladdr(__builtin_return_address(1), &info))
    tmp += fmt::format("\t-> {}\n", info.dli_sname);
  if (dladdr(__builtin_return_address(2), &info))
    tmp += fmt::format("\t-> {}\n", info.dli_sname);
  if (dladdr(__builtin_return_address(3), &info))
    tmp += fmt::format("\t-> {}\n", info.dli_sname);
  if (dladdr(__builtin_return_address(4), &info))
    tmp += fmt::format("\t-> {}\n", info.dli_sname);
  if (dladdr(__builtin_return_address(5), &info))
    tmp += fmt::format("\t-> {}\n", info.dli_sname);
  if (dladdr(__builtin_return_address(6), &info))
    tmp += fmt::format("\t-> {}\n", info.dli_sname);

  print<Error>("[FATAL TRACE] " + tmp);
  exit(-1);
#else
  panic(fmt, args...);
#endif
}
} // namespace Util
