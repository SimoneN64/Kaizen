#include <core/registers/Cop1.hpp>
#include <core/registers/Registers.hpp>
#include <core/Interpreter.hpp>
#include <core/Mem.hpp>
#include <Float.hpp>

namespace n64 {
template<> auto Cop1::FGR<s32>(Cop0Status& status, u32 index) -> s32& {
  if (status.fr) {
    return fgr[index].int32l;
  } else {
    if (index & 1) {
      return fgr[index & ~1].int32h;
    } else {
      return fgr[index].int32l;
    }
  }
}

template<> auto Cop1::FGR<u32>(Cop0Status& status, u32 index) -> u32& {
  return (u32&)FGR<s32>(status, index);
}

template<> auto Cop1::FGR<f32>(Cop0Status& status, u32 index) -> f32& {
  if (status.fr) {
    return fgr[index].float32l;
  } else {
    if (index & 1) {
      return fgr[index & ~1].float32h;
    } else {
      return fgr[index].float32l;
    }
  }
}

template<> auto Cop1::FGR<s64>(Cop0Status& status, u32 index) -> s64& {
  if (status.fr) {
    return fgr[index].int64;
  } else {
    return fgr[index & ~1].int64;
  }
}

template<> auto Cop1::FGR<u64>(Cop0Status& status, u32 index) -> u64& {
  return (u64&)FGR<s64>(status, index);
}

template<> auto Cop1::FGR<f64>(Cop0Status& status, u32 index) -> f64& {
  if (status.fr) {
    return fgr[index].float64;
  } else {
    return fgr[index & ~1].float64;
  }
}

FORCE_INLINE bool FireFPUException(Registers& regs) {
  FCR31& fcr31 = regs.cop1.fcr31;
  u32 enable = fcr31.enable | (1 << 5);
  if(fcr31.cause & enable) {
    FireException(regs, ExceptionCode::FloatingPointError, 0, regs.oldPC);
    return true;
  }

  return false;
}

#define CheckFPUException() do { if(FireFPUException(regs)) { return; } } while(0)

FORCE_INLINE int PushRoundingMode(const FCR31& fcr31) {
  int og = fegetround();
  switch (fcr31.rounding_mode) {
    case 0: fesetround(FE_TONEAREST); break;
    case 1: fesetround(FE_TOWARDZERO); break;
    case 2: fesetround(FE_UPWARD); break;
    case 3: fesetround(FE_DOWNWARD); break;
  }

  return og;
}

void Cop1::SetCauseUnimplemented() {
  fcr31.cause_unimplemented_operation = true;
}

void Cop1::SetCauseUnderflow() {
  fcr31.cause_underflow = true;
  if(!fcr31.enable_underflow) {
    fcr31.flag_underflow = true;
  }
}

void Cop1::SetCauseInexact() {
  fcr31.cause_inexact_operation = true;
  if(!fcr31.enable_inexact_operation) {
    fcr31.flag_inexact_operation = true;
  }
}

void Cop1::SetCauseDivisionByZero() {
  fcr31.cause_division_by_zero = true;
  if(!fcr31.enable_division_by_zero) {
    fcr31.flag_division_by_zero = true;
  }
}

void Cop1::SetCauseOverflow() {
  fcr31.cause_overflow = true;
  if(!fcr31.enable_overflow) {
    fcr31.flag_overflow = true;
  }
}

void Cop1::SetCauseInvalid() {
  fcr31.cause_invalid_operation = true;
  if(!fcr31.enable_invalid_operation) {
    fcr31.flag_invalid_operation = true;
  }
}

#define PUSHROUNDING int orig_round = PushRoundingMode(fcr31)
#define POPROUNDING fesetround(orig_round)
#define OP_CheckExcept(op) do { feclearexcept(FE_ALL_EXCEPT); PUSHROUNDING; op; SetFPUCauseRaised(regs, fetestexcept(FE_ALL_EXCEPT)); POPROUNDING; } while(0)
#define CVT_OP_CheckExcept(op) do { feclearexcept(FE_ALL_EXCEPT); op; SetFPUCauseCVTRaised(regs, fetestexcept(FE_ALL_EXCEPT)); CheckFPUException(); } while(0)

#define OP(T, op) do { \
  CheckFPUUsable(); \
  auto fs = FGR<T>(regs.cop0.status, FS(instr)); \
  auto ft = FGR<T>(regs.cop0.status, FT(instr)); \
  CheckArg(fs); \
  CheckArg(ft); \
  T result; \
  OP_CheckExcept({result = (op);});             \
  CheckResult(result);                     \
  FGR<T>(regs.cop0.status, FD(instr)) = result; \
} while(0)

FORCE_INLINE void SetCauseByArgWCVT(Registers& regs, f32 f) {
  switch (std::fpclassify(f.operator float())) {
    case FP_NAN:
    case FP_INFINITE:
    case FP_SUBNORMAL:
      regs.cop1.SetCauseUnimplemented();
      CheckFPUException();
      break;

    case FP_NORMAL:
      // Check overflow
      if (f >= 2147483648.f || f < -2147483648.f) {
        regs.cop1.SetCauseUnimplemented();
        CheckFPUException();
      }
      break;

    case FP_ZERO:
      break; // Fine
  }
}

FORCE_INLINE void SetCauseByArgWCVT(Registers& regs, f64 f) {
  switch (std::fpclassify(f.operator double ())) {
    case FP_NAN:
    case FP_INFINITE:
    case FP_SUBNORMAL:
      regs.cop1.SetCauseUnimplemented();
      CheckFPUException();
      break;

    case FP_NORMAL:
      // Check overflow
      if (f >= 2147483648.f || f < -2147483648.f) {
        regs.cop1.SetCauseUnimplemented();
        CheckFPUException();
      }
      break;

    case FP_ZERO:
      break; // Fine
  }
}

FORCE_INLINE void SetCauseByArgLCVT(Registers& regs, f32 f) {
  switch (std::fpclassify(f.operator float ())) {
    case FP_NAN:
    case FP_INFINITE:
    case FP_SUBNORMAL:
      regs.cop1.SetCauseUnimplemented();
      CheckFPUException();
      break;

    case FP_NORMAL:
      // Check overflow
      if (f >= 9007199254740992.f || f <= -9007199254740992.f) {
        regs.cop1.SetCauseUnimplemented();
        CheckFPUException();
      }
      break;

    case FP_ZERO:
      break; // Fine
  }
}

FORCE_INLINE void SetCauseByArgLCVT(Registers& regs, f64 f) {
  switch (std::fpclassify(f.operator double ())) {
    case FP_NAN:
    case FP_INFINITE:
    case FP_SUBNORMAL:
      regs.cop1.SetCauseUnimplemented();
      CheckFPUException();
      break;

    case FP_NORMAL:
      // Check overflow
      if (f >= 9007199254740992.0 || f <= -9007199254740992.0) {
        regs.cop1.SetCauseUnimplemented();
        CheckFPUException();
      }
      break;

    case FP_ZERO:
      break; // Fine
  }
}

#define CheckWCVTArg(f) do { SetCauseByArgWCVT(regs, f); CheckFPUException(); } while(0)
#define CheckLCVTArg(f) do { SetCauseByArgLCVT(regs, f); CheckFPUException(); } while(0)

FORCE_INLINE void SetFPUCauseRaised(Registers& regs, int raised) {
  if (raised == 0) {
    return;
  }

  if (raised & FE_UNDERFLOW) {
    if (!regs.cop1.fcr31.fs || regs.cop1.fcr31.enable_underflow || regs.cop1.fcr31.enable_inexact_operation) {
      regs.cop1.SetCauseUnimplemented();
      return;
    } else {
      regs.cop1.SetCauseUnderflow();
    }
  }

  if (raised & FE_INEXACT) {
    regs.cop1.SetCauseInexact();
  }

  if (raised & FE_DIVBYZERO) {
    regs.cop1.SetCauseDivisionByZero();
  }

  if (raised & FE_OVERFLOW) {
    regs.cop1.SetCauseOverflow();
  }

  if (raised & FE_INVALID) {
    regs.cop1.SetCauseInvalid();
  }
}

FORCE_INLINE void SetFPUCauseCVTRaised(Registers& regs, int raised) {
  if(raised & FE_INVALID) {
    regs.cop1.SetCauseUnimplemented();
    return;
  }

  SetFPUCauseRaised(regs, raised);
}

#define F_TO_U32(f) (*((u32*)(&(f))))
#define D_TO_U64(d) (*((u64*)(&(d))))
#define U64_TO_D(d) (*((double*)(&(d))))
#define U32_TO_F(f) (*((float*)(&(f))))

FORCE_INLINE void SetCauseByArg(Registers& regs, f32& f) {
  auto fp_class = std::fpclassify(f.operator float());
  switch(fp_class) {
    case FP_NAN:
      if(f.is_quiet()) {
        regs.cop1.SetCauseInvalid();
        CheckFPUException();
      } else {
        regs.cop1.SetCauseUnimplemented();
        CheckFPUException();
      }
      break;
    case FP_SUBNORMAL:
      regs.cop1.SetCauseUnimplemented();
      CheckFPUException();
      break;
    case FP_INFINITE:
    case FP_ZERO:
    case FP_NORMAL:
      break; // No-op, these are fine.
    default:
      Util::panic("Unknown floating point classification: {}", fp_class);
  }
}

FORCE_INLINE void SetCauseByArg(Registers& regs, f64& f) {
  auto fp_class = std::fpclassify(f.operator double());
  switch(fp_class) {
    case FP_NAN:
      if(f.is_quiet()) {
        regs.cop1.SetCauseInvalid();
        CheckFPUException();
      } else {
        regs.cop1.SetCauseUnimplemented();
        CheckFPUException();
      }
      break;
    case FP_SUBNORMAL:
      regs.cop1.SetCauseUnimplemented();
      CheckFPUException();
      break;
    case FP_INFINITE:
    case FP_ZERO:
    case FP_NORMAL:
      break; // No-op, these are fine.
    default:
      Util::panic("Unknown floating point classification: {}", fp_class);
  }
}

#define CheckArg(f) do { SetCauseByArg(regs, f); CheckFPUException(); } while(0)

FORCE_INLINE void SetCauseOnResult(Registers& regs, f64& d) {
  Cop1& cop1 = regs.cop1;
  auto fp_class = std::fpclassify(d.operator double());
  u64 c = 0x7FF7FFFFFFFFFFFF;
  double magic = U64_TO_D(c), min = std::numeric_limits<double>::min();
  switch (fp_class) {
    case FP_NAN:
      d = magic; // set result to sNAN
      break;
    case FP_SUBNORMAL:
      if (!cop1.fcr31.fs || cop1.fcr31.enable_underflow || cop1.fcr31.enable_inexact_operation) {
        regs.cop1.SetCauseUnimplemented();
        CheckFPUException();
      } else {
        // Since the if statement checks for the corresponding enable bits, it's safe to turn these cause bits on here.
        regs.cop1.SetCauseUnderflow();
        regs.cop1.SetCauseInexact();
        switch (cop1.fcr31.rounding_mode) {
          case 0:
          case 1:
            d = std::copysign(0, d.operator double());
            break;
          case 2:
            if (std::signbit(d.operator double())) {
              d = -(double)0;
            } else {
              d = min;
            }
            break;
          case 3:
            if (std::signbit(d.operator double())) {
              d = -min;
            } else {
              d = 0;
            }
            break;
        }
      }
      break;
    case FP_INFINITE:
    case FP_ZERO:
    case FP_NORMAL:
      break; // No-op, these are fine.
    default:
      Util::panic("Unknown FP classification: {}", fp_class);
  }
}

FORCE_INLINE void SetCauseOnResult(Registers& regs, f32& f) {
  Cop1& cop1 = regs.cop1;
  auto fp_class = std::fpclassify(f.operator float());
  u32 c = 0x7FBFFFFF;
  float magic = U32_TO_F(c), min = std::numeric_limits<float>::min();
  switch (fp_class) {
    case FP_NAN:
      f = magic; // set result to sNAN
      break;
    case FP_SUBNORMAL:
      if (!cop1.fcr31.fs || cop1.fcr31.enable_underflow || cop1.fcr31.enable_inexact_operation) {
        regs.cop1.SetCauseUnimplemented();
        CheckFPUException();
      } else {
        // Since the if statement checks for the corresponding enable bits, it's safe to turn these cause bits on here.
        regs.cop1.SetCauseUnderflow();
        regs.cop1.SetCauseInexact();
        switch (cop1.fcr31.rounding_mode) {
          case 0:
          case 1:
            f = std::copysign(0, f.operator float ());
            break;
          case 2:
            if (std::signbit(f.operator float ())) {
              f = -(float )0;
            } else {
              f = min;
            }
            break;
          case 3:
            if (std::signbit(f.operator float ())) {
              f = -min;
            } else {
              f = 0;
            }
            break;
        }
      }
      break;
    case FP_INFINITE:
    case FP_ZERO:
    case FP_NORMAL:
      break; // No-op, these are fine.
    default:
      Util::panic("Unknown FP classification: {}", fp_class);
  }
}

#define CheckResult(f) do { SetCauseOnResult(regs, (f)); CheckFPUException(); } while(0)

#define any_unordered(fs, ft) ((fs).is_nan() || (ft).is_nan())
#define CheckRound(a, b) do { if ((a) != (b)) { SetCauseInexact(); } CheckFPUException(); } while(0)

#define checknanregs(fs, ft) do { \
  if((fs).is_nan() || (ft).is_nan()) {                                       \
    regs.cop1.SetCauseInvalid();                                           \
    CheckFPUException();                                             \
  }                                                                  \
} while(0)

#define checkqnanregs(fs, ft) do { \
  if((fs).is_quiet() || (ft).is_quiet()) {                                     \
    regs.cop1.SetCauseInvalid();                                           \
    CheckFPUException();                                             \
  }                                                                  \
} while(0)

void Cop1::absd(Registers& regs, u32 instr) {
  OP(f64, std::fabs(fs.operator double()));
}

void Cop1::abss(Registers& regs, u32 instr) {
  OP(f32, std::fabs(fs.operator float()));
}

void Cop1::adds(Registers& regs, u32 instr) {
  OP(f32, fs + ft);
}

void Cop1::addd(Registers& regs, u32 instr) {
  OP(f64, fs + ft);
}

void Cop1::ceills(Registers& regs, u32 instr) {
  CheckFPUUsable();
  auto fs = FGR<f32>(regs.cop0.status, FS(instr));
  CheckLCVTArg(fs);
  s64 result;
  CVT_OP_CheckExcept({ result = std::ceil(fs.operator float()); });
  CheckRound(fs.to_bits(), result);
  FGR<s64>(regs.cop0.status, FD(instr)) = result;
}

void Cop1::ceilws(Registers& regs, u32 instr) {
  CheckFPUUsable();
  auto fs = FGR<f32>(regs.cop0.status, FS(instr));
  CheckWCVTArg(fs);
  s32 result;
  CVT_OP_CheckExcept({ result = std::ceil(fs.operator float()); });
  CheckRound(fs.to_bits(), result);
  FGR<s32>(regs.cop0.status, FD(instr)) = result;
}

void Cop1::ceilld(Registers& regs, u32 instr) {
  CheckFPUUsable();
  auto fs = FGR<f64>(regs.cop0.status, FS(instr));
  CheckLCVTArg(fs);
  s64 result;
  CVT_OP_CheckExcept({ result = std::ceil(fs.operator double()); });
  CheckRound(fs.to_bits(), result);
  FGR<s64>(regs.cop0.status, FD(instr)) = result;
}

void Cop1::ceilwd(Registers& regs, u32 instr) {
  CheckFPUUsable();
  auto fs = FGR<f64>(regs.cop0.status, FS(instr));
  CheckWCVTArg(fs);
  s32 result;
  CVT_OP_CheckExcept({ result = std::ceil(fs.operator double()); });
  CheckRound(fs.to_bits(), result);
  FGR<s32>(regs.cop0.status, FD(instr)) = result;
}

void Cop1::cfc1(Registers& regs, u32 instr) const {
  CheckFPUUsable_PreserveCause();
  u8 fd = RD(instr);
  s32 val = 0;
  switch(fd) {
    case 0: val = fcr0; break;
    case 31:
      val = fcr31.read();
      break;
    default: Util::panic("Undefined CFC1 with rd != 0 or 31");
  }
  regs.gpr[RT(instr)] = val;
}

void Cop1::ctc1(Registers& regs, u32 instr) {
  CheckFPUUsable_PreserveCause();
  u8 fs = RD(instr);
  u32 val = regs.gpr[RT(instr)];
  switch(fs) {
    case 0: break;
    case 31: {
      u32 prevRound = fcr31.rounding_mode;
      fcr31.write(val);
      if (prevRound != fcr31.rounding_mode) {
        switch (fcr31.rounding_mode) {
          case 0: fesetround(FE_TONEAREST); break;
          case 1: fesetround(FE_TOWARDZERO); break;
          case 2: fesetround(FE_UPWARD); break;
          case 3: fesetround(FE_DOWNWARD); break;
        }
      }
      CheckFPUException();
    } break;
    default: Util::panic("Undefined CTC1 with rd != 0 or 31");
  }
}

void Cop1::cvtds(Registers& regs, u32 instr) {
  CheckFPUUsable();
  auto fs = FGR<f32>(regs.cop0.status, FS(instr));
  CheckArg(fs);
  f64 result;
  OP_CheckExcept({ result = double(fs.operator float()); });
  CheckResult(result);
  FGR<f64>(regs.cop0.status, FD(instr)) = result;
}

void Cop1::cvtsd(Registers& regs, u32 instr) {
  CheckFPUUsable();
  auto fs = FGR<f64>(regs.cop0.status, FS(instr));
  CheckArg(fs);
  f32 result;
  OP_CheckExcept({ result = float(fs.operator double()); });
  CheckResult(result);
  FGR<f32>(regs.cop0.status, FD(instr)) = result;
}

void Cop1::cvtsw(Registers& regs, u32 instr) {
  CheckFPUUsable();
  auto fs = FGR<s32>(regs.cop0.status, FS(instr));
  f32 result;
  OP_CheckExcept({ result = float(fs); });
  CheckResult(result);
  FGR<f32>(regs.cop0.status, FD(instr)) = result;
}

void Cop1::cvtsl(Registers& regs, u32 instr) {
  CheckFPUUsable();
  auto fs = FGR<s64>(regs.cop0.status, FS(instr));
  if (fs >= s64(0x0080000000000000) || fs < s64(0xff80000000000000)) {
    SetCauseUnimplemented();
    CheckFPUException();
  }
  f32 result;
  OP_CheckExcept({ result = float(fs); });
  CheckResult(result);
  FGR<f32>(regs.cop0.status, FD(instr)) = result;
}

void Cop1::cvtwd(Registers& regs, u32 instr) {
  CheckFPUUsable();
  auto fs = FGR<f64>(regs.cop0.status, FS(instr));
  CheckWCVTArg(fs);
  s32 result;
  PUSHROUNDING;
  CVT_OP_CheckExcept({ result = std::rint(fs.operator double()); });
  POPROUNDING;
  CheckRound(fs.to_bits(), result);
  FGR<s32>(regs.cop0.status, FD(instr)) = result;
}

void Cop1::cvtws(Registers& regs, u32 instr) {
  CheckFPUUsable();
  auto fs = FGR<f32>(regs.cop0.status, FS(instr));
  CheckWCVTArg(fs);
  s32 result;
  PUSHROUNDING;
  CVT_OP_CheckExcept({ result = std::rint(fs.operator float()); });
  POPROUNDING;
  CheckRound(fs.to_bits(), result);
  FGR<s32>(regs.cop0.status, FD(instr)) = result;
}

void Cop1::cvtls(Registers& regs, u32 instr) {
  CheckFPUUsable();
  auto fs = FGR<f32>(regs.cop0.status, FS(instr));
  CheckLCVTArg(fs);
  s64 result;
  PUSHROUNDING;
  CVT_OP_CheckExcept({ result = std::rint(fs.operator float()); });
  POPROUNDING;
  CheckRound(fs.to_bits(), result);
  FGR<s64>(regs.cop0.status, FD(instr)) = result;
}

void Cop1::cvtdw(Registers& regs, u32 instr) {
  CheckFPUUsable();
  auto fs = FGR<s32>(regs.cop0.status, FS(instr));
  f64 result;
  OP_CheckExcept({ result = double(fs); });
  CheckResult(result);
  FGR<f64>(regs.cop0.status, FD(instr)) = result;
}

