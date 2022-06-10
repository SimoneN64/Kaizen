#pragma once
#include <Mem.hpp>
#include <util.hpp>

namespace natsukashii::gb::core {
template <class T1, class T2>
struct Reg {
  Reg() : raw(0) {};
  union {
    T1 hi;
    T2 lo;
  };
  u16 raw = 0;
};

struct RegF {
  RegF() : raw(0) {}
  RegF(const u8& val) : raw(val) {}

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

  void reset() {
    zero(false);
    negative(false);
    halfcarry(false);
    carry(false);
  }

  void set(bool z, bool n, bool hc, bool ca) {
    zero(z);
    negative(n);
    halfcarry(hc);
    carry(ca);
  }

  u8& get() { return raw; }
private:
  u8 raw = 0;

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
  Reg<u8, RegF> AF;
  Reg<u8,   u8> BC;
  Reg<u8,   u8> DE;
  Reg<u8,   u8> HL;
  u16 pc = 0, sp = 0;

    u8& a() { return AF.hi; }
  RegF& f() { return AF.lo; }
    u8& b() { return BC.hi; }
    u8& c() { return BC.lo; }
    u8& d() { return DE.hi; }
    u8& e() { return DE.lo; }
    u8& h() { return HL.hi; }
    u8& l() { return HL.lo; }
  
  u16& af() { return AF.raw; }
  u16& bc() { return BC.raw; }
  u16& de() { return DE.raw; }
  u16& hl() { return HL.raw; } 
};

struct Cpu {
  Cpu();
  void Step(Mem&);
private:
  void FetchDecodeExecute(Mem& mem);
  Registers regs;

  template <int group>
  u16 GetR16(u8 bits) {
    static_assert(group > 0 && group < 3, "Invalid GetR16 group");
    if constexpr (group == 1) {
      switch(bits & 3) {
        case 0: return regs.bc();
        case 1: return regs.de();
        case 2: return regs.hl();
        case 3: return regs.sp;
      }
    } else if constexpr (group == 2) {
      switch(bits & 3) {
        case 0: return regs.bc();
        case 1: return regs.de();
        case 2: return regs.hl()++;
        case 3: return regs.hl()--;
      }
    } else if constexpr (group == 3) {
      switch(bits & 3) {
        case 0: return regs.bc();
        case 1: return regs.de();
        case 2: return regs.hl();
        case 3: return regs.af();
      }
    }
    return 0;
  }

  template <int group>
  void SetR16(u8 bits, u16 val) {
    static_assert(group > 0 && group < 3, "Invalid SetR16 group");
    if constexpr (group == 1) {
      switch(bits & 3) {
        case 0: regs.bc() = val; break;
        case 1: regs.de() = val; break;
        case 2: regs.hl() = val; break;
        case 3: regs.sp = val; break;
      }
    } else if constexpr (group == 2) {
      switch(bits & 3) {
        case 0: regs.bc() = val; break;
        case 1: regs.de() = val; break;
        case 2: regs.hl() = val; regs.hl()++; break;
        case 3: regs.hl() = val; regs.hl()--; break;
      }
    } else if constexpr (group == 3) {
      switch(bits & 3) {
        case 0: regs.bc() = val; break;
        case 1: regs.de() = val; break;
        case 2: regs.hl() = val; break;
        case 3: regs.af() = val; break;
      }
    }
  }

  u8 GetR8(u8 bits, Mem& mem) {
    switch(bits & 7) {
      case 0: return regs.b();
      case 1: return regs.c();
      case 2: return regs.d();
      case 3: return regs.e();
      case 4: return regs.h();
      case 5: return regs.l();
      case 6: return mem.Read8(regs.hl());
      case 7: return regs.a();
    }
    return 0;
  }

  void SetR8(u8 bits, u8 val, Mem& mem) {
    switch(bits & 7) {
      case 0: regs.b() = val; break;
      case 1: regs.c() = val; break;
      case 2: regs.d() = val; break;
      case 3: regs.e() = val; break;
      case 4: regs.h() = val; break;
      case 5: regs.l() = val; break;
      case 6: return mem.Write8(regs.hl(), val);
      case 7: regs.a() = val; break;
    }
  }
};
}
