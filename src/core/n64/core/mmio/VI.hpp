#pragma once
#include <common.hpp>

namespace natsukashii::n64::core {
union VIBurst {
  struct {
    unsigned hsyncW:8;
    unsigned burstW:8;
    unsigned vsyncW:4;
    unsigned burstStart:10;
    unsigned:2;
  };
  u32 raw;
};

union VIHsyncLeap {
  struct {
    unsigned leapB:10;
    unsigned:6;
    unsigned leapA:10;
    unsigned:6;
  };
  u32 raw;
} ;

union VIVideo {
  struct {
    unsigned end:10;
    unsigned:6;
    unsigned start:10;
    unsigned:6;
  };
  u32 raw;
};

union VIScale {
  struct {
    unsigned scale:12;
    unsigned:4;
    unsigned offset:12;
    unsigned:4;
  };
  u32 raw;
};

enum VIFormat {
  blank = 0,
  reserved = 1,
  f5553 = 2,
  f8888 = 3
};

union VIStatus {
  struct {
    u8 format:2;
    unsigned serrate:1;
    unsigned:29;
  };

  u32 raw;
};

struct MI;
struct Registers;

struct VI {
  VI();
  u32 Read(u32);
  void Write(MI&, Registers&, u32, u32);
  VIScale xscale{}, yscale{};
  VIVideo hvideo{}, vvideo{};
  VIHsyncLeap hsyncLeap{};
  VIStatus status{};
  VIBurst burst{}, vburst{};
  u32 origin, width, current;
  u32 vsync, hsync, intr;
  u32 hstart{}, vstart{};
  int swaps{};
  int numHalflines;
  int numFields;
  int cyclesPerHalfline;
};
} // natsukashii::n64::core
