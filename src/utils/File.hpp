#pragma once
#include <fstream>
#include <log.hpp>

namespace Util {
FORCE_INLINE std::vector<u8> ReadFileBinary(const std::string &path) {
  std::ifstream file(path, std::ios::binary);
  return {std::istreambuf_iterator{file}, {}};
}

FORCE_INLINE void WriteFileBinary(const std::vector<u8> &data, const std::string &path) {
  std::ofstream file(path, std::ios::binary);
  std::copy(data.begin(), data.end(), std::ostreambuf_iterator{file});
}

template <size_t Size>
FORCE_INLINE void WriteFileBinary(const std::array<u8, Size> &data, const std::string &path) {
  std::ofstream file(path, std::ios::binary);
  std::copy(data.begin(), data.end(), std::ostreambuf_iterator{file});
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
} // namespace Util