void Cop1::cvtdl(Registers& regs, u32 instr) {
  CheckFPUUsable();
  auto fs = FGR<s64>(regs.cop0.status, FS(instr));

  if (fs >= s64(0x0080000000000000) || fs < s64(0xff80000000000000)) {
    SetCauseUnimplemented();
    CheckFPUException();
  }
  f64 result;
  OP_CheckExcept({ result = double(fs); });
  CheckResult(result);
  FGR<f64>(regs.cop0.status, FD(instr)) = result;
}

void Cop1::cvtld(Registers& regs, u32 instr) {
  CheckFPUUsable();
  auto fs = FGR<f64>(regs.cop0.status, FS(instr));
  CheckLCVTArg(fs);
  s64 result;
  PUSHROUNDING;
  CVT_OP_CheckExcept({ result = std::rint(fs.operator double()); });
  POPROUNDING;
  CheckRound(fs.to_bits(), result);
  FGR<s64>(regs.cop0.status, FD(instr)) = result;
}

template <typename T>
void Cop1::cf(Registers& regs, u32 instr) {
  CheckFPUUsable();
  T fs = FGR<T>(regs.cop0.status, FS(instr));
  T ft = FGR<T>(regs.cop0.status, FT(instr));
  checkqnanregs(fs, ft);
  fcr31.compare = false;
}

template <typename T>
void Cop1::cun(Registers& regs, u32 instr) {
  CheckFPUUsable();
  T fs = FGR<T>(regs.cop0.status, FS(instr));
  T ft = FGR<T>(regs.cop0.status, FT(instr));
  checkqnanregs(fs, ft);
  fcr31.compare = any_unordered(fs, ft);
}

template <typename T>
void Cop1::ceq(Registers& regs, u32 instr) {
  CheckFPUUsable();
  T fs = FGR<T>(regs.cop0.status, FS(instr));
  T ft = FGR<T>(regs.cop0.status, FT(instr));
  checkqnanregs(fs, ft);
  fcr31.compare = fs == ft;
}

template <typename T>
void Cop1::cueq(Registers& regs, u32 instr) {
  CheckFPUUsable();
  T fs = FGR<T>(regs.cop0.status, FS(instr));
  T ft = FGR<T>(regs.cop0.status, FT(instr));
  checkqnanregs(fs, ft);
  fcr31.compare = fs == ft || any_unordered(fs, ft);
}

template <typename T>
void Cop1::colt(Registers& regs, u32 instr) {
  CheckFPUUsable();
  T fs = FGR<T>(regs.cop0.status, FS(instr));
  T ft = FGR<T>(regs.cop0.status, FT(instr));
  checkqnanregs(fs, ft);
  fcr31.compare = fs < ft;
}

template <typename T>
void Cop1::cult(Registers& regs, u32 instr) {
  CheckFPUUsable();
  T fs = FGR<T>(regs.cop0.status, FS(instr));
  T ft = FGR<T>(regs.cop0.status, FT(instr));
  checkqnanregs(fs, ft);
  fcr31.compare = fs < ft || any_unordered(fs, ft);
}

template <typename T>
void Cop1::cole(Registers& regs, u32 instr) {
  CheckFPUUsable();
  T fs = FGR<T>(regs.cop0.status, FS(instr));
  T ft = FGR<T>(regs.cop0.status, FT(instr));
  checkqnanregs(fs, ft);
  fcr31.compare = fs <= ft;
}

template <typename T>
void Cop1::cule(Registers& regs, u32 instr) {
  CheckFPUUsable();
  T fs = FGR<T>(regs.cop0.status, FS(instr));
  T ft = FGR<T>(regs.cop0.status, FT(instr));
  checkqnanregs(fs, ft);
  fcr31.compare = fs <= ft || any_unordered(fs, ft);
}

template <typename T>
void Cop1::csf(Registers& regs, u32 instr) {
  CheckFPUUsable();
  T fs = FGR<T>(regs.cop0.status, FS(instr));
  T ft = FGR<T>(regs.cop0.status, FT(instr));
  checknanregs(fs, ft);
  fcr31.compare = false;
}

template <typename T>
void Cop1::cngle(Registers& regs, u32 instr) {
  CheckFPUUsable();
  T fs = FGR<T>(regs.cop0.status, FS(instr));
  T ft = FGR<T>(regs.cop0.status, FT(instr));
  checknanregs(fs, ft);
  fcr31.compare = any_unordered(fs, ft);
}

template <typename T>
void Cop1::cseq(Registers& regs, u32 instr) {
  CheckFPUUsable();
  T fs = FGR<T>(regs.cop0.status, FS(instr));
  T ft = FGR<T>(regs.cop0.status, FT(instr));
  checknanregs(fs, ft);
  fcr31.compare = fs == ft;
}

template <typename T>
void Cop1::cngl(Registers& regs, u32 instr) {
  CheckFPUUsable();
  T fs = FGR<T>(regs.cop0.status, FS(instr));
  T ft = FGR<T>(regs.cop0.status, FT(instr));
  checknanregs(fs, ft);
  fcr31.compare = fs == ft || any_unordered(fs, ft);
}

template <typename T>
void Cop1::clt(Registers& regs, u32 instr) {
  CheckFPUUsable();
  T fs = FGR<T>(regs.cop0.status, FS(instr));
  T ft = FGR<T>(regs.cop0.status, FT(instr));
  checknanregs(fs, ft);
  fcr31.compare = fs < ft;
}

template <typename T>
void Cop1::cnge(Registers& regs, u32 instr) {
  CheckFPUUsable();
  T fs = FGR<T>(regs.cop0.status, FS(instr));
  T ft = FGR<T>(regs.cop0.status, FT(instr));
  checknanregs(fs, ft);
  fcr31.compare = fs < ft || any_unordered(fs, ft);
}

template <typename T>
void Cop1::cle(Registers& regs, u32 instr) {
  CheckFPUUsable();
  T fs = FGR<T>(regs.cop0.status, FS(instr));
  T ft = FGR<T>(regs.cop0.status, FT(instr));
  checknanregs(fs, ft);
  fcr31.compare = fs <= ft;
}

template <typename T>
void Cop1::cngt(Registers& regs, u32 instr) {
  CheckFPUUsable();
  T fs = FGR<T>(regs.cop0.status, FS(instr));
  T ft = FGR<T>(regs.cop0.status, FT(instr));
  checknanregs(fs, ft);
  fcr31.compare = fs <= ft || any_unordered(fs, ft);
}

template void Cop1::cf<f32>(Registers&, u32 instr);
template void Cop1::cun<f32>(Registers&, u32 instr);
template void Cop1::ceq<f32>(Registers&, u32 instr);
template void Cop1::cueq<f32>(Registers&, u32 instr);
template void Cop1::colt<f32>(Registers&, u32 instr);
template void Cop1::cult<f32>(Registers&, u32 instr);
template void Cop1::cole<f32>(Registers&, u32 instr);
template void Cop1::cule<f32>(Registers&, u32 instr);
template void Cop1::csf<f32>(Registers&, u32 instr);
template void Cop1::cngle<f32>(Registers&, u32 instr);
template void Cop1::cseq<f32>(Registers&, u32 instr);
template void Cop1::cngl<f32>(Registers&, u32 instr);
template void Cop1::clt<f32>(Registers&, u32 instr);
template void Cop1::cnge<f32>(Registers&, u32 instr);
template void Cop1::cle<f32>(Registers&, u32 instr);
template void Cop1::cngt<f32>(Registers&, u32 instr);
template void Cop1::cf<f64>(Registers&, u32 instr);
template void Cop1::cun<f64>(Registers&, u32 instr);
template void Cop1::ceq<f64>(Registers&, u32 instr);
template void Cop1::cueq<f64>(Registers&, u32 instr);
template void Cop1::colt<f64>(Registers&, u32 instr);
template void Cop1::cult<f64>(Registers&, u32 instr);
template void Cop1::cole<f64>(Registers&, u32 instr);
template void Cop1::cule<f64>(Registers&, u32 instr);
template void Cop1::csf<f64>(Registers&, u32 instr);
template void Cop1::cngle<f64>(Registers&, u32 instr);
template void Cop1::cseq<f64>(Registers&, u32 instr);
template void Cop1::cngl<f64>(Registers&, u32 instr);
template void Cop1::clt<f64>(Registers&, u32 instr);
template void Cop1::cnge<f64>(Registers&, u32 instr);
template void Cop1::cle<f64>(Registers&, u32 instr);
template void Cop1::cngt<f64>(Registers&, u32 instr);

void Cop1::divs(Registers &regs, u32 instr) {
  OP(f32, fs / ft);
}

void Cop1::divd(Registers &regs, u32 instr) {
  OP(f64, fs / ft);
}

void Cop1::muls(Registers &regs, u32 instr) {
  OP(f32, fs * ft);
}

void Cop1::muld(Registers& regs, u32 instr) {
  OP(f64, fs * ft);
}

void Cop1::subs(Registers &regs, u32 instr) {
  OP(f32, fs - ft);
}

void Cop1::subd(Registers &regs, u32 instr) {
  OP(f64, fs - ft);
}

void Cop1::movs(Registers& regs, u32 instr) {
  CheckFPUUsable_PreserveCause();
  auto val = FGR<u64>(regs.cop0.status, FS(instr));
  FGR<u64>(regs.cop0.status, FD(instr)) = val;
}

void Cop1::movd(Registers& regs, u32 instr) {
  CheckFPUUsable_PreserveCause();
  auto val = FGR<f64>(regs.cop0.status, FS(instr));
  FGR<f64>(regs.cop0.status, FD(instr)) = val;
}

void Cop1::negs(Registers &regs, u32 instr) {
  OP(f32, -fs.operator float());
}

void Cop1::negd(Registers &regs, u32 instr) {
  OP(f64, -fs.operator double());
}

void Cop1::sqrts(Registers &regs, u32 instr) {
  OP(f32, std::sqrt(fs.operator float()));
}

void Cop1::sqrtd(Registers &regs, u32 instr) {
  OP(f64, std::sqrt(fs.operator double()));
}

void Cop1::roundls(Registers& regs, u32 instr) {
  CheckFPUUsable();
  auto fs = FGR<f32>(regs.cop0.status, FS(instr));
  CheckLCVTArg(fs);
  s64 result;
  CVT_OP_CheckExcept({ result = std::nearbyint(fs.operator float()); });
  CheckRound(fs.to_bits(), result);
  FGR<s64>(regs.cop0.status, FD(instr)) = result;
}

void Cop1::roundld(Registers& regs, u32 instr) {
  CheckFPUUsable();
  auto fs = FGR<f64>(regs.cop0.status, FS(instr));
  CheckLCVTArg(fs);
  s64 result;
  CVT_OP_CheckExcept({ result = std::nearbyint(fs.operator double()); });
  CheckRound(fs.to_bits(), result);
  FGR<s64>(regs.cop0.status, FD(instr)) = result;
}

void Cop1::roundws(Registers& regs, u32 instr) {
  CheckFPUUsable();
  auto fs = FGR<f32>(regs.cop0.status, FS(instr));
  CheckWCVTArg(fs);
  s32 result;
  CVT_OP_CheckExcept({ result = std::nearbyint(fs.operator float()); });
  CheckRound(fs.to_bits(), result);
  FGR<s32>(regs.cop0.status, FD(instr)) = result;
}

void Cop1::roundwd(Registers& regs, u32 instr) {
  CheckFPUUsable();
  auto fs = FGR<f64>(regs.cop0.status, FS(instr));
  CheckWCVTArg(fs);
  s32 result;
  CVT_OP_CheckExcept({ result = std::nearbyint(fs.operator double()); });
  CheckRound(fs.to_bits(), result);
  FGR<s32>(regs.cop0.status, FD(instr)) = result;
}

void Cop1::floorls(Registers& regs, u32 instr) {
  CheckFPUUsable();
  auto fs = FGR<f32>(regs.cop0.status, FS(instr));
  CheckLCVTArg(fs);
  s64 result;
  CVT_OP_CheckExcept({ result = std::floor(fs.operator float()); });
  CheckRound(fs.to_bits(), result);
  FGR<s64>(regs.cop0.status, FD(instr)) = result;
}

void Cop1::floorld(Registers& regs, u32 instr) {
  CheckFPUUsable();
  auto fs = FGR<f64>(regs.cop0.status, FS(instr));
  CheckLCVTArg(fs);
  s64 result;
  CVT_OP_CheckExcept({ result = std::floor(fs.operator double()); });
  CheckRound(fs.to_bits(), result);
  FGR<s64>(regs.cop0.status, FD(instr)) = result;
}

void Cop1::floorws(Registers& regs, u32 instr) {
  CheckFPUUsable();
  auto fs = FGR<f32>(regs.cop0.status, FS(instr));
  CheckWCVTArg(fs);
  s32 result;
  CVT_OP_CheckExcept({ result = std::floor(fs.operator float()); });
  CheckRound(fs.to_bits(), result);
  FGR<s32>(regs.cop0.status, FD(instr)) = result;
}

void Cop1::floorwd(Registers& regs, u32 instr) {
  CheckFPUUsable();
  auto fs = FGR<f64>(regs.cop0.status, FS(instr));
  CheckWCVTArg(fs);
  s32 result;
  CVT_OP_CheckExcept({ result = std::floor(fs.operator double()); });
  CheckRound(fs.to_bits(), result);
  FGR<s32>(regs.cop0.status, FD(instr)) = result;
}

void Cop1::truncws(Registers& regs, u32 instr) {
  CheckFPUUsable();
  auto fs = FGR<f32>(regs.cop0.status, FS(instr));
  CheckWCVTArg(fs);
  s32 result;
  CVT_OP_CheckExcept({ result = std::trunc(fs.operator float()); });
  CheckRound(fs.to_bits(), result);
  FGR<s32>(regs.cop0.status, FD(instr)) = result;
}

void Cop1::truncwd(Registers& regs, u32 instr) {
  CheckFPUUsable();
  auto fs = FGR<f64>(regs.cop0.status, FS(instr));
  CheckWCVTArg(fs);
  s32 result;
  CVT_OP_CheckExcept({ result = std::trunc(fs.operator double()); });
  CheckRound(fs.to_bits(), result);
  FGR<s32>(regs.cop0.status, FD(instr)) = result;
}

void Cop1::truncls(Registers& regs, u32 instr) {
  CheckFPUUsable();
  auto fs = FGR<f32>(regs.cop0.status, FS(instr));
  CheckLCVTArg(fs);
  s64 result;
  CVT_OP_CheckExcept({ result = std::trunc(fs.operator float()); });
  CheckRound(fs.to_bits(), result);
  FGR<s64>(regs.cop0.status, FD(instr)) = result;
}

void Cop1::truncld(Registers& regs, u32 instr) {
  CheckFPUUsable();
  auto fs = FGR<f64>(regs.cop0.status, FS(instr));
  CheckLCVTArg(fs);
  s64 result;
  CVT_OP_CheckExcept({ result = std::trunc(fs.operator double()); });
  CheckRound(fs.to_bits(), result);
  FGR<s64>(regs.cop0.status, FD(instr)) = result;
}

template<class T>
void Cop1::lwc1(T &cpu, Mem &mem, u32 instr) {
  if constexpr(std::is_same_v<decltype(cpu), Interpreter&>) {
    Registers& regs = cpu.regs;
    CheckFPUUsable_PreserveCause();
    lwc1Interp(cpu.regs, mem, instr);
  } else if constexpr (std::is_same_v<decltype(cpu), JIT&>) {
    lwc1JIT(cpu, mem, instr);
  } else {
    Util::panic("What the fuck did you just give me?!!");
  }
}

