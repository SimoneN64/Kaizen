#include <Mem.hpp>
#include <fstream>
#include <util.hpp>

namespace natsukashii::n64::core {
Mem::Mem() {
  rdram.resize(RDRAM_SIZE);
  sram.resize(SRAM_SIZE);
}

void Mem::LoadROM(const std::string& filename) {
  std::ifstream file(filename, std::ios::binary);
  file.unsetf(std::ios::skipws);

  if(!file.is_open()) {
    util::panic("Unable to open {}!", filename);
  }

  file.seekg(std::ios::end);
  auto size = file.tellg();
  auto size_adjusted = util::NextPow2(size);
  romMask = size_adjusted - 1;
  file.seekg(std::ios::beg);

  std::vector<u8> rom;
  rom.reserve(size_adjusted);
  file.read(reinterpret_cast<char*>(rom.data()), size);

  file.close();
  util::SwapN64Rom(size, rom.data());
  memcpy(dmem, rom.data(), 0x1000);
}
}