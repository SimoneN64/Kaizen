#pragma once
#include <common.hpp>
#include <vector>

namespace natsukashii::gb::core {
struct Cartridge {
  virtual u8 Read(u16 addr) { return 0; }
  virtual void Write(u16 addr, u8 val) {}
};

struct NoMBC : public Cartridge {
  explicit NoMBC(std::vector<u8> rom);
  u8 Read(u16 addr) override;
  void Write(u16 addr, u8 val) override;
private:
  std::vector<u8> data{};
};
}
