#pragma once
#include <common.hpp>
#include <memory>
#include <mio/mmap.hpp>
#include <vector>
#include <memory_regions.hpp>
#include <mbc.hpp>

namespace natsukashii::core {
struct IO {
  [[nodiscard]] bool BootROMMapped() const { return ff50 != 1; }
private:
  u8 ff50 = 0;
};

struct Mem {
  Mem() = default;
  void LoadROM(const std::string& filename);
  template <typename T>
  T Read(u16 addr);
  template <typename T>
  void Write(u16 addr, T val);
  template <typename T>
  T Consume(u16& pc);
private:
  std::unique_ptr<Cartridge> cart;
  IO io{};
  u8 bootrom[BOOTROM_SIZE]{};
};
}
