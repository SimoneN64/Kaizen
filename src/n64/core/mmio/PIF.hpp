#pragma once
#include <common.hpp>

namespace n64 {

union Controller {
  struct {
    u8 b1, b2;
    s8 b3, b4;
  } __attribute__((__packed__));
  u32 raw;
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