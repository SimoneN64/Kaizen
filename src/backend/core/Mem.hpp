#pragma once
#include <common.hpp>
#include <backend/MemoryRegions.hpp>
#include <backend/core/MMIO.hpp>
#include <vector>
#include <log.hpp>
#include <Registers.hpp>
#include <algorithm>
#include <GameDB.hpp>

namespace n64 {
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

enum class FlashState : u8 {
  Idle, Erase, Write, Read, Status
};

struct Flash {
  explicit Flash(mio::mmap_sink&);
  ~Flash() = default;
  void Reset();
  void Load(SaveType, const std::string&);
  FlashState state{};
  u64 status{};
  size_t eraseOffs{};
  size_t writeOffs{};
  u8 writeBuf[128]{};
  std::string flashPath{};
  mio::mmap_sink& saveData;

  enum FlashCommands : u8 {
    FLASH_COMMAND_EXECUTE = 0xD2,
    FLASH_COMMAND_STATUS = 0xE1,
    FLASH_COMMAND_SET_ERASE_OFFSET = 0x4B,
    FLASH_COMMAND_ERASE = 0x78,
    FLASH_COMMAND_SET_WRITE_OFFSET = 0xA5,
    FLASH_COMMAND_WRITE = 0xB4,
    FLASH_COMMAND_READ = 0xF0,
  };

  void CommandExecute();
  void CommandStatus();
  void CommandSetEraseOffs(u32);
  void CommandErase();
  void CommandSetWriteOffs(u32);
  void CommandWrite();
  void CommandRead();
  std::vector<u8> Serialize();
  void Deserialize(const std::vector<u8>& data);
  template <typename T>
  void Write(u32 index, T val);
  template <typename T>
  T Read(u32 index) const;
};

struct Mem {
  ~Mem() = default;
  Mem(Registers&);
  void Reset();
  void LoadSRAM(SaveType, fs::path);
  static std::vector<u8> OpenROM(const std::string&, size_t&);
  static std::vector<u8> OpenArchive(const std::string&, size_t&);
  void LoadROM(bool, const std::string&);
  [[nodiscard]] auto GetRDRAM() const -> u8* {
    return mmio.rdp.rdram;
  }

  std::vector<u8> Serialize();
  void Deserialize(const std::vector<u8>&);

  template <typename T>
  T Read(Registers&, u32);
  template <typename T>
  void Write(Registers&, u32, u32);
  void Write(Registers&, u32, u64);

  template <typename T>
  T BackupRead(u32);
  template <typename T>
  void BackupWrite(u32, T);

  MMIO mmio;

  FORCE_INLINE void DumpRDRAM() const {
    FILE *fp = fopen("rdram.dump", "wb");
    u8 *temp = (u8*)calloc(RDRAM_SIZE, 1);
    memcpy(temp, mmio.rdp.rdram, RDRAM_SIZE);
    Util::SwapBuffer32(RDRAM_SIZE, temp);
    fwrite(temp, 1, RDRAM_SIZE, fp);
    free(temp);
    fclose(fp);
  }

  FORCE_INLINE void DumpIMEM() const {
    FILE *fp = fopen("imem.bin", "wb");
    u8 *temp = (u8*)calloc(IMEM_SIZE, 1);
    memcpy(temp, mmio.rsp.imem, IMEM_SIZE);
    Util::SwapBuffer32(IMEM_SIZE, temp);
    fwrite(temp, 1, IMEM_SIZE, fp);
    free(temp);
    fclose(fp);
  }

  FORCE_INLINE void DumpDMEM() const {
    FILE *fp = fopen("dmem.dump", "wb");
    u8 *temp = (u8*)calloc(DMEM_SIZE, 1);
    memcpy(temp, mmio.rsp.dmem, DMEM_SIZE);
    Util::SwapBuffer32(DMEM_SIZE, temp);
    fwrite(temp, 1, DMEM_SIZE, fp);
    free(temp);
    fclose(fp);
  }
  uintptr_t writePages[PAGE_COUNT]{}, readPages[PAGE_COUNT]{};
  ROM rom;
  SaveType saveType = SAVE_NONE;
  Flash flash;
private:
  friend struct SI;
  friend struct PI;
  friend struct AI;
  friend struct RSP;
  friend struct Core;
  u8 isviewer[ISVIEWER_SIZE]{};
  std::string sramPath{};
  mio::mmap_sink saveData{};
  int mmioSize{}, flashSize{};

  FORCE_INLINE bool IsROMPAL() {
    static const char pal_codes[] = {'D', 'F', 'I', 'P', 'S', 'U', 'X', 'Y'};
    return std::any_of(std::begin(pal_codes), std::end(pal_codes), [this](char a) {
      return rom.cart[0x3d] == a;
    });
  }
};
}
