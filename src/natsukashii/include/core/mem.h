#pragma once
#include <common.h>

typedef struct {
  union {
    struct {
      unsigned:2;
      unsigned select_btn:1;
      unsigned select_dpad:1;
      unsigned btn_start_down:1;
      unsigned btn_select_up:1;
      unsigned btn_a_left:1;
      unsigned btn_b_right:1;
    } PACKED;
    u8 raw;
  };

  bool dpad, button;
} joyp_t;

INLINE void joyp_write(joyp_t* joyp, u8 val) {
  joyp->raw = val;
  joyp->raw |= 0b11000000;
}

typedef struct {
  u8 bootrom, intf;
  u8 nr41, nr42, nr43, nr44, nr50, nr51, nr52;
  joyp_t joyp;
} io_t;

typedef struct {
  u8 ie;
  io_t io;
  u8 bootrom[BOOTROM_SIZE];
  u8 wram[WRAM_SIZE];
  u8 hram[HRAM_SIZE];
} mem_t;