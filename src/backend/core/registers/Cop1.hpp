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

  u32 read() const {
    return (fs << 24) | (compare << 23) | (cause << 12) | (enable << 7) | (flag << 2) | rounding_mode;
  }

  void write(u32 val) {
    fs = (val & 0x01000000) >> 24;
    compare = (val & 0x00800000) >> 23;
    cause = (val & 0x0003f000) >> 12;
    enable = (val & 0x00000f80) >> 7;
    flag = (val & 0x0000007c) >> 2;
    rounding_mode = val & 3;
  }
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
#define CheckFPUUsable_PreserveCause() do { if(!regs.cop0.status.cu1) { FireException(regs, ExceptionCode::CoprocessorUnusable, 1, regs.oldPC); return; } } while(0)
#define CheckFPUUsable() do { CheckFPUUsable_PreserveCause(); regs.cop1.fcr31.cause = 0; } while(0)
  Cop1();
  u32 fcr0{};
  FCR31 fcr31{};
  FGR fgr[32]{};
  void Reset();
  template <class T> // either JIT or Interpreter
  void decode(T&, u32);
  friend struct Interpreter;
  friend struct JIT;

  void SetCauseUnimplemented(Registers&);
  void SetCauseUnderflow(Registers&);
  void SetCauseInexact(Registers&);
  void SetCauseDivisionByZero(Registers&);
  void SetCauseOverflow(Registers&);
  void SetCauseInvalid(Registers&);

  template<typename T>
  FORCE_INLINE T GetFGR_FR(Cop0& cop0, u8 r) {
    if constexpr (std::is_same_v<T, u32> || std::is_same_v<T, s32>) {
      if (cop0.status.fr) {
        return fgr[r].lo;
      } else {
        if (r & 1) {
          return fgr[r & ~1].hi;
        } else {
          return fgr[r].lo;
        }
      }
    } else if constexpr (std::is_same_v<T, u64> || std::is_same_v<T, s64>) {
      if (!cop0.status.fr) {
        // When this bit is not set, accessing odd registers is not allowed.
        r &= ~1;
      }

      return fgr[r].raw;
    }
  }

  template<typename T>
  FORCE_INLINE void SetFGR_FR(Cop0& cop0, u8 r, T value) {
    if constexpr (std::is_same_v<T, u32> || std::is_same_v<T, s32>) {
      if (cop0.status.fr) {
        fgr[r].lo = value;
      } else {
        if (r & 1) {
          fgr[r & ~1].hi = value;
        } else {
          fgr[r].lo = value;
        }
      }
    } else if constexpr (std::is_same_v<T, u64> || std::is_same_v<T, s64>) {
      if (!cop0.status.fr) {
        // When this bit is not set, accessing odd registers is not allowed.
        r &= ~1;
      }

      fgr[r].raw = value;
    }
  }

  template<typename T>
  FORCE_INLINE void SetFGR(u8 r, T value) {
    fgr[r].raw = value;
  }

  template<typename T>
  FORCE_INLINE u64 GetFGR(u8 r) {
    if constexpr (std::is_same_v<T, u32> || std::is_same_v<T, s32>) {
      return fgr[r].lo;
    } else if constexpr (std::is_same_v<T, u64> || std::is_same_v<T, s64>) {
      return fgr[r].raw;
    }
  }

  template <typename T>
  FORCE_INLINE T GetFGR_FS(Cop0& cop0, u8 fs) {
    if constexpr (std::is_same_v<T, u32> || std::is_same_v<T, s32>) {
      if (!cop0.status.fr) {
        fs &= ~1;
      }
      return fgr[fs].lo;
    } else if constexpr (std::is_same_v<T, float> || std::is_same_v<T, double>) {
      if (!cop0.status.fr) {
        fs &= ~1;
      }
      return GetFGR_Raw<T>(fs);
    }
  }

  template <typename T>
  FORCE_INLINE T GetFGR_Raw(u8 r) {
    if constexpr (std::is_same_v<T, float>) {
      static_assert(sizeof(float) == sizeof(u32), "float and u32 need to both be 32 bits for this to work.");
      auto rawvalue = GetFGR<u32>(r);
      float floatvalue;
      memcpy(&floatvalue, &rawvalue, sizeof(float));
      return floatvalue;
    } else if constexpr (std::is_same_v<T, double>) {
      static_assert(sizeof(double) == sizeof(u64), "double and u64 need to both be 64 bits for this to work.");
      double doublevalue;
      auto rawvalue = GetFGR<u64>(r);
      memcpy(&doublevalue, &rawvalue, sizeof(double));
      return doublevalue;
    }
  }

  template <typename T>
  FORCE_INLINE void SetFGR_Raw(u8 r, T val) {
    if constexpr (std::is_same_v<T, float>) {
      static_assert(sizeof(float) == sizeof(u32), "float and u32 need to both be 32 bits for this to work.");

      u32 rawvalue;
      memcpy(&rawvalue, &val, sizeof(float));
      SetFGR<u32>(r, rawvalue);
    } else if constexpr (std::is_same_v<T, double>) {
      static_assert(sizeof(double) == sizeof(u64), "double and u64 need to both be 64 bits for this to work.");

      u64 rawvalue;
      memcpy(&rawvalue, &val, sizeof(double));
      SetFGR<u64>(r, rawvalue);
    }
  }

  template <typename T>
  FORCE_INLINE T GetFGR_FT(u8 ft) {
    return GetFGR_Raw<T>(ft);
  }
private:
  void decodeInterp(Interpreter&, u32);
  void decodeJIT(JIT&, u32);
  void absd(Registers&, u32 instr);
  void abss(Registers&, u32 instr);
  void adds(Registers&, u32 instr);
  void addd(Registers&, u32 instr);
  void subs(Registers&, u32 instr);
  void subd(Registers&, u32 instr);
  void ceills(Registers&, u32 instr);
  void ceilws(Registers&, u32 instr);
  void ceilld(Registers&, u32 instr);
  void ceilwd(Registers&, u32 instr);
  void cfc1(Registers&, u32 instr) const;
  void ctc1(Registers&, u32 instr);
  void unimplemented(Registers&);
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
  void movs(Registers&, u32 instr);
  void movd(Registers&, u32 instr);
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
  void lwc1JIT(JIT&, Mem&, u32) {
    Util::panic("[JIT]: lwc1 not implemented!");
  }
  void swc1JIT(JIT&, Mem&, u32) {
    Util::panic("[JIT]: swc1 not implemented!");
  }
  void ldc1JIT(JIT&, Mem&, u32) {
    Util::panic("[JIT]: ldc1 not implemented!");
  }
  void sdc1JIT(JIT&, Mem&, u32) {
    Util::panic("[JIT]: sdc1 not implemented!");
  }
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
