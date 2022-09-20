#pragma once
#include <common.hpp>
#include <n64/memory_regions.hpp>
#include <n64/core/MMIO.hpp>
#include <vector>
#include <util.hpp>

namespace n64 {
struct Registers;
struct Mem {
  ~Mem() = default;
  Mem();
  void Reset();
  u32 LoadROM(const std::string&);
  [[nodiscard]] auto GetRDRAM() -> u8* {
    return mmio.rdp.dram.data();
  }

  template <bool tlb = true>
  u8 Read8(Registers&, u32, s64);
  template <bool tlb = true>
  u16 Read16(Registers&, u32, s64);
  template <bool tlb = true>
  u32 Read32(Registers&, u32, s64);
  template <bool tlb = true>
  u64 Read64(Registers&, u32, s64);
  template <bool tlb = true>
  void Write8(Registers&, u32, u32, s64);
  template <bool tlb = true>
  void Write16(Registers&, u32, u32, s64);
  template <bool tlb = true>
  void Write32(Registers&, u32, u32, s64);
  template <bool tlb = true>
  void Write64(Registers&, u32, u64, s64);

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
  u8 isviewer[ISVIEWER_SIZE]{};
  size_t romMask;
};

template <bool tlb = true>
bool MapVAddr(Registers& regs, TLBAccessType accessType, u32 vaddr, u32& paddr);
}
