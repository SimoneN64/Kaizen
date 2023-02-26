#pragma once
#include <MemoryRegions.hpp>
#include <SDL_gamecontroller.h>

namespace n64 {

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
  s8 joy_x;
  s8 joy_y;
};

static_assert(sizeof(Controller) == 4);

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
  void ProcessPIFCommands(Mem&);
  void ExecutePIF(Mem& mem, Registers& regs);
  void DoPIFHLE(Mem& mem, Registers& regs, bool pal, CICType cicType);
  void UpdateController();
  bool gamepadConnected = false;
  SDL_GameController* gamepad;
  Controller controller;
  u8 pifBootrom[PIF_BOOTROM_SIZE]{}, pifRam[PIF_RAM_SIZE];
  u8 Read(u32 addr) {
    addr &= 0x7FF;
    if(addr < 0x7c0) return pifBootrom[addr];
    return pifRam[addr];
  }

  void Write(u32 addr, u8 val) {
    addr &= 0x7FF;
    if(addr < 0x7c0) return;
    pifRam[addr] = val;
  }
};
}