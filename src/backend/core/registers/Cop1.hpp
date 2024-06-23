#pragma once
#include <core/registers/Cop0.hpp>
#include <cstring>

namespace n64 {
struct Cop1;

union FCR31 {
  FCR31() = default;
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

  [[nodiscard]] u32 read() const {
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

union FloatingPointReg {
  struct {
    s32 int32;
    s32 int32h;
  } __attribute__((__packed__));
  struct {
    u32 uint32;
    u32 uint32h;
  } __attribute__((__packed__));
  struct {
    s64 int64;
  } __attribute__((__packed__));
  struct {
    u64 uint64;
  } __attribute__((__packed__));
  struct {
    float float32;
    float float32h;
  } __attribute__((__packed__));
  struct {
    double float64;
  } __attribute__((__packed__));
};

struct Interpreter;
struct JIT;
struct Registers;

struct Cop1 {
  explicit Cop1(Registers&);
  u32 fcr0{};
  FCR31 fcr31{};
  FloatingPointReg fgr[32]{};

  void Reset();
  template <class T> // either JIT or Interpreter
  void decode(T&, u32);
  friend struct Interpreter;

  bool CheckFPUException();
  bool FireException();
  template <bool preserveCause = false>
  bool CheckFPUUsable();
  template <typename T>
  bool CheckResult(T&);
  template <typename T>
  bool CheckArg(T&);
  template <typename T>
  bool CheckArgs(T&, T&);

  template<typename T, bool quiet, bool cf>
  bool XORDERED(T fs, T ft);

  template <typename T>
  bool CheckCVTArg(float &f);
  template <typename T>
  bool CheckCVTArg(double &f);

  template <typename T>
  void SetCauseByArg(T f);
  template <bool cvt = false>
  bool TestExceptions();
  bool SetCauseUnimplemented();
  bool SetCauseUnderflow();
  bool SetCauseInexact();
  bool SetCauseDivisionByZero();
  bool SetCauseOverflow();
  bool SetCauseInvalid();
private:
  template <typename T>
  auto FGR_T(Cop0Status&, u32) -> T&;
  template <typename T>
  auto FGR_S(Cop0Status&, u32) -> T&;
  template <typename T>
  auto FGR_D(Cop0Status&, u32) -> T&;
  void decodeInterp(Interpreter&, u32);
  void decodeJIT(JIT&, u32);
  void absd(u32 instr);
  void abss(u32 instr);
  void adds(u32 instr);
  void addd(u32 instr);
  void subs(u32 instr);
  void subd(u32 instr);
  void ceills(u32 instr);
  void ceilws(u32 instr);
  void ceilld(u32 instr);
  void ceilwd(u32 instr);
  void cfc1(u32 instr);
  void ctc1(u32 instr);
  void unimplemented();
  void roundls(u32 instr);
  void roundld(u32 instr);
  void roundws(u32 instr);
  void roundwd(u32 instr);
  void floorls(u32 instr);
  void floorld(u32 instr);
  void floorws(u32 instr);
  void floorwd(u32 instr);
  void cvtls(u32 instr);
  void cvtws(u32 instr);
  void cvtds(u32 instr);
  void cvtsw(u32 instr);
  void cvtdw(u32 instr);
  void cvtsd(u32 instr);
  void cvtwd(u32 instr);
  void cvtld(u32 instr);
  void cvtdl(u32 instr);
  void cvtsl(u32 instr);
  template <typename T>
  void cf(u32 instr);
  template <typename T>
  void cun(u32 instr);
  template <typename T>
  void ceq(u32 instr);
  template <typename T>
  void cueq(u32 instr);
  template <typename T>
  void colt(u32 instr);
  template <typename T>
  void cult(u32 instr);
  template <typename T>
  void cole(u32 instr);
  template <typename T>
  void cule(u32 instr);
  template <typename T>
  void csf(u32 instr);
  template <typename T>
  void cngle(u32 instr);
  template <typename T>
  void cseq(u32 instr);
  template <typename T>
  void cngl(u32 instr);
  template <typename T>
  void clt(u32 instr);
  template <typename T>
  void cnge(u32 instr);
  template <typename T>
  void cle(u32 instr);
  template <typename T>
  void cngt(u32 instr);
  void divs(u32 instr);
  void divd(u32 instr);
  void muls(u32 instr);
  void muld(u32 instr);
  void movs(u32 instr);
  void movd(u32 instr);
  void negs(u32 instr);
  void negd(u32 instr);
  void sqrts(u32 instr);
  void sqrtd(u32 instr);
  template<class T>
  void lwc1(T&, Mem&, u32);
  template<class T>
  void swc1(T&, Mem&, u32);
  template<class T>
  void ldc1(T&, Mem&, u32);
  template<class T>
  void sdc1(T&, Mem&, u32);

  void lwc1Interp(Mem&, u32);
  void swc1Interp(Mem&, u32);
  void ldc1Interp(Mem&, u32);
  void sdc1Interp(Mem&, u32);
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
  void mfc1(u32 instr);
  void dmfc1(u32 instr);
  void mtc1(u32 instr);
  void dmtc1(u32 instr);
  void truncws(u32 instr);
  void truncwd(u32 instr);
  void truncls(u32 instr);
  void truncld(u32 instr);

  Registers& regs;
};
}
