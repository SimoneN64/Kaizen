#pragma once
#include <common.hpp>
#include <backend/MemoryRegions.hpp>
#include <backend/core/MMIO.hpp>
#include <vector>
#include <backend/RomHelpers.hpp>
#include <log.hpp>
#include <Registers.hpp>
#include <algorithm>

namespace n64 {
struct CartInfo {
  bool isPAL;
  u32 cicType;
  u32 crc;
};

struct JIT;

struct Mem {
  ~Mem() {
    free(sram);
  }
  Mem();
  void Reset();
  CartInfo LoadROM(const std::string&);
  [[nodiscard]] auto GetRDRAM() const -> u8* {
    return mmio.rdp.rdram;
  }

  u8 Read8(Registers&, u32);
  u16 Read16(Registers&, u32);
  u32 Read32(Registers&, u32);
  u64 Read64(Registers&, u32);
  void Write8(Registers&, JIT&, u32, u32);
  void Write16(Registers&, JIT&, u32, u32);
  void Write32(Registers&, JIT&, u32, u32);
  void Write64(Registers&, JIT&, u32, u64);
  void Write8(Registers&, u32, u32);
  void Write16(Registers&, u32, u32);
  void Write32(Registers&, u32, u32);
  void Write64(Registers&, u32, u64);

  MMIO mmio;

  inline void DumpRDRAM() const {
    FILE *fp = fopen("rdram.dump", "wb");
    u8 *temp = (u8*)calloc(RDRAM_SIZE, 1);
    memcpy(temp, mmio.rdp.rdram, RDRAM_SIZE);
    Util::SwapBuffer32(RDRAM_SIZE, temp);
    fwrite(temp, 1, RDRAM_SIZE, fp);
    free(temp);
    fclose(fp);
  }

  inline void DumpIMEM() const {
    FILE *fp = fopen("imem.bin", "wb");
    u8 *temp = (u8*)calloc(IMEM_SIZE, 1);
    memcpy(temp, mmio.rsp.imem, IMEM_SIZE);
    Util::SwapBuffer32(IMEM_SIZE, temp);
    fwrite(temp, 1, IMEM_SIZE, fp);
    free(temp);
    fclose(fp);
  }

  inline void DumpDMEM() const {
    FILE *fp = fopen("dmem.dump", "wb");
    u8 *temp = (u8*)calloc(DMEM_SIZE, 1);
    memcpy(temp, mmio.rsp.dmem, DMEM_SIZE);
    Util::SwapBuffer32(DMEM_SIZE, temp);
    fwrite(temp, 1, DMEM_SIZE, fp);
    free(temp);
    fclose(fp);
  }
  uintptr_t writePages[PAGE_COUNT], readPages[PAGE_COUNT];
private:
  friend struct SI;
  friend struct PI;
  friend struct AI;
  friend struct RSP;
  friend struct Core;
  u8* sram, *cart;
  u8 isviewer[ISVIEWER_SIZE]{};
  size_t romMask = 0;

  static void SetCICType(u32& cicType, u32 checksum) {
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
        Util::warn("Could not determine CIC TYPE! Checksum: {:08X} is unknown!\n", checksum);
        cicType = UNKNOWN_CIC_TYPE;
        break;
    }
  }

  bool IsROMPAL() {
    static const char pal_codes[] = {'D', 'F', 'I', 'P', 'S', 'U', 'X', 'Y'};
    return std::any_of(std::begin(pal_codes), std::end(pal_codes), [this](char a) {
      return cart[0x3e] == a;
    });
  }
};
}
