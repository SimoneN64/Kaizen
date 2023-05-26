#pragma once
#include <common.hpp>
#include <backend/MemoryRegions.hpp>
#include <backend/core/MMIO.hpp>
#include <vector>
#include <backend/RomHelpers.hpp>
#include <log.hpp>
#include <Registers.hpp>
#include <algorithm>
#include <GameDB.hpp>

namespace n64 {
struct CartInfo {
  bool isPAL;
  u32 cicType;
  u32 crc;
};

struct JIT;
struct CachedInterpreter;

struct ROMHeader {
  u8 initialValues[4];
  u32 clockRate;
  u32 programCounter;
  u32 release;
  u32 crc1;
  u32 crc2;
  u64 unknown;
  char imageName[20];
  u32 unknown2;
  u32 manufacturerId;
  u16 cartridgeId;
  char countryCode[2];
  u8 bootCode[4032];
};

struct ROM {
  u8* cart;
  size_t size;
  size_t mask;
  ROMHeader header;
  CICType cicType;
  char gameNameCart[20];
  std::string gameNameDB;
  char code[4];
  bool pal;
};

struct Mem {
  ~Mem() {
    free(sram);
  }
  Mem();
  void Reset();
  void LoadROM(const std::string&);
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
  void Write8(Registers&, CachedInterpreter&, u32, u32);
  void Write16(Registers&, CachedInterpreter&, u32, u32);
  void Write32(Registers&, CachedInterpreter&, u32, u32);
  void Write64(Registers&, CachedInterpreter&, u32, u64);
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
  ROM rom;
  SaveType saveType;
private:
  friend struct SI;
  friend struct PI;
  friend struct AI;
  friend struct RSP;
  friend struct Core;
  u8* sram;
  u8 isviewer[ISVIEWER_SIZE]{};

  bool IsROMPAL() {
    static const char pal_codes[] = {'D', 'F', 'I', 'P', 'S', 'U', 'X', 'Y'};
    return std::any_of(std::begin(pal_codes), std::end(pal_codes), [this](char a) {
      return rom.cart[0x3e] == a;
    });
  }
};
}
