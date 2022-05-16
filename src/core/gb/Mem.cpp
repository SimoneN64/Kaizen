#include <Mem.hpp>
#include <util.hpp>
#include <memory_regions.hpp>
#include <fstream>

namespace natsukashii::core {
template <class T>
T ReadCart(const std::unique_ptr<Cartridge>& cart, u16 addr) {
  if constexpr(sizeof(T) == 1) return cart->Read8(addr);
  else if constexpr(sizeof(T) == 2) return cart->Read16(addr);
  else if constexpr(sizeof(T) == 4) return cart->Read32(addr);
}

template <class T>
void WriteCart(const std::unique_ptr<Cartridge>& cart, u16 addr, T val) {
  if constexpr(sizeof(T) == 1) cart->Write8(addr, val);
  else if constexpr(sizeof(T) == 2) cart->Write16(addr, val);
  else if constexpr(sizeof(T) == 4) cart->Write32(addr, val);
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

  cart = std::make_unique<NoMBC>(rom);
}

template <typename T>
T Mem::Read(u16 addr) {
  switch(addr) {
    case ROM_RNG00: return io.BootROMMapped() ? bootrom[addr] : ReadCart<T>(cart, addr);
    case ROM_RNGNN: return ReadCart<T>(cart, addr);
    default: util::panic("[READ] Unimplemented addr:  {:04X}", addr);
  }

  return 0;
}

template u8 Mem::Read<u8>(u16);
template u16 Mem::Read<u16>(u16);
template u32 Mem::Read<u32>(u16);

template <typename T>
void Mem::Write(u16 addr, T val) {
  switch(addr) {
    case ROM_RNG00: case ROM_RNGNN: WriteCart<T>(cart, addr, val);
    default: util::panic("[WRITE] Unimplemented addr:  {:04X}", addr);
  }
}

template void Mem::Write<u8>(u16, u8);
template void Mem::Write<u16>(u16, u16);
template void Mem::Write<u32>(u16, u32);

template <typename T>
T Mem::Consume(u16& pc) {
  T result = Read<T>(pc);
  pc += sizeof(T);
  return result;
}

template u8 Mem::Consume<u8>(u16&);
template u16 Mem::Consume<u16>(u16&);
template u32 Mem::Consume<u32>(u16&);
}
