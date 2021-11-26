#pragma once
#define REGS_UNION(f, s) \
  union { \
    struct { \
      u8 ##s; \
      u8 ##f; \
    } PACKED; \
    u16 ##fs; \
  };
