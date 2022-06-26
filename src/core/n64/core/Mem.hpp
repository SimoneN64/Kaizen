#pragma once
#include <common.hpp>
#include <n64/memory_regions.hpp>
#include <vector>

namespace natsukashii::n64::core {
struct Mem {
  ~Mem() = default;
  Mem();
  void LoadROM(const std::string&);
private:
  std::vector<u8> cart, rdram, sram;
  u8 dmem[DMEM_SIZE]{}, imem[IMEM_SIZE]{}, pifRam[PIF_RAM_SIZE]{};
  u8 pifBootrom[PIF_BOOTROM_SIZE]{};
  size_t romMask;
};
}
