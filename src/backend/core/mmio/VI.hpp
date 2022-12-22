#pragma once
#include <common.hpp>

namespace n64 {
union VIBurst {
  /*struct {
    unsigned hsyncW:8;
    unsigned burstW:8;
    unsigned vsyncW:4;
    unsigned burstStart:10;
    unsigned:2;
  };*/
  u32 raw;
};

union VIHsyncLeap {
  /*struct {
    unsigned leapB:10;
    unsigned:6;
    unsigned leapA:10;
    unsigned:6;
  };*/
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

union AxisScale {
  u32 raw;
  struct {
    unsigned scaleDecimal:10;
    unsigned scaleInteger:2;
    unsigned subpixelOffsetDecimal:10;
    unsigned subpixelOffsetInteger:2;
    unsigned:4;
  };
  struct {
    unsigned scale:12;
    unsigned subpixelOffset:12;
    unsigned:4;
  };
};

enum VIFormat {
  blank = 0,
  reserved = 1,
  f5553 = 2,
  f8888 = 3
};

union VIStatus {
  struct {
    u8 type:2;
    bool gamma_dither_enable:1;
    bool gamma_enable:1;
    bool divot_enable:1;
    bool reserved_always_off:1;
    bool serrate:1;
    bool reserved_diagnostics_only:1;
    unsigned antialias_mode:3;
    unsigned:21;
  };

  u32 raw;
};

union AxisStart {
  u32 raw;
  struct {
    unsigned end:10;
    unsigned:6;
    unsigned start:10;
    unsigned:6;
  };
};

struct MI;
struct Registers;

struct VI {
  VI();
  void Reset();
  [[nodiscard]] u32 Read(u32) const;
  void Write(MI&, Registers&, u32, u32);
  AxisScale xscale{}, yscale{};
  VIHsyncLeap hsyncLeap{};
  VIStatus status{};
  VIBurst burst{};
  u32 vburst{};
  u32 origin, width, current;
  u32 vsync, hsync, intr;
  AxisStart hstart{}, vstart{};
  int swaps{};
  int numHalflines;
  int numFields;
  int cyclesPerHalfline;
};
} // backend
