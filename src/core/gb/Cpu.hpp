#pragma once
#include <common.hpp>

namespace natsukashii::core {
template <class A, class B>
struct RegisterPair {
  A low;
  B high;
  auto operator=(const u16& rhs) {
    low = rhs & 0xff;
    high = rhs >> 8;
    return *this;
  }
};

union RegF {
  struct {
    unsigned z:1;
    unsigned n:1;
    unsigned h:1;
    unsigned c:1;
    unsigned:4;
  };
  u8 raw;
};

struct Cpu {
private:
  RegisterPair<u8, RegF> af;
  RegisterPair<u8, u8> bc;
  RegisterPair<u8, u8> de;
  RegisterPair<u8, u8> hl;
};
}
