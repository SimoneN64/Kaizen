#pragma once
#include <common.hpp>
#include <fmt/color.h>
#include <fmt/core.h>
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
void print(const std::string &fmt, Args... args) {
  if (messageType >= globalLogLevel) {
#ifndef _WIN32
    if (messageType == Error) {
      fmt::print(fmt::emphasis::bold | fg(fmt::color::red), fmt::runtime(fmt), args...);
    } else if (messageType == Warn) {
      fmt::print(fg(fmt::color::yellow), fmt::runtime(fmt), args...);
    } else if (messageType == Info || messageType == Trace || messageType == Always) {
      fmt::print(fmt::runtime(fmt), args...);
    } else if (messageType == Debug) {
#ifndef NDEBUG
      fmt::print(fmt::runtime(fmt), args...);
#endif
    }
#else
    if (messageType == Debug) {
#ifndef NDEBUG
      fmt::print(fmt::runtime(fmt), args...);
#endif
    } else {
      fmt::print(fmt::runtime(fmt), args...);
    }
#endif
  }
}

template <typename... Args>
void panic(const std::string &fmt, Args... args) {
  print<Error>("[FATAL] " + fmt + "\n", args...);
  exit(-1);
}

template <typename... Args>
void error(const std::string &fmt, Args... args) {
  print<Error>("[ERROR] " + fmt + "\n", args...);
}

template <typename... Args>
void warn(const std::string &fmt, Args... args) {
  print<Warn>("[WARN] " + fmt + "\n", args...);
}

template <typename... Args>
void info(const std::string &fmt, Args... args) {
  print("[INFO] " + fmt + "\n", args...);
}

template <typename... Args>
void debug(const std::string &fmt, Args... args) {
  print<Debug>("[DEBUG] " + fmt + "\n", args...);
}

template <typename... Args>
void trace(const std::string &fmt, Args... args) {
  print<Trace>("[TRACE] " + fmt + "\n", args...);
}

template <typename... Args>
void always(const std::string &fmt, Args... args) {
  print<Always>(fmt + "\n", args...);
}

template <typename... Args>
void panic_trace(const std::string &fmt, Args... args) {
#if !defined(NDEBUG) && !defined(_WIN32)
  Dl_info info;
  auto tmp = fmt::format(fmt::runtime(fmt + '\n'), args...);
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
