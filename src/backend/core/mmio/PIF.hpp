#pragma once
#include <MemoryRegions.hpp>
#include <SDL_gamecontroller.h>
#include <GameDB.hpp>
#include <filesystem>

namespace fs = std::filesystem;

namespace n64 {

enum AccessoryType {
  ACCESSORY_NONE,
  ACCESSORY_MEMPACK,
  ACCESSORY_RUMBLE_PACK
};

struct Controller {
  union {
    u8 byte1;
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
    u8 byte2;
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
};

static_assert(sizeof(Controller) == 4);

enum JoybusType {
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
};

struct Mem;
struct Registers;

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

struct CartInfo;

struct PIF {
  ~PIF();
  void Reset();
  void LoadMempak(fs::path);
  void LoadEeprom(SaveType, fs::path);
  void ProcessCommands(Mem&);
  void InitDevices(SaveType);
  void CICChallenge();
  void ExecutePIF(Mem& mem, Registers& regs);
  void DoPIFHLE(Mem& mem, Registers& regs, bool pal, CICType cicType);
  void UpdateController();
  bool ReadButtons(u8*) const;
  void ControllerID(u8*) const;
  void MempakRead(u8*, u8*) const;
  void MempakWrite(u8*, u8*) const;
  void EepromRead(u8*, u8*, const Mem&) const;
  void EepromWrite(u8*, u8*, const Mem&) const;

  bool gamepadConnected = false;
  SDL_GameController* gamepad{};
  JoybusDevice joybusDevices[6]{};
  u8 bootrom[PIF_BOOTROM_SIZE]{}, ram[PIF_RAM_SIZE]{}, *mempak = nullptr, *eeprom = nullptr;
  int channel = 0;
  std::string mempakPath{}, eepromPath{};

  size_t eepromSize{};

  u8 Read(u32 addr) {
    addr &= 0x7FF;
    if(addr < 0x7c0) return bootrom[addr];
    return ram[addr & PIF_RAM_DSIZE];
  }

  void Write(u32 addr, u8 val) {
    addr &= 0x7FF;
    if(addr < 0x7c0) return;
    ram[addr & PIF_RAM_DSIZE] = val;
  }

  inline AccessoryType getAccessoryType() const {
    if (channel >= 4 || joybusDevices[channel].type != JOYBUS_CONTROLLER) {
      return ACCESSORY_NONE;
    } else {
      return joybusDevices[channel].accessoryType;
    }
  }
};
}