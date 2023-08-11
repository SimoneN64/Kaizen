#pragma once
#include <core/registers/Cop0.hpp>
#include <cstring>

namespace n64 {
union FCR31 {
  struct {
    unsigned rounding_mode:2;
    unsigned flag_inexact_operation:1;
    unsigned flag_underflow:1;
    unsigned flag_overflow:1;
    unsigned flag_division_by_zero:1;
    unsigned flag_invalid_operation:1;
    unsigned enable_inexact_operation:1;
    unsigned enable_underflow:1;
    unsigned enable_overflow:1;
    unsigned enable_division_by_zero:1;
    unsigned enable_invalid_operation:1;
    unsigned cause_inexact_operation:1;
    unsigned cause_underflow:1;
    unsigned cause_overflow:1;
    unsigned cause_division_by_zero:1;
    unsigned cause_invalid_operation:1;
    unsigned cause_unimplemented_operation:1;
    unsigned:5;
    unsigned compare:1;
    unsigned fs:1;
    unsigned:7;
  } __attribute__((__packed__));

  struct {
    unsigned:2;
    unsigned flag:5;
    unsigned enable:5;
    unsigned cause:6;
    unsigned:14;
  } __attribute__((__packed__));

  u32 raw;
};

enum CompConds {
  F, UN, EQ, UEQ,
  OLT, ULT, OLE, ULE,
  SF, NGLE, SEQ, NGL,
  LT, NGE, LE, NGT
};

union FGR {
  struct {
    s32 lo:32;
    s32 hi:32;
  } __attribute__((__packed__));

  s64 raw;
};

struct Interpreter;
struct JIT;
struct Registers;

struct Cop1 {
  Cop1();
  u32 fcr0{};
  FCR31 fcr31{};
  FGR fgr[32]{};
  void Reset();
  template <class T> // either JIT or Interpreter
  void decode(T&, u32);
  friend struct Interpreter;
  friend struct JIT;

  template <typename T>
  FORCE_INLINE void SetReg(Cop0& cop0, u8 index, T value) {
    if constexpr(sizeof(T) == 4) {
      if (cop0.status.fr) {
        fgr[index].lo = value;
      } else {
        if (index & 1) {
          fgr[index & ~1].hi = value;
        } else {
          fgr[index].lo = value;
        }
      }
    } else if constexpr(sizeof(T) == 8) {
      if(!cop0.status.fr) {
        index &= ~1;
      }

      fgr[index].raw = value;
    }
  }

  template <typename T>
  FORCE_INLINE T GetReg(Cop0& cop0, u8 index) {
    if constexpr(sizeof(T) == 4) {
      if(cop0.status.fr) {
        return fgr[index].lo;
      } else {
        if (index & 1) {
          return fgr[index & ~1].hi;
        } else {
          return fgr[index].lo;
        }
      }
    } else if constexpr(sizeof(T) == 8) {
      if(!cop0.status.fr) {
        index &= ~1;
      }

      return fgr[index].raw;
    }
  }

  template <typename T>
  FORCE_INLINE void SetCop1Reg(Cop0& cop0, u8 index, T value) {
    if constexpr (sizeof(T) == 4) {
      u32 raw;
      memcpy(&raw, &value, sizeof(T));
      SetReg<u32>(cop0, index, raw);
    } else if constexpr (sizeof(T) == 8) {
      u64 raw;
      memcpy(&raw, &value, sizeof(T));
      SetReg<u64>(cop0, index, raw);
    }
  }

  template <typename T>
  FORCE_INLINE T GetCop1Reg(Cop0& cop0, u8 index) {
    T value;
    if constexpr (sizeof(T) == 4) {
      u32 raw = GetReg<u32>(cop0, index);
      memcpy(&value, &raw, sizeof(T));
    } else if constexpr (sizeof(T) == 8) {
      u64 raw = GetReg<u64>(cop0, index);
      memcpy(&value, &raw, sizeof(T));
    }
    return value;
  }
private:
  void decodeInterp(Interpreter&, u32);
  void decodeJIT(JIT&, u32);
  void absd(Registers&, u32 instr);
  void abss(Registers&, u32 instr);
  void absw(Registers&, u32 instr);
  void absl(Registers&, u32 instr);
  void adds(Registers&, u32 instr);
  void addd(Registers&, u32 instr);
  void subs(Registers&, u32 instr);
  void subd(Registers&, u32 instr);
  void subw(Registers&, u32 instr);
  void subl(Registers&, u32 instr);
  void ceills(Registers&, u32 instr);
  void ceilws(Registers&, u32 instr);
  void ceilld(Registers&, u32 instr);
  void ceilwd(Registers&, u32 instr);
  void cfc1(Registers&, u32 instr) const;
  void ctc1(Registers&, u32 instr);
  void roundls(Registers&, u32 instr);
  void roundld(Registers&, u32 instr);
  void roundws(Registers&, u32 instr);
  void roundwd(Registers&, u32 instr);
  void floorls(Registers&, u32 instr);
  void floorld(Registers&, u32 instr);
  void floorws(Registers&, u32 instr);
  void floorwd(Registers&, u32 instr);
  void cvtls(Registers&, u32 instr);
  void cvtws(Registers&, u32 instr);
  void cvtds(Registers&, u32 instr);
  void cvtsw(Registers&, u32 instr);
  void cvtdw(Registers&, u32 instr);
  void cvtsd(Registers&, u32 instr);
  void cvtwd(Registers&, u32 instr);
  void cvtld(Registers&, u32 instr);
  void cvtdl(Registers&, u32 instr);
  void cvtsl(Registers&, u32 instr);
  template <typename T>
  void ccond(Registers&, u32 instr, CompConds);
  void divs(Registers&, u32 instr);
  void divd(Registers&, u32 instr);
  void muls(Registers&, u32 instr);
  void muld(Registers&, u32 instr);
  void mulw(Registers&, u32 instr);
  void mull(Registers&, u32 instr);
  void movs(Registers&, u32 instr);
  void movd(Registers&, u32 instr);
  void movw(Registers&, u32 instr);
  void movl(Registers&, u32 instr);
  void negs(Registers&, u32 instr);
  void negd(Registers&, u32 instr);
  void sqrts(Registers&, u32 instr);
  void sqrtd(Registers&, u32 instr);
  template<class T>
  void lwc1(T&, Mem&, u32);
  template<class T>
  void swc1(T&, Mem&, u32);
  template<class T>
  void ldc1(T&, Mem&, u32);
  template<class T>
  void sdc1(T&, Mem&, u32);

  void lwc1Interp(Registers&, Mem&, u32);
  void swc1Interp(Registers&, Mem&, u32);
  void ldc1Interp(Registers&, Mem&, u32);
  void sdc1Interp(Registers&, Mem&, u32);
  void lwc1JIT(JIT&, Mem&, u32);
  void swc1JIT(JIT&, Mem&, u32);
  void ldc1JIT(JIT&, Mem&, u32);
  void sdc1JIT(JIT&, Mem&, u32);
  void mfc1(Registers&, u32 instr);
  void dmfc1(Registers&, u32 instr);
  void mtc1(Registers&, u32 instr);
  void dmtc1(Registers&, u32 instr);
  void truncws(Registers&, u32 instr);
  void truncwd(Registers&, u32 instr);
  void truncls(Registers&, u32 instr);
  void truncld(Registers&, u32 instr);
};
}
