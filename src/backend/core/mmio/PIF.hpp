#pragma once
#include <common.hpp>

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

const u32 cicSeeds[] = {
  0x0,
  0x00043F3F, // CIC_NUS_6101
  0x00043F3F, // CIC_NUS_7102
  0x00003F3F, // CIC_NUS_6102_7101
  0x0000783F, // CIC_NUS_6103_7103
  0x0000913F, // CIC_NUS_6105_7105
  0x0000853F, // CIC_NUS_6106_7106
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

void ProcessPIFCommands(u8*, Controller&, Mem&);
void DoPIFHLE(Mem& mem, Registers& regs, CartInfo cartInfo);
}