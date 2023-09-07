#include <core/registers/Cop1.hpp>
#include <core/registers/Registers.hpp>
#include <core/Interpreter.hpp>
#include <core/JIT.hpp>
#include <core/Mem.hpp>
#include <cmath>
#include <cfenv>

namespace n64 {
FORCE_INLINE bool FireFPUException(Registers& regs) {
  FCR31& fcr31 = regs.cop1.fcr31;
  if(fcr31.cause & ((1 << 5) | fcr31.enable)) {
    FireException(regs, ExceptionCode::FloatingPointError, 0, true);
    return true;
  }

  return false;
}

#define CheckFPUException() do { if(FireFPUException(regs)) { return; } } while(0)
#define CheckRound(a, b) do { if ((a) != (b)) { fcr31.cause_inexact_operation = true; if(!fcr31.enable_inexact_operation) { fcr31.flag_inexact_operation = true; } } CheckFPUException(); } while(0)
#define SetCauseUnimplemented() do { regs.cop1.fcr31.cause_unimplemented_operation = true; } while(0)
#define SetCauseUnderflow() do { \
  regs.cop1.fcr31.cause_underflow = true; \
  if(!regs.cop1.fcr31.enable_underflow) { \
    regs.cop1.fcr31.flag_underflow = true; \
  } \
} while(0)
#define SetCauseInexact() do { \
  regs.cop1.fcr31.cause_inexact_operation = true; \
  if(!regs.cop1.fcr31.enable_inexact_operation) { \
    regs.cop1.fcr31.flag_inexact_operation = true; \
  } \
} while(0)
#define SetCauseDivisionByZero() do { \
  regs.cop1.fcr31.cause_division_by_zero = true; \
  if(!regs.cop1.fcr31.enable_division_by_zero) { \
    regs.cop1.fcr31.flag_division_by_zero = true; \
  } \
} while(0)
#define SetCauseOverflow() do { \
  regs.cop1.fcr31.cause_overflow = true; \
  if(!regs.cop1.fcr31.enable_overflow) { \
    regs.cop1.fcr31.flag_overflow = true; \
  } \
} while(0)
#define SetCauseInvalid() do { \
  regs.cop1.fcr31.cause_invalid_operation = true; \
  if(!regs.cop1.fcr31.enable_invalid_operation) { \
    regs.cop1.fcr31.flag_invalid_operation = true; \
  } \
} while(0)

FORCE_INLINE int PushRoundingMode(const FCR31& fcr31) {
  int og = fegetround();
  switch(fcr31.rounding_mode) {
    case 0: fesetround(FE_TONEAREST); break;
    case 1: fesetround(FE_TOWARDZERO); break;
    case 2: fesetround(FE_UPWARD); break;
    case 3: fesetround(FE_DOWNWARD); break;
  }

  return og;
}

#define CheckCVTArg(f) do { SetCauseByArgCVT(regs, f); CheckFPUException(); } while(0)
#define CheckArg(f) do { SetCauseByArg(regs, f); CheckFPUException(); } while(0)
#define PUSHROUNDING int orig_round = PushRoundingMode(regs.cop1.fcr31)
#define POPROUNDING fesetround(orig_round)
#define OP_CheckExcept(op) do { PUSHROUNDING; feclearexcept(FE_ALL_EXCEPT); op; SetFPUCauseRaised(regs, fetestexcept(FE_ALL_EXCEPT)); POPROUNDING; } while(0)
#define CVT_OP_CheckExcept(op) do { feclearexcept(FE_ALL_EXCEPT); op; SetFPUCauseCVTRaised(regs, fetestexcept(FE_ALL_EXCEPT)); CheckFPUException(); } while(0)

#define OP(T, op) do { \
  CheckFPUUsable(); \
  auto fs = GetFGR_FS<T>(regs.cop0, FS(instr)); \
  auto ft = GetFGR_FT<T>(FT(instr));            \
  CheckArg(fs); \
  CheckArg(ft); \
  T result; \
  OP_CheckExcept({result = (op);});             \
  CheckResult(result);                     \
  SetFGR_Raw<T>(FD(instr), result); \
} while(0)

template <typename T>
FORCE_INLINE void SetCauseByArgCVT(Registers& regs, T f) {
  T min, max;
  if constexpr(std::is_same_v<T, float>) {
    min = -2147483648.0f;
    max = 2147483648.0f;
  } else if constexpr(std::is_same_v<T, double>) {
    min = -9007199254740992.000000;
    max = 9007199254740992.000000;
  }

  switch (std::fpclassify(f)) {
    case FP_NAN:
    case FP_INFINITE:
    case FP_SUBNORMAL:
      SetCauseUnimplemented();
      break;

    case FP_NORMAL:
      // Check overflow
      if (f >= max || f <= min) {
        SetCauseUnimplemented();
      }
      break;

    case FP_ZERO:
      break; // Fine
  }
}

FORCE_INLINE void SetFPUCauseRaised(Registers& regs, int raised) {
  if (raised == 0) {
    return;
  }

  if (raised & FE_UNDERFLOW) {
    if (!regs.cop1.fcr31.fs || regs.cop1.fcr31.enable_underflow || regs.cop1.fcr31.enable_inexact_operation) {
      SetCauseUnimplemented();
      return;
    } else {
      SetCauseUnderflow();
    }
  }

  if (raised & FE_INEXACT) {
    SetCauseInexact();
  }

  if (raised & FE_DIVBYZERO) {
    SetCauseDivisionByZero();
  }

  if (raised & FE_OVERFLOW) {
    SetCauseOverflow();
  }

  if (raised & FE_INVALID) {
    SetCauseInvalid();
  }
}

FORCE_INLINE void SetFPUCauseCVTRaised(Registers& regs, int raised) {
  if(raised & FE_INVALID) {
    SetCauseUnimplemented();
    return;
  }

  SetFPUCauseRaised(regs, raised);
}

template <typename T>
FORCE_INLINE void SetCauseByArg(Registers& regs, T f) {
  int c = std::fpclassify(f);
  switch(c) {
    case FP_NAN:
      if(isqnan(f)) {
        SetCauseInvalid();
      } else {
        SetCauseUnimplemented();
      }
      break;
    case FP_SUBNORMAL:
      SetCauseUnimplemented();
      break;
    case FP_INFINITE:
    case FP_ZERO:
    case FP_NORMAL:
      break; // No-op, these are fine.
    default:
      Util::panic("Unknown floating point classification: {}", c);
  }
}

#define F_TO_U32(f) (*((u32*)(&(f))))
#define D_TO_U64(d) (*((u64*)(&(d))))
#define U64_TO_D(d) (*((double*)(&(d))))
#define U32_TO_F(f) (*((float*)(&(f))))

template <typename T>
FORCE_INLINE void SetCauseOnResult(Registers& regs, T& d) {
  Cop1& cop1 = regs.cop1;
  int classification = std::fpclassify(d);
  T magic, min;
  if constexpr(std::is_same_v<T, float>) {
    u32 c = 0x7FBFFFFF;
    magic = U32_TO_F(c);
    min = FLT_MIN;
  } else if constexpr(std::is_same_v<T, double>) {
    u64 c = 0x7FF7FFFFFFFFFFFF;
    magic = U64_TO_D(c);
    min = DBL_MIN;
  }
  switch (classification) {
    case FP_NAN:
      d = magic; // set result to sNAN
      break;
    case FP_SUBNORMAL:
      if (!cop1.fcr31.fs || cop1.fcr31.enable_underflow || cop1.fcr31.enable_inexact_operation) {
        SetCauseUnimplemented();
      } else {
        // Since the if statement checks for the corresponding enable bits, it's safe to turn these cause bits on here.
        SetCauseUnderflow();
        SetCauseInexact();
        switch (cop1.fcr31.rounding_mode) {
          case 0:
          case 1:
            d = std::copysign(0, d);
            break;
          case 2:
            if (std::signbit(d)) {
              d = -(T)0;
            } else {
              d = min;
            }
            break;
          case 3:
            if (std::signbit(d)) {
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
      Util::panic("Unknown FP classification: {}", classification);
  }
}

#define CheckResult(f) do { SetCauseOnResult(regs, (f)); CheckFPUException(); } while(0)

#define any_unordered(fs, ft) (std::isnan(fs) || std::isnan(ft))

template <typename T>
FORCE_INLINE bool isnan(T f) {
  if constexpr(std::is_same_v<T, float>) {
    u32 v = F_TO_U32(f);
    return ((v & 0x7F800000) == 0x7F800000) && ((v & 0x7FFFFF) != 0);
  } else if constexpr(std::is_same_v<T, double>) {
    u64 v = D_TO_U64(f);
    return ((v & 0x7FF0000000000000) == 0x7FF0000000000000) && ((v & 0xFFFFFFFFFFFFF) != 0);
  } else {
    Util::panic("Invalid float type in isnan");
  }
}

template <typename T>
FORCE_INLINE bool isqnan(T f) {
  if constexpr(std::is_same_v<T, float>) {
    u32 v = F_TO_U32(f);
    return (v & 0x7FC00000) == 0x7FC00000;
  } else if constexpr(std::is_same_v<T, double>) {
    u64 v = D_TO_U64(f);
    return (v & 0x7FF8000000000000) == 0x7FF8000000000000;
  } else {
    Util::panic("Invalid float type in isqnan");
  }
}

#define checknanregs(fs, ft) do { \
  if(isnan(fs) || isnan(ft)) {                                       \
    regs.cop1.fcr31.cause_invalid_operation = true;                  \
    if(!regs.cop1.fcr31.enable_invalid_operation) {                  \
      regs.cop1.fcr31.flag_invalid_operation = true;                 \
    }                                                                \
    CheckFPUException();                                             \
  }                                                                  \
} while(0)

#define checkqnanregs(fs, ft) do { \
  if(isqnan(fs) || isqnan(ft)) {                                     \
    regs.cop1.fcr31.cause_invalid_operation = true;                  \
    if(!regs.cop1.fcr31.enable_invalid_operation) {                  \
      regs.cop1.fcr31.flag_invalid_operation = true;                 \
    }                                                                \
    CheckFPUException();                                             \
  }                                                                  \
} while(0)

void Cop1::absd(Registers& regs, u32 instr) {
  OP(double, std::abs(fs));
}

void Cop1::abss(Registers& regs, u32 instr) {
  OP(float, std::abs(fs));
}

void Cop1::adds(Registers& regs, u32 instr) {
  OP(float, fs + ft);
}

void Cop1::addd(Registers& regs, u32 instr) {
  OP(double, fs + ft);
}

void Cop1::ceills(Registers& regs, u32 instr) {
  CheckFPUUsable();
  auto fs = GetFGR_FS<float>(regs.cop0, FS(instr));
  CheckCVTArg(fs);
  s64 result;
  CVT_OP_CheckExcept({ result = std::ceil(fs); });
  CheckRound(fs, result);
  SetFGR(FD(instr), result);
}

void Cop1::ceilws(Registers& regs, u32 instr) {
  CheckFPUUsable();
  auto fs = GetFGR_FS<float>(regs.cop0, FS(instr));
  CheckCVTArg(fs);
  s32 result;
  CVT_OP_CheckExcept({ result = std::ceil(fs); });
  CheckRound(fs, result);
  SetFGR(FD(instr), result);
}

void Cop1::ceilld(Registers& regs, u32 instr) {
  CheckFPUUsable();
  auto fs = GetFGR_FS<double>(regs.cop0, FS(instr));
  CheckCVTArg(fs);
  s64 result;
  CVT_OP_CheckExcept({ result = std::ceil(fs); });
  CheckRound(fs, result);
  SetFGR(FD(instr), result);
}

void Cop1::ceilwd(Registers& regs, u32 instr) {
  CheckFPUUsable();
  auto fs = GetFGR_FS<double>(regs.cop0, FS(instr));
  CheckCVTArg(fs);
  s32 result;
  CVT_OP_CheckExcept({ result = std::ceil(fs); });
  CheckRound(fs, result);
  SetFGR(FD(instr), result);
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
      fcr31.write(val);
      FireFPUException(regs);
    } break;
    default: Util::panic("Undefined CTC1 with rd != 0 or 31");
  }
}

void Cop1::cvtds(Registers& regs, u32 instr) {
  CheckFPUUsable();
  auto fs = GetFGR_FS<float>(regs.cop0, FS(instr));
  CheckArg(fs);
  double result;
  OP_CheckExcept({ result = double(fs); });
  CheckResult(result);
  SetFGR_Raw(FD(instr), result);
}

void Cop1::cvtsd(Registers& regs, u32 instr) {
  CheckFPUUsable();
  auto fs = GetFGR_FS<double>(regs.cop0, FS(instr));
  CheckArg(fs);
  float result;
  OP_CheckExcept({ result = float(fs); });
  CheckResult(result);
  SetFGR_Raw(FD(instr), result);
}

void Cop1::cvtwd(Registers& regs, u32 instr) {
  CheckFPUUsable();
  auto fs = GetFGR_FS<double>(regs.cop0, FS(instr));
  CheckCVTArg(fs);
  s32 result;
  PUSHROUNDING;
  CVT_OP_CheckExcept({ result = s32(fs); });
  POPROUNDING;
  CheckRound(fs, result);
  SetFGR(FD(instr), result);
}

void Cop1::cvtws(Registers& regs, u32 instr) {
  CheckFPUUsable();
  auto fs = GetFGR_FS<float>(regs.cop0, FS(instr));
  CheckCVTArg(fs);
  s32 result;
  PUSHROUNDING;
  CVT_OP_CheckExcept({ result = s32(fs); });
  POPROUNDING;
  CheckRound(fs, result);
  SetFGR(FD(instr), result);
}

void Cop1::cvtls(Registers& regs, u32 instr) {
  CheckFPUUsable();
  auto fs = GetFGR_FS<float>(regs.cop0, FS(instr));
  CheckCVTArg(fs);
  s64 result;
  PUSHROUNDING;
  CVT_OP_CheckExcept({ result = std::rint(fs); });
  POPROUNDING;
  CheckRound(fs, result);
  SetFGR(FD(instr), result);
}

void Cop1::cvtsl(Registers& regs, u32 instr) {
  CheckFPUUsable();
  auto fs = GetFGR_FR<s64>(regs.cop0, FS(instr));
  if (fs >= s64(0x0080000000000000) || fs < s64(0xff80000000000000)) {
    SetCauseUnimplemented();
    CheckFPUException();
  }
  float result;
  OP_CheckExcept({ result = float(fs); });
  CheckResult(result);
  SetFGR_Raw(FD(instr), result);
}

void Cop1::cvtdw(Registers& regs, u32 instr) {
  CheckFPUUsable();
  auto fs = GetFGR_FS<s32>(regs.cop0, FS(instr));
  double result;
  OP_CheckExcept({ result = double(fs); });
  CheckResult(result);
  SetFGR_Raw(FD(instr), result);
}

void Cop1::cvtsw(Registers& regs, u32 instr) {
  CheckFPUUsable();
  auto fs = GetFGR_FS<s32>(regs.cop0, FS(instr));
  float result;
  OP_CheckExcept({ result = float(fs); });
  CheckResult(result);
  SetFGR_Raw(FD(instr), result);
}

void Cop1::cvtdl(Registers& regs, u32 instr) {
  CheckFPUUsable();
  auto fs = GetFGR_FR<s64>(regs.cop0, FS(instr));

  if (fs >= s64(0x0080000000000000) || fs < s64(0xff80000000000000)) {
    SetCauseUnimplemented();
    CheckFPUException();
  }
  double result;
  OP_CheckExcept({ result = double(fs); });
  CheckResult(result);
  SetFGR_Raw(FD(instr), result);
}

void Cop1::cvtld(Registers& regs, u32 instr) {
  CheckFPUUsable();
  auto fs = GetFGR_FS<double>(regs.cop0, FS(instr));
  CheckCVTArg(fs);
  s64 result;
  PUSHROUNDING;
  CVT_OP_CheckExcept({ result = std::rint(fs); });
  POPROUNDING;
  CheckRound(fs, result);
  SetFGR(FD(instr), result);
}

template <typename T>
inline bool CalculateCondition(T fs, T ft, CompConds cond) {
  switch(cond) {
    case F: case SF: return false;
    case UN: case NGLE: return any_unordered(fs, ft);
    case EQ: case SEQ: return fs == ft;
    case UEQ: case NGL: return fs == ft || any_unordered(fs, ft);
    case OLT: case LT: return fs < ft;
    case ULT: case NGE: return fs < ft || any_unordered(fs, ft);
    case OLE: case LE: return fs <= ft;
    case ULE: case NGT: return fs <= ft || any_unordered(fs, ft);
  }
}

template <typename T>
inline void CheckInvalidRegs(Registers& regs, T fs, T ft, CompConds cond) {
  switch(cond) {
    case F ... ULE: checkqnanregs(fs, ft); break;
    case SF ... NGT: checknanregs(fs, ft); break;
  }
}

template <typename T>
void Cop1::ccond(Registers& regs, u32 instr, CompConds cond) {
  CheckFPUUsable();
  T fs = GetFGR_FS<T>(regs.cop0, FS(instr));
  T ft = GetFGR_FT<T>(FT(instr));
  CheckInvalidRegs(regs, fs, ft, cond);
  fcr31.compare = CalculateCondition(fs, ft, cond);
}

template void Cop1::ccond<float>(Registers& regs, u32 instr, CompConds cond);
template void Cop1::ccond<double>(Registers& regs, u32 instr, CompConds cond);

void Cop1::divs(Registers &regs, u32 instr) {
  OP(float, fs / ft);
}

void Cop1::divd(Registers &regs, u32 instr) {
  OP(double, fs / ft);
}

void Cop1::muls(Registers &regs, u32 instr) {
  OP(float, fs * ft);
}

void Cop1::muld(Registers& regs, u32 instr) {
  OP(double, fs * ft);
}

void Cop1::subs(Registers &regs, u32 instr) {
  OP(float, fs - ft);
}

void Cop1::subd(Registers &regs, u32 instr) {
  OP(double, fs - ft);
}

void Cop1::movs(Registers& regs, u32 instr) {
  CheckFPUUsable_PreserveCause();
  auto val = GetFGR_FR<u64>(regs.cop0, FS(instr));
  SetFGR(FD(instr), val);
}

void Cop1::movd(Registers& regs, u32 instr) {
  CheckFPUUsable_PreserveCause();
  auto val = GetFGR_FS<double>(regs.cop0, FS(instr));
  SetFGR_Raw(FD(instr), val);
}

void Cop1::negs(Registers &regs, u32 instr) {
  OP(float, -fs);
}

void Cop1::negd(Registers &regs, u32 instr) {
  OP(double, -fs);
}

void Cop1::sqrts(Registers &regs, u32 instr) {
  OP(float, std::sqrt(fs));
}

void Cop1::sqrtd(Registers &regs, u32 instr) {
  OP(double, std::sqrt(fs));
}

void Cop1::roundls(Registers& regs, u32 instr) {
  CheckFPUUsable();
  auto fs = GetFGR_FS<float>(regs.cop0, FS(instr));
  CheckCVTArg(fs);
  s64 result;
  CVT_OP_CheckExcept({ result = std::nearbyint(fs); });
  CheckRound(fs, result);
  SetFGR(FD(instr), result);
}

void Cop1::roundld(Registers& regs, u32 instr) {
  CheckFPUUsable();
  auto fs = GetFGR_FS<double>(regs.cop0, FS(instr));
  CheckCVTArg(fs);
  s64 result;
  CVT_OP_CheckExcept({ result = std::nearbyint(fs); });
  CheckRound(fs, result);
  SetFGR(FD(instr), result);
}

void Cop1::roundws(Registers& regs, u32 instr) {
  CheckFPUUsable();
  auto fs = GetFGR_FS<float>(regs.cop0, FS(instr));
  CheckCVTArg(fs);
  s32 result;
  CVT_OP_CheckExcept({ result = std::nearbyint(fs); });
  CheckRound(fs, result);
  SetFGR(FD(instr), result);
}

void Cop1::roundwd(Registers& regs, u32 instr) {
  CheckFPUUsable();
  auto fs = GetFGR_FS<double>(regs.cop0, FS(instr));
  CheckCVTArg(fs);
  s32 result;
  CVT_OP_CheckExcept({ result = std::nearbyint(fs); });
  CheckRound(fs, result);
  SetFGR(FD(instr), result);
}

void Cop1::floorls(Registers& regs, u32 instr) {
  CheckFPUUsable();
  auto fs = GetFGR_FS<float>(regs.cop0, FS(instr));
  CheckCVTArg(fs);
  s64 result;
  CVT_OP_CheckExcept({ result = std::floor(fs); });
  CheckRound(fs, result);
  SetFGR(FD(instr), result);
}

void Cop1::floorld(Registers& regs, u32 instr) {
  CheckFPUUsable();
  auto fs = GetFGR_FS<double>(regs.cop0, FS(instr));
  CheckCVTArg(fs);
  s64 result;
  CVT_OP_CheckExcept({ result = std::floor(fs); });
  CheckRound(fs, result);
  SetFGR(FD(instr), result);
}

void Cop1::floorws(Registers& regs, u32 instr) {
  CheckFPUUsable();
  auto fs = GetFGR_FS<float>(regs.cop0, FS(instr));
  CheckCVTArg(fs);
  s32 result;
  CVT_OP_CheckExcept({ result = std::floor(fs); });
  CheckRound(fs, result);
  SetFGR(FD(instr), result);
}

void Cop1::floorwd(Registers& regs, u32 instr) {
  CheckFPUUsable();
  auto fs = GetFGR_FS<double>(regs.cop0, FS(instr));
  CheckCVTArg(fs);
  s32 result;
  CVT_OP_CheckExcept({ result = std::floor(fs); });
  CheckRound(fs, result);
  SetFGR(FD(instr), result);
}

void Cop1::truncws(Registers& regs, u32 instr) {
  CheckFPUUsable();
  auto fs = GetFGR_FS<float>(regs.cop0, FS(instr));
  CheckCVTArg(fs);
  s32 result;
  CVT_OP_CheckExcept({ result = std::trunc(fs); });
  CheckRound(fs, result);
  SetFGR(FD(instr), result);
}

void Cop1::truncwd(Registers& regs, u32 instr) {
  CheckFPUUsable();
  auto fs = GetFGR_FS<double>(regs.cop0, FS(instr));
  CheckCVTArg(fs);
  s32 result;
  CVT_OP_CheckExcept({ result = std::trunc(fs); });
  CheckRound(fs, result);
  SetFGR(FD(instr), result);
}

void Cop1::truncls(Registers& regs, u32 instr) {
  CheckFPUUsable();
  auto fs = GetFGR_FS<float>(regs.cop0, FS(instr));
  CheckCVTArg(fs);
  s64 result;
  CVT_OP_CheckExcept({ result = std::trunc(fs); });
  CheckRound(fs, result);
  SetFGR(FD(instr), result);
}

void Cop1::truncld(Registers& regs, u32 instr) {
  CheckFPUUsable();
  auto fs = GetFGR_FS<double>(regs.cop0, FS(instr));
  CheckCVTArg(fs);
  s64 result;
  CVT_OP_CheckExcept({ result = std::trunc(fs); });
  CheckRound(fs, result);
  SetFGR(FD(instr), result);
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
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, LOAD), 0, true);
  } else {
    u32 data = mem.Read32(regs, physical);
    SetFGR_FR<u32>(regs.cop0, FT(instr), data);
  }
}

void Cop1::swc1Interp(Registers& regs, Mem& mem, u32 instr) {
  u64 addr = (s64)(s16)instr + regs.gpr[BASE(instr)];

  u32 physical;
  if(!MapVAddr(regs, STORE, addr, physical)) {
    HandleTLBException(regs, addr);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, STORE), 0, true);
  } else {
    mem.Write32(regs, physical, GetFGR_FR<u32>(regs.cop0, FT(instr)));
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
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, LOAD), 0, true);
  } else {
    u64 data = mem.Read64(regs, physical);
    SetFGR_FR<u64>(regs.cop0, FT(instr), data);
  }
}

void Cop1::sdc1Interp(Registers& regs, Mem& mem, u32 instr) {
  u64 addr = (s64)(s16)instr + regs.gpr[BASE(instr)];

  u32 physical;
  if(!MapVAddr(regs, STORE, addr, physical)) {
    HandleTLBException(regs, addr);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, STORE), 0, true);
  } else {
    mem.Write64(regs, physical, GetFGR_FR<u64>(regs.cop0, FT(instr)));
  }
}

void Cop1::mfc1(Registers& regs, u32 instr) {
  CheckFPUUsable_PreserveCause();
  regs.gpr[RT(instr)] = (s32)GetFGR_FR<u32>(regs.cop0, FS(instr));
}

void Cop1::dmfc1(Registers& regs, u32 instr) {
  CheckFPUUsable_PreserveCause();
  regs.gpr[RT(instr)] = (s64)GetFGR_FR<u64>(regs.cop0, FS(instr));
}

void Cop1::mtc1(Registers& regs, u32 instr) {
  CheckFPUUsable_PreserveCause();
  SetFGR_FR<u32>(regs.cop0, FS(instr), regs.gpr[RT(instr)]);
}

void Cop1::dmtc1(Registers& regs, u32 instr) {
  CheckFPUUsable_PreserveCause();
  SetFGR_FR<u64>(regs.cop0, FS(instr), regs.gpr[RT(instr)]);
}

}
