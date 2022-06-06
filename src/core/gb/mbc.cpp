#include <mbc.hpp>
#include <utility>
#include <util.hpp>

namespace natsukashii::gb::core {
NoMBC::NoMBC(std::vector<u8> rom) : data(std::move(rom)) {}

u8 NoMBC::Read(u16 addr) {
  return data[addr];
}

void NoMBC::Write(u16 addr, u8 val) {
  util::panic("Writing to a NoMBC cartridge is not allowed! (Addr: {:04X})", addr);
}
}
