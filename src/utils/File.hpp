#pragma once
#include <fstream>
#include <log.hpp>

namespace Util {
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
}