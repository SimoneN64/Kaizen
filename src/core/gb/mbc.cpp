#include <mbc.hpp>
#include <utility>
#include <util.hpp>

namespace natsukashii::core {
NoMBC::NoMBC(std::vector<u8> rom) : data(std::move(rom)) {}

u8 NoMBC::Read8(u16 addr) {
  return data[addr];
}

void NoMBC::Write8(u16 addr, u8 val) {
  util::panic("Writing to a NoMBC cartridge is not allowed!");
}

u16 NoMBC::Read16(u16 addr) {
  return ((u16)Read8(addr) << 8) | Read8(addr + 1);
}

void NoMBC::Write16(u16 addr, u16 val) {
  Write8(addr, val >> 8);
  Write8(addr + 1, val & 0xff);
}
}