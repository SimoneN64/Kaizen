#pragma once
#include <common.hpp>
#include <memory_regions.hpp>
#include <vector>

namespace natsukashii::n64::core {
struct Mem {
  Mem();
  void LoadROM(const std::string&);
private:
  std::vector<u8> cart, rdram, sram;
  u8 dmem[DMEM_SIZE]{}, imem[IMEM_SIZE]{}, pif_ram[PIF_RAM_SIZE]{};
  u8 pif_bootrom[PIF_BOOTROM_SIZE]{};
  size_t rom_mask;
};
}
