#pragma once
#include "Mem.hpp"

namespace natsukashii::core {
#define af regs.AF
#define bc regs.BC
#define de regs.DE
#define hl regs.HL
#define pc regs.PC
#define sp regs.SP

#define a af.A
#define f af.F
#define b bc.B
#define c bc.C
#define d de.D
#define e de.E
#define h hl.H
#define l hl.L

#define REGIMPL(type1, reg1, type2, reg2) \
  struct reg##reg1##reg2 { \
    reg##reg1##reg2() {} \
    union { \
      type1 reg1; \
      type2 reg2; \
    }; \
    u16 raw = 0; \
    reg##reg1##reg2& operator=(const u16& rhs) { \
      reg1 = rhs >> 8; \
      reg2 = rhs & 0xff; \
      return *this; \
    } \
  } reg1##reg2

struct RegF {
  RegF() : raw(0) {}
  RegF(const u8& val) : raw(val) {}
  u8 raw = 0;

  RegF& operator=(const u8& rhs) {
    raw |= ((rhs >> 7) << 7);
    raw |= ((rhs >> 6) << 6);
    raw |= ((rhs >> 5) << 5);
    raw |= ((rhs >> 4) << 4);

    return *this;
  }

  bool zero() { return (raw >> 7) & 1; }
  bool negative() { return (raw >> 6) & 1; }
  bool halfcarry() { return (raw >> 5) & 1; }
  bool carry() { return (raw >> 4) & 1; }

  void zero(const bool& rhs) {
    raw &= ~0xF;
    raw |= (rhs << 7);
  }

  void negative(const bool& rhs) {
    raw &= ~0xF;
    raw |= (rhs << 6);
  }

  void halfcarry(const bool& rhs) {
    raw &= ~0xF;
    raw |= (rhs << 5);
  }

  void carry(const bool& rhs) {
    raw &= ~0xF;
    raw |= (rhs << 4);
  }
};

struct Registers {
  REGIMPL(u8, A, RegF, F);
  REGIMPL(u8, B, u8, C);
  REGIMPL(u8, C, u8, E);
  REGIMPL(u8, D, u8, L);
  u16 PC = 0, SP = 0;
};

struct Cpu {
  Cpu();
  void Step(Mem&);
private:
  void DecodeAndExecute(u8);
  Registers regs;
};
}
