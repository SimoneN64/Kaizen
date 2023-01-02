#pragma once
#include <common.hpp>
#include <backend/MemoryRegions.hpp>
#include <backend/core/MMIO.hpp>
#include <vector>
#include <backend/RomHelpers.hpp>
#include <log.hpp>

namespace n64 {
struct Registers;

struct CartInfo {
  bool isPAL;
  u32 cicType;
  u32 crc;
};

struct Mem {
  ~Mem() = default;
  Mem();
  void Reset();
  CartInfo LoadROM(const std::string&);
  [[nodiscard]] auto GetRDRAM() -> u8* {
    return mmio.rdp.rdram.data();
  }

  template <bool tlb = true>
  u8 Read8(Registers&, u64, s64);
  template <bool tlb = true>
  u16 Read16(Registers&, u64, s64);
  template <bool tlb = true>
  u32 Read32(Registers&, u64, s64);
  template <bool tlb = true>
  u64 Read64(Registers&, u64, s64);
  template <bool tlb = true>
  void Write8(Registers&, u64, u32, s64);
  template <bool tlb = true>
  void Write16(Registers&, u64, u32, s64);
  template <bool tlb = true>
  void Write32(Registers&, u64, u32, s64);
  template <bool tlb = true>
  void Write64(Registers&, u64, u64, s64);

  MMIO mmio;
  u8 pifRam[PIF_RAM_SIZE]{};

  inline void DumpRDRAM() const {
    FILE *fp = fopen("rdram.dump", "wb");
    u8 *temp = (u8*)calloc(RDRAM_SIZE, 1);
    memcpy(temp, mmio.rdp.rdram.data(), RDRAM_SIZE);
    util::SwapBuffer32(RDRAM_SIZE, temp);
    fwrite(temp, 1, RDRAM_SIZE, fp);
    free(temp);
    fclose(fp);
  }

  inline void DumpIMEM() const {
    FILE *fp = fopen("imem.bin", "wb");
    u8 *temp = (u8*)calloc(IMEM_SIZE, 1);
    memcpy(temp, mmio.rsp.imem, IMEM_SIZE);
    util::SwapBuffer32(IMEM_SIZE, temp);
    fwrite(temp, 1, IMEM_SIZE, fp);
    free(temp);
    fclose(fp);
  }

  inline void DumpDMEM() const {
    FILE *fp = fopen("dmem.dump", "wb");
    u8 *temp = (u8*)calloc(DMEM_SIZE, 1);
    memcpy(temp, mmio.rsp.dmem, DMEM_SIZE);
    util::SwapBuffer32(DMEM_SIZE, temp);
    fwrite(temp, 1, DMEM_SIZE, fp);
    free(temp);
    fclose(fp);
  }
  std::vector<uintptr_t> writePages, readPages;
private:
  friend struct SI;
  friend struct PI;
  friend struct AI;
  friend struct Cpu;
  friend struct RSP;
  friend struct Core;
  std::vector<u8> cart, sram;
  u8 pifBootrom[PIF_BOOTROM_SIZE]{};
  u8 isviewer[ISVIEWER_SIZE]{};
  size_t romMask;

  void SetCICType(u32& cicType, u32 checksum) {
    switch(checksum) {
      case 0xEC8B1325: // 7102
        cicType = CIC_NUS_7102;
        break;
      case 0x1DEB51A9: // 6101
        cicType = CIC_NUS_6101;
        break;
      case 0xC08E5BD6:
        cicType = CIC_NUS_6102_7101;
        break;
      case 0x03B8376A:
        cicType = CIC_NUS_6103_7103;
        break;
      case 0xCF7F41DC:
        cicType = CIC_NUS_6105_7105;
        break;
      case 0xD1059C6A:
        cicType = CIC_NUS_6106_7106;
        break;
      default:
        util::warn("Could not determine CIC TYPE! Checksum: {:08X} is unknown!\n", checksum);
        cicType = UNKNOWN_CIC_TYPE;
        break;
    }
  }

  bool IsROMPAL() {
    static const char pal_codes[] = {'D', 'F', 'I', 'P', 'S', 'U', 'X', 'Y'};
    for (int i = 0; i < 8; i++) {
      if (cart[0x3e] == pal_codes[i]) {
        return true;
      }
    }
    return false;
  }
};

template <bool tlb = true>
bool MapVAddr(Registers& regs, TLBAccessType accessType, u64 vaddr, u32& paddr);
}