template void Cop1::lwc1<Interpreter>(Interpreter&, Mem&, u32);
template void Cop1::lwc1<JIT>(JIT&, Mem&, u32);

template<class T>
void Cop1::swc1(T &cpu, Mem &mem, u32 instr) {
  if constexpr(std::is_same_v<decltype(cpu), Interpreter&>) {
    Registers& regs = cpu.regs;
    CheckFPUUsable_PreserveCause();
    swc1Interp(cpu.regs, mem, instr);
  } else if constexpr (std::is_same_v<decltype(cpu), JIT&>) {
    swc1JIT(cpu, mem, instr);
  } else {
    Util::panic("What the fuck did you just give me?!!");
  }
}

template void Cop1::swc1<Interpreter>(Interpreter&, Mem&, u32);
template void Cop1::swc1<JIT>(JIT&, Mem&, u32);

template<class T>
void Cop1::ldc1(T &cpu, Mem &mem, u32 instr) {
  if constexpr(std::is_same_v<decltype(cpu), Interpreter&>) {
    Registers& regs = cpu.regs;
    CheckFPUUsable_PreserveCause();
    ldc1Interp(cpu.regs, mem, instr);
  } else if constexpr (std::is_same_v<decltype(cpu), JIT&>) {
    ldc1JIT(cpu, mem, instr);
  } else {
    Util::panic("What the fuck did you just give me?!!");
  }
}

template void Cop1::ldc1<Interpreter>(Interpreter&, Mem&, u32);
template void Cop1::ldc1<JIT>(JIT&, Mem&, u32);

template<class T>
void Cop1::sdc1(T &cpu, Mem &mem, u32 instr) {
  if constexpr(std::is_same_v<decltype(cpu), Interpreter&>) {
    Registers& regs = cpu.regs;
    CheckFPUUsable_PreserveCause();
    sdc1Interp(cpu.regs, mem, instr);
  } else if constexpr (std::is_same_v<decltype(cpu), JIT&>) {
    sdc1JIT(cpu, mem, instr);
  } else {
    Util::panic("What the fuck did you just give me?!!");
  }
}

template void Cop1::sdc1<Interpreter>(Interpreter&, Mem&, u32);
template void Cop1::sdc1<JIT>(JIT&, Mem&, u32);

void Cop1::lwc1Interp(Registers& regs, Mem& mem, u32 instr) {
  u64 addr = (s64)(s16)instr + regs.gpr[BASE(instr)];

  u32 physical;
  if(!MapVAddr(regs, LOAD, addr, physical)) {
    HandleTLBException(regs, addr);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, LOAD), 0, regs.oldPC);
  } else {
    u32 data = mem.Read<u32>(regs, physical);
    FGR<u32>(regs.cop0.status, FT(instr)) = data;
  }
}

void Cop1::swc1Interp(Registers& regs, Mem& mem, u32 instr) {
  u64 addr = (s64)(s16)instr + regs.gpr[BASE(instr)];

  u32 physical;
  if(!MapVAddr(regs, STORE, addr, physical)) {
    HandleTLBException(regs, addr);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, STORE), 0, regs.oldPC);
  } else {
    mem.Write<u32>(regs, physical, FGR<u32>(regs.cop0.status, FT(instr)));
  }
}

void Cop1::unimplemented(Registers& regs) {
  CheckFPUUsable();
  fcr31.cause_unimplemented_operation = true;
  FireFPUException(regs);
}

void Cop1::ldc1Interp(Registers& regs, Mem& mem, u32 instr) {
  u64 addr = (s64)(s16)instr + regs.gpr[BASE(instr)];

  u32 physical;
  if(!MapVAddr(regs, LOAD, addr, physical)) {
    HandleTLBException(regs, addr);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, LOAD), 0, regs.oldPC);
  } else {
    u64 data = mem.Read<u64>(regs, physical);
    FGR<u64>(regs.cop0.status, FT(instr)) = data;
  }
}

void Cop1::sdc1Interp(Registers& regs, Mem& mem, u32 instr) {
  u64 addr = (s64)(s16)instr + regs.gpr[BASE(instr)];

  u32 physical;
  if(!MapVAddr(regs, STORE, addr, physical)) {
    HandleTLBException(regs, addr);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, STORE), 0, regs.oldPC);
  } else {
    mem.Write(regs, physical, FGR<u64>(regs.cop0.status, FT(instr)));
  }
}

void Cop1::mfc1(Registers& regs, u32 instr) {
  CheckFPUUsable_PreserveCause();
  regs.gpr[RT(instr)] = FGR<s32>(regs.cop0.status, FS(instr));
}

void Cop1::dmfc1(Registers& regs, u32 instr) {
  CheckFPUUsable_PreserveCause();
  regs.gpr[RT(instr)] = FGR<s64>(regs.cop0.status, FS(instr));
}

void Cop1::mtc1(Registers& regs, u32 instr) {
  CheckFPUUsable_PreserveCause();
  FGR<u32>(regs.cop0.status, FS(instr)) = regs.gpr[RT(instr)];
}

void Cop1::dmtc1(Registers& regs, u32 instr) {
  CheckFPUUsable_PreserveCause();
  FGR<u64>(regs.cop0.status, FS(instr)) = regs.gpr[RT(instr)];
}

}
