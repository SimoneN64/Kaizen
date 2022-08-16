#pragma once
#include <common.hpp>
#include <n64/memory_regions.hpp>
#include <n64/core/MMIO.hpp>
#include <vector>

namespace n64 {
struct Registers;
struct Mem {
  ~Mem() = default;
  Mem();
  void Reset();
  void LoadROM(const std::string&);
  [[nodiscard]] auto GetRDRAM() -> u8* {
    return mmio.rdp.dram.data();
  }
  template <class T, bool tlb = true>
  T Read(Registers&, u32, s64);
  template <class T, bool tlb = true>
  void Write(Registers&, u32, T, s64);
  u8 pifRam[PIF_RAM_SIZE]{};
private:
  friend struct SI;
  friend struct PI;
  friend struct AI;
  friend struct Cpu;
  friend struct RSP;
  friend struct Core;
  MMIO mmio;
  std::vector<u8> cart, sram;
  u8 pifBootrom[PIF_BOOTROM_SIZE]{};
  size_t romMask;
};
}
