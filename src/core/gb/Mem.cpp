#include <Mem.hpp>
#include <util.hpp>
#include <memory_regions.hpp>
#include <fstream>
#include <toml.hpp>

namespace natsukashii::gb::core {
Mem::Mem() {
  auto data = toml::parse("config.toml");
  auto gb = toml::find(data, "gb");
  auto bootromPath = toml::find<std::string>(gb, "bootrom");

  LoadBootROM(bootromPath);
}

void Mem::LoadBootROM(const std::string &filename) {
  std::ifstream file(filename, std::ios::binary);
  file.unsetf(std::ios::skipws);

  if(!file.is_open()) {
    util::panic("Unable to open {}!", filename);
  }

  file.read(reinterpret_cast<char*>(bootrom), 256);
  file.close();
}

void Mem::LoadROM(const std::string& filename) {
  std::ifstream file(filename, std::ios::binary);
  file.unsetf(std::ios::skipws);

  if(!file.is_open()) {
    util::panic("Unable to open {}!", filename);
  }

  file.seekg(std::ios::end);
  auto size = file.tellg();
  file.seekg(std::ios::beg);

  std::vector<u8> rom;
  rom.reserve(size);
  rom.insert(rom.begin(),
             std::istream_iterator<u8>(file),
             std::istream_iterator<u8>());

  file.close();
  switch(rom[0x147]) {
    case 0:
      cart = std::make_unique<NoMBC>(rom);
      break;
    default:
      util::panic("Unimplemented cartridge type {:02X}!", rom[0x147]);
  }
}

u8 Mem::Read8(u16 addr) {
  switch(addr) {
    case ROM_RNG00: return io.BootROMMapped() ? bootrom[addr] : cart->Read(addr);
    case ROM_RNGNN: return cart->Read(addr);
    default: util::panic("[READ] Unimplemented addr:  {:04X}", addr);
  }

  return 0;
}

void Mem::Write8(u16 addr, u8 val) {
  switch(addr) {
    case ROM_RNG00: case ROM_RNGNN: cart->Write(addr, val);
    default: util::panic("[WRITE] Unimplemented addr:  {:04X}", addr);
  }
}

u8 Mem::Consume8(u16& pc) {
  u8 result = Read8(pc);
  pc += 1;
  return result;
}

u16 Mem::Read16(u16 addr) {
  return ((u16)Read8(addr) << 8) | Read8(addr + 1);
}

void Mem::Write16(u16 addr, u16 val) {
  Write8(addr, val >> 8);
  Write8(addr + 1, val & 0xff);
}

u16 Mem::Consume16(u16& pc) {
  u8 hi = Consume8(pc);
  return ((u16)hi << 8) | Consume8(pc);
}
}
