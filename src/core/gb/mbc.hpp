#pragma once
#include <common.hpp>
#include <memory_regions.hpp>
#include <vector>

namespace natsukashii::core {
struct Cartridge {
  virtual u8 Read8(u16 addr);
  virtual u16 Read16(u16 addr);
  virtual void Write8(u16 addr, u8 val);
  virtual void Write16(u16 addr, u16 val);
};

struct NoMBC : public Cartridge {
  explicit NoMBC(std::vector<u8> rom);
  u8 Read8(u16 addr) override;
  void Write8(u16 addr, u8 val) override;
  u16 Read16(u16 addr) override;
  void Write16(u16 addr, u16 val) override;
private:
  std::vector<u8> data{};
};
}