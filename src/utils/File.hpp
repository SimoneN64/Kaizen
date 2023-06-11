#pragma once
#include <fstream>
#include <log.hpp>

namespace Util {
FORCE_INLINE std::vector<u8> ReadFileBinary(const std::string& path) {
  std::ifstream file(path, std::ios::binary);
  return {std::istreambuf_iterator{file}, {}};
}

FORCE_INLINE size_t NextPow2(size_t num) {
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
}