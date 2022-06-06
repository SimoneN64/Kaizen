#pragma once
#include <common.hpp>
#include <memory>
#include <mio/mmap.hpp>
#include <vector>
#include "memory_regions.hpp"
#include "mbc.hpp"

namespace natsukashii::gb::core {
struct IO {
  [[nodiscard]] bool BootROMMapped() const { return ff50 != 1; }
private:
  u8 ff50 = 0;
};

struct Mem {
  Mem();
  void LoadROM(const std::string& filename);
  u8 Read8(u16 addr);
  void Write8(u16 addr, u8 val);
  u8 Consume8(u16& pc);
  u16 Read16(u16 addr);
  void Write16(u16 addr, u16 val);
  u16 Consume16(u16& pc);
private:
  void LoadBootROM(const std::string& filename);
  std::unique_ptr<Cartridge> cart;
  IO io{};
  u8 bootrom[BOOTROM_SIZE]{};
};
}
