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
#define CheckFPUUsable_PreserveCause() do { if(!regs.cop0.status.cu1) { FireException(regs, ExceptionCode::CoprocessorUnusable, 1, regs.oldPC); return; } } while(0)
#define CheckFPUUsable() do { CheckFPUUsable_PreserveCause(); regs.cop1.fcr31.cause = 0; } while(0)
  Cop1();
  u32 fcr0{};
  FCR31 fcr31{};
  FloatingPointReg fgr[32]{};
  void Reset();
  template <class T> // either JIT or Interpreter
  void decode(T&, u32);
  friend struct Interpreter;
  friend struct JIT;

  void SetCauseUnimplemented();
  void SetCauseUnderflow();
  void SetCauseInexact();
  void SetCauseDivisionByZero();
  void SetCauseOverflow();
  void SetCauseInvalid();
  int fp_class=0;
private:
  template <typename T> auto FGR(Cop0Status&, u32) -> T&;
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
  void cf(Registers&, u32 instr);
  template <typename T>
  void cun(Registers&, u32 instr);
  template <typename T>
  void ceq(Registers&, u32 instr);
  template <typename T>
  void cueq(Registers&, u32 instr);
  template <typename T>
  void colt(Registers&, u32 instr);
  template <typename T>
  void cult(Registers&, u32 instr);
  template <typename T>
  void cole(Registers&, u32 instr);
  template <typename T>
  void cule(Registers&, u32 instr);
  template <typename T>
  void csf(Registers&, u32 instr);
  template <typename T>
  void cngle(Registers&, u32 instr);
  template <typename T>
  void cseq(Registers&, u32 instr);
  template <typename T>
  void cngl(Registers&, u32 instr);
  template <typename T>
  void clt(Registers&, u32 instr);
  template <typename T>
  void cnge(Registers&, u32 instr);
  template <typename T>
  void cle(Registers&, u32 instr);
  template <typename T>
  void cngt(Registers&, u32 instr);
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
