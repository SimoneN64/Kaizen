#pragma once
#include <MemoryRegions.hpp>
#include <SDL_gamecontroller.h>
#include <GameDB.hpp>
#include <filesystem>
#include <mio/mmap.hpp>
#include <vector>

namespace fs = std::filesystem;

namespace n64 {

enum AccessoryType : u8 {
  ACCESSORY_NONE,
  ACCESSORY_MEMPACK,
  ACCESSORY_RUMBLE_PACK
};

struct Controller {
  union {
    u8 byte1{};
    struct {
      bool dp_right:1;
      bool dp_left:1;
      bool dp_down:1;
      bool dp_up:1;
      bool start:1;
      bool z:1;
      bool b:1;
      bool a:1;
    };
  };
  union {
    u8 byte2{};
    struct {
      bool c_right:1;
      bool c_left:1;
      bool c_down:1;
      bool c_up:1;
      bool r:1;
      bool l:1;
      bool zero:1;
      bool joy_reset:1;
    };
  };
  s8 joy_x{};
  s8 joy_y{};

  Controller() = default;
  Controller& operator=(u32 v) {
    joy_y = v & 0xff;
    joy_x = v >> 8;
    byte2 = v >> 16;
    byte1 = v >> 24;

    return *this;
  }
};

static_assert(sizeof(Controller) == 4);

enum JoybusType : u8 {
  JOYBUS_NONE,
  JOYBUS_CONTROLLER,
  JOYBUS_DANCEPAD,
  JOYBUS_VRU,
  JOYBUS_MOUSE,
  JOYBUS_RANDNET_KEYBOARD,
  JOYBUS_DENSHA_DE_GO,
  JOYBUS_4KB_EEPROM,
  JOYBUS_16KB_EEPROM
};

struct JoybusDevice {
  JoybusType type{};
  AccessoryType accessoryType{};
  Controller controller{};

  JoybusDevice() = default;
};

struct Mem;
struct Registers;

// https://github.com/ares-emulator/ares/blob/master/ares/n64/cic/cic.cpp
// https://github.com/ares-emulator/ares/blob/master/LICENSE
constexpr u32 cicSeeds[] = {
  0x0,
  0x00043F3F, // CIC_NUS_6101
  0x00043F3F, // CIC_NUS_7102
  0x00043F3F, // CIC_NUS_6102_7101
  0x00047878, // CIC_NUS_6103_7103
  0x00049191, // CIC_NUS_6105_7105
  0x00048585, // CIC_NUS_6106_7106
};

enum CICType {
  UNKNOWN_CIC_TYPE,
  CIC_NUS_6101,
  CIC_NUS_7102,
  CIC_NUS_6102_7101,
  CIC_NUS_6103_7103,
  CIC_NUS_6105_7105,
  CIC_NUS_6106_7106
};

struct PIF {
  PIF() = default;
  ~PIF() = default;
  void Reset();
  void MaybeLoadMempak();
  void LoadEeprom(SaveType, const std::string&);
  void ProcessCommands(Mem&);
  void InitDevices(SaveType);
  void CICChallenge();
  static void ExecutePIF(Mem& mem, Registers& regs);
  static void DoPIFHLE(Mem& mem, Registers& regs, bool pal, CICType cicType);
  void UpdateController(Controller);
  bool ReadButtons(u8*) const;
  void ControllerID(u8*) const;
  void MempakRead(const u8*, u8*);
  void MempakWrite(u8*, u8*);
  void EepromRead(const u8*, u8*, const Mem&) const;
  void EepromWrite(const u8*, u8*, const Mem&);
  std::vector<u8> Serialize();

  bool gamepadConnected = false, mempakOpen = false;
  SDL_GameController* gamepad{};
  JoybusDevice joybusDevices[6]{};
  u8 bootrom[PIF_BOOTROM_SIZE]{}, ram[PIF_RAM_SIZE]{};
  mio::mmap_sink mempak, eeprom;
  int channel = 0;
  std::string mempakPath{}, eepromPath{};
  size_t eepromSize{};

  FORCE_INLINE u8 Read(u32 addr) {
    addr &= 0x7FF;
    if(addr < 0x7c0) return bootrom[addr];
    return ram[addr & PIF_RAM_DSIZE];
  }

  FORCE_INLINE void Write(u32 addr, u8 val) {
    addr &= 0x7FF;
    if(addr < 0x7c0) return;
    ram[addr & PIF_RAM_DSIZE] = val;
  }

  FORCE_INLINE AccessoryType getAccessoryType() const {
    if (channel >= 4 || joybusDevices[channel].type != JOYBUS_CONTROLLER) {
      return ACCESSORY_NONE;
    } else {
      return joybusDevices[channel].accessoryType;
    }
  }
};
}