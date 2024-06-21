#include <core/registers/Cop1.hpp>
#include <core/registers/Registers.hpp>
#include <core/Interpreter.hpp>
#include <core/Mem.hpp>
#include <cmath>
#include <cfenv>

namespace n64 {
template<> auto Cop1::FGR_T<s32>(Cop0Status& status, u32 index) -> s32& {
  if (status.fr) {
    return fgr[index].int32;
  } else if (index & 1) {
    return fgr[index & ~1].int32h;
  } else {
    return fgr[index & ~1].int32;
  }
}

template<> auto Cop1::FGR_T<u32>(Cop0Status& status, u32 index) -> u32& {
  return (u32&)FGR_T<s32>(status, index);
}

template<> auto Cop1::FGR_T<float>(Cop0Status& status, u32 index) -> float& {
  return fgr[index].float32;
}

template<> auto Cop1::FGR_T<s64>(Cop0Status& status, u32 index) -> s64& {
  if (status.fr) {
    return fgr[index].int64;
  } else {
    return fgr[index & ~1].int64;
  }
}

template<> auto Cop1::FGR_T<u64>(Cop0Status& status, u32 index) -> u64& {
  return (u64&)FGR_T<s64>(status, index);
}

template<> auto Cop1::FGR_T<double>(Cop0Status& status, u32 index) -> double& {
  return fgr[index].float64;
}

template<> auto Cop1::FGR_S<s32>(Cop0Status& status, u32 index) -> s32& {
  if (status.fr) {
    return fgr[index].int32;
  } else {
    return fgr[index & ~1].int32;
  }
}

template<> auto Cop1::FGR_S<u32>(Cop0Status& status, u32 index) -> u32& {
  return (u32&)FGR_S<s32>(status, index);
}

template<> auto Cop1::FGR_S<float>(Cop0Status& status, u32 index) -> float& {
  if (status.fr) {
    return fgr[index].float32;
  } else {
    return fgr[index & ~1].float32;
  }
}

template<> auto Cop1::FGR_S<s64>(Cop0Status& status, u32 index) -> s64& {
  return FGR_T<s64>(status, index);
}

template<> auto Cop1::FGR_S<u64>(Cop0Status& status, u32 index) -> u64& {
  return (u64&)FGR_S<s64>(status, index);
}

template<> auto Cop1::FGR_S<double>(Cop0Status& status, u32 index) -> double& {
  if (status.fr) {
    return fgr[index].float64;
  } else {
    return fgr[index & ~1].float64;
  }
}

template<> auto Cop1::FGR_D<s32>(Cop0Status& status, u32 index) -> s32& {
  fgr[index].int32h = 0;
  return fgr[index].int32;
}

template<> auto Cop1::FGR_D<u32>(Cop0Status& status, u32 index) -> u32& {
  return (u32&)FGR_D<s32>(status, index);
}

template<> auto Cop1::FGR_D<float>(Cop0Status& status, u32 index) -> float& {
  fgr[index].float32h = 0;
  return fgr[index].float32;
}

template<> auto Cop1::FGR_D<s64>(Cop0Status& status, u32 index) -> s64& {
  return fgr[index].int64;
}

template<> auto Cop1::FGR_D<u64>(Cop0Status& status, u32 index) -> u64& {
  return (u64&)FGR_D<s64>(status, index);
}

template<> auto Cop1::FGR_D<double>(Cop0Status& status, u32 index) -> double& {
  return FGR_T<double>(status, index);
}

bool Cop1::FireException() {
  u32 enable = fcr31.enable | (1 << 5);
  if(fcr31.cause & enable) {
    regs.cop0.FireException(ExceptionCode::FloatingPointError, 0, regs.oldPC);
    return true;
  }

  return false;
}

#define CheckFPUException() do { if(FireException()) { return; } } while(0)

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
#define OP_CheckExcept(op) do { feclearexcept(FE_ALL_EXCEPT); PUSHROUNDING; op; SetCauseRaised(fetestexcept(FE_ALL_EXCEPT)); POPROUNDING; } while(0)
#define CVT_OP_CheckExcept(op) do { feclearexcept(FE_ALL_EXCEPT); op; SetCauseRaisedCVT(fetestexcept(FE_ALL_EXCEPT)); CheckFPUException(); } while(0)

#define OP(T, op) do { \
  CheckFPUUsable(); \
  auto fs = FGR_S<T>(regs.cop0.status, FS(instr)); \
  auto ft = FGR_T<T>(regs.cop0.status, FT(instr)); \
  CheckArg(fs); \
  CheckArg(ft); \
  T result; \
  OP_CheckExcept({result = (op);});             \
  CheckResult(result);                     \
  FGR_D<T>(regs.cop0.status, FD(instr)) = result; \
} while(0)

template <typename T>
void Cop1::SetCauseByArgWCVT(T f) {
  switch (std::fpclassify(f)) {
    case FP_NAN:
    case FP_INFINITE:
    case FP_SUBNORMAL:
      SetCauseUnimplemented();
      //CheckFPUException();
      break;

    case FP_NORMAL:
      // Check overflow
      if (f >= 2147483648.0f || f < -2147483648.0f) {
        SetCauseUnimplemented();
        //CheckFPUException();
      }
      break;

    case FP_ZERO:
      break; // Fine
  }
}

template <typename T>
void Cop1::SetCauseByArgLCVT(T f) {
  switch (std::fpclassify(f)) {
    case FP_NAN:
    case FP_INFINITE:
    case FP_SUBNORMAL:
      SetCauseUnimplemented();
      //CheckFPUException();
      break;

    case FP_NORMAL:
      // Check overflow
      if (f >= 9007199254740992.000000 || f <= -9007199254740992.000000) {
        SetCauseUnimplemented();
        //CheckFPUException();
      }
      break;

    case FP_ZERO:
      break; // Fine
  }
}

#define CheckWCVTArg(f) do { SetCauseByArgWCVT(f); CheckFPUException(); } while(0)
#define CheckLCVTArg(f) do { SetCauseByArgLCVT(f); CheckFPUException(); } while(0)

void Cop1::SetCauseRaised(int raised) {
  if (raised == 0) {
    return;
  }

  if (raised & FE_UNDERFLOW) {
    if (!fcr31.fs || fcr31.enable_underflow || fcr31.enable_inexact_operation) {
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

void Cop1::SetCauseRaisedCVT(int raised) {
  if(raised & FE_INVALID) {
    SetCauseUnimplemented();
    return;
  }

  SetCauseRaised(raised);
}

#define F_TO_U32(f) (*((u32*)(&(f))))
#define D_TO_U64(d) (*((u64*)(&(d))))
#define U64_TO_D(d) (*((double*)(&(d))))
#define U32_TO_F(f) (*((float*)(&(f))))

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

template <typename T>
void Cop1::SetCauseByArg(T f) {
  auto fp_class = std::fpclassify(f);
  switch(fp_class) {
    case FP_NAN:
      if(isqnan(f)) {
        SetCauseInvalid();
        //CheckFPUException();
      } else {
        SetCauseUnimplemented();
        //CheckFPUException();
      }
      break;
    case FP_SUBNORMAL:
      SetCauseUnimplemented();
      //CheckFPUException();
      break;
    case FP_INFINITE:
    case FP_ZERO:
    case FP_NORMAL:
      break; // No-op, these are fine.
    default:
      Util::panic("Unknown floating point classification: {}", fp_class);
  }
}

#define CheckArg(f) do { SetCauseByArg(f); CheckFPUException(); } while(0)

template <typename T>
void Cop1::SetCauseOnResult(T& d) {
  auto fp_class = std::fpclassify(d);
  T magic, min;
  if constexpr(std::is_same_v<T, float>) {
    u32 c = 0x7FBFFFFF;
    magic = U32_TO_F(c);
    min = std::numeric_limits<float>::min();
  } else if constexpr(std::is_same_v<T, double>) {
    u64 c = 0x7FF7FFFFFFFFFFFF;
    magic = U64_TO_D(c);
    min = std::numeric_limits<double>::min();
  }
  switch (fp_class) {
    case FP_NAN:
      d = magic; // set result to sNAN
      break;
    case FP_SUBNORMAL:
      if (!fcr31.fs || fcr31.enable_underflow || fcr31.enable_inexact_operation) {
        SetCauseUnimplemented();
        //CheckFPUException();
      } else {
        // Since the if statement checks for the corresponding enable bits, it's safe to turn these cause bits on here.
        SetCauseUnderflow();
        SetCauseInexact();
        switch (fcr31.rounding_mode) {
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
      Util::panic("Unknown FP classification: {}", fp_class);
  }
}

#define CheckResult(f) do { SetCauseOnResult((f)); CheckFPUException(); } while(0)

#define any_unordered(fs, ft) (std::isnan(fs) || std::isnan(ft))
#define CheckRound(a, b) do { if ((a) != (b)) { SetCauseInexact(); } CheckFPUException(); } while(0)

template <typename T>
FORCE_INLINE bool is_nan(T f) {
  if constexpr(std::is_same_v<T, float>) {
    u32 v = F_TO_U32(f);
    return ((v & 0x7F800000) == 0x7F800000) && ((v & 0x7FFFFF) != 0);
  } else if constexpr(std::is_same_v<T, double>) {
    u64 v = D_TO_U64(f);
    return ((v & 0x7FF0000000000000) == 0x7FF0000000000000) && ((v & 0xFFFFFFFFFFFFF) != 0);
  } else {
    Util::panic("Invalid float type in is_nan");
  }
}

#define checknanregs(fs, ft) do { \
  if(is_nan(fs) || is_nan(ft)) {                                       \
    SetCauseInvalid();                                           \
    CheckFPUException();                                             \
  }                                                                  \
} while(0)

#define checkqnanregs(fs, ft) do { \
  if(isqnan(fs) || isqnan(ft)) {                                     \
    SetCauseInvalid();                                           \
    CheckFPUException();                                             \
  }                                                                  \
} while(0)

void Cop1::absd(u32 instr) {
  OP(double, std::fabs(fs));
}

void Cop1::abss(u32 instr) {
  OP(float, std::fabs(fs));
}

void Cop1::adds(u32 instr) {
  OP(float, fs + ft);
}

void Cop1::addd(u32 instr) {
  OP(double, fs + ft);
}

void Cop1::ceills(u32 instr) {
  CheckFPUUsable();
  auto fs = FGR_S<float>(regs.cop0.status, FS(instr));
  CheckLCVTArg(fs);
  s64 result;
  CVT_OP_CheckExcept({ result = std::ceil(fs); });
  CheckRound(fs, result);
  FGR_D<s64>(regs.cop0.status, FD(instr)) = result;
}

void Cop1::ceilws(u32 instr) {
  CheckFPUUsable();
  auto fs = FGR_S<float>(regs.cop0.status, FS(instr));
  CheckWCVTArg(fs);
  s32 result;
  CVT_OP_CheckExcept({ result = std::ceil(fs); });
  CheckRound(fs, result);
  FGR_D<s32>(regs.cop0.status, FD(instr)) = result;
}

void Cop1::ceilld(u32 instr) {
  CheckFPUUsable();
  auto fs = FGR_S<double>(regs.cop0.status, FS(instr));
  CheckLCVTArg(fs);
  s64 result;
  CVT_OP_CheckExcept({ result = std::ceil(fs); });
  CheckRound(fs, result);
  FGR_D<s64>(regs.cop0.status, FD(instr)) = result;
}

void Cop1::ceilwd(u32 instr) {
  CheckFPUUsable();
  auto fs = FGR_S<double>(regs.cop0.status, FS(instr));
  CheckWCVTArg(fs);
  s32 result;
  CVT_OP_CheckExcept({ result = std::ceil(fs); });
  CheckRound(fs, result);
  FGR_D<s32>(regs.cop0.status, FD(instr)) = result;
}

void Cop1::cfc1(u32 instr) const {
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

void Cop1::ctc1(u32 instr) {
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

void Cop1::cvtds(u32 instr) {
  CheckFPUUsable();
  auto fs = FGR_S<float>(regs.cop0.status, FS(instr));
  CheckArg(fs);
  double result;
  OP_CheckExcept({ result = double(fs); });
  CheckResult(result);
  FGR_D<double>(regs.cop0.status, FD(instr)) = result;
}

void Cop1::cvtsd(u32 instr) {
  CheckFPUUsable();
  auto fs = FGR_S<double>(regs.cop0.status, FS(instr));
  CheckArg(fs);
  float result;
  OP_CheckExcept({ result = float(fs); });
  CheckResult(result);
  FGR_D<float>(regs.cop0.status, FD(instr)) = result;
}

void Cop1::cvtsw(u32 instr) {
  CheckFPUUsable();
  auto fs = FGR_S<s32>(regs.cop0.status, FS(instr));
  float result;
  OP_CheckExcept({ result = float(fs); });
  CheckResult(result);
  FGR_D<float>(regs.cop0.status, FD(instr)) = result;
}

void Cop1::cvtsl(u32 instr) {
  CheckFPUUsable();
  auto fs = FGR_S<s64>(regs.cop0.status, FS(instr));
  if (fs >= s64(0x0080000000000000) || fs < s64(0xff80000000000000)) {
    SetCauseUnimplemented();
    CheckFPUException();
  }
  float result;
  OP_CheckExcept({ result = float(fs); });
  CheckResult(result);
  FGR_D<float>(regs.cop0.status, FD(instr)) = result;
}

void Cop1::cvtwd(u32 instr) {
  CheckFPUUsable();
  auto fs = FGR_S<double>(regs.cop0.status, FS(instr));
  CheckWCVTArg(fs);
  s32 result;
  PUSHROUNDING;
  CVT_OP_CheckExcept({ result = std::rint(fs); });
  POPROUNDING;
  CheckRound(fs, result);
  FGR_D<s32>(regs.cop0.status, FD(instr)) = result;
}

void Cop1::cvtws(u32 instr) {
  CheckFPUUsable();
  auto fs = FGR_S<float>(regs.cop0.status, FS(instr));
  CheckWCVTArg(fs);
  s32 result;
  PUSHROUNDING;
  CVT_OP_CheckExcept({ result = std::rint(fs); });
  POPROUNDING;
  CheckRound(fs, result);
  FGR_D<s32>(regs.cop0.status, FD(instr)) = result;
}

void Cop1::cvtls(u32 instr) {
  CheckFPUUsable();
  auto fs = FGR_S<float>(regs.cop0.status, FS(instr));
  CheckLCVTArg(fs);
  s64 result;
  PUSHROUNDING;
  CVT_OP_CheckExcept({ result = std::rint(fs); });
  POPROUNDING;
  CheckRound(fs, result);
  FGR_D<s64>(regs.cop0.status, FD(instr)) = result;
}

void Cop1::cvtdw(u32 instr) {
  CheckFPUUsable();
  auto fs = FGR_S<s32>(regs.cop0.status, FS(instr));
  double result;
  OP_CheckExcept({ result = double(fs); });
  CheckResult(result);
  FGR_D<double>(regs.cop0.status, FD(instr)) = result;
}

void Cop1::cvtdl(u32 instr) {
  CheckFPUUsable();
  auto fs = FGR_S<s64>(regs.cop0.status, FS(instr));

  if (fs >= s64(0x0080000000000000) || fs < s64(0xff80000000000000)) {
    SetCauseUnimplemented();
    CheckFPUException();
  }
  double result;
  OP_CheckExcept({ result = double(fs); });
  CheckResult(result);
  FGR_D<double>(regs.cop0.status, FD(instr)) = result;
}

void Cop1::cvtld(u32 instr) {
  CheckFPUUsable();
  auto fs = FGR_S<double>(regs.cop0.status, FS(instr));
  CheckLCVTArg(fs);
  s64 result;
  PUSHROUNDING;
  CVT_OP_CheckExcept({ result = std::rint(fs); });
  POPROUNDING;
  CheckRound(fs, result);
  FGR_D<s64>(regs.cop0.status, FD(instr)) = result;
}

template <typename T>
void Cop1::cf(u32 instr) {
  CheckFPUUsable();
  T fs = FGR_S<T>(regs.cop0.status, FS(instr));
  T ft = FGR_T<T>(regs.cop0.status, FT(instr));
  checkqnanregs(fs, ft);
  fcr31.compare = false;
}

template <typename T>
void Cop1::cun(u32 instr) {
  CheckFPUUsable();
  T fs = FGR_S<T>(regs.cop0.status, FS(instr));
  T ft = FGR_T<T>(regs.cop0.status, FT(instr));
  checkqnanregs(fs, ft);
  fcr31.compare = any_unordered(fs, ft);
}

template <typename T>
void Cop1::ceq(u32 instr) {
  CheckFPUUsable();
  T fs = FGR_S<T>(regs.cop0.status, FS(instr));
  T ft = FGR_T<T>(regs.cop0.status, FT(instr));
  checkqnanregs(fs, ft);
  fcr31.compare = fs == ft;
}

template <typename T>
void Cop1::cueq(u32 instr) {
  CheckFPUUsable();
  T fs = FGR_S<T>(regs.cop0.status, FS(instr));
  T ft = FGR_T<T>(regs.cop0.status, FT(instr));
  checkqnanregs(fs, ft);
  fcr31.compare = fs == ft || any_unordered(fs, ft);
}

template <typename T>
void Cop1::colt(u32 instr) {
  CheckFPUUsable();
  T fs = FGR_S<T>(regs.cop0.status, FS(instr));
  T ft = FGR_T<T>(regs.cop0.status, FT(instr));
  checkqnanregs(fs, ft);
  fcr31.compare = fs < ft;
}

template <typename T>
void Cop1::cult(u32 instr) {
  CheckFPUUsable();
  T fs = FGR_S<T>(regs.cop0.status, FS(instr));
  T ft = FGR_T<T>(regs.cop0.status, FT(instr));
  checkqnanregs(fs, ft);
  fcr31.compare = fs < ft || any_unordered(fs, ft);
}

template <typename T>
void Cop1::cole(u32 instr) {
  CheckFPUUsable();
  T fs = FGR_S<T>(regs.cop0.status, FS(instr));
  T ft = FGR_T<T>(regs.cop0.status, FT(instr));
  checkqnanregs(fs, ft);
  fcr31.compare = fs <= ft;
}

template <typename T>
void Cop1::cule(u32 instr) {
  CheckFPUUsable();
  T fs = FGR_S<T>(regs.cop0.status, FS(instr));
  T ft = FGR_T<T>(regs.cop0.status, FT(instr));
  checkqnanregs(fs, ft);
  fcr31.compare = fs <= ft || any_unordered(fs, ft);
}

template <typename T>
void Cop1::csf(u32 instr) {
  CheckFPUUsable();
  T fs = FGR_S<T>(regs.cop0.status, FS(instr));
  T ft = FGR_T<T>(regs.cop0.status, FT(instr));
  checknanregs(fs, ft);
  fcr31.compare = false;
}

template <typename T>
void Cop1::cngle(u32 instr) {
  CheckFPUUsable();
  T fs = FGR_S<T>(regs.cop0.status, FS(instr));
  T ft = FGR_T<T>(regs.cop0.status, FT(instr));
  checknanregs(fs, ft);
  fcr31.compare = any_unordered(fs, ft);
}

template <typename T>
void Cop1::cseq(u32 instr) {
  CheckFPUUsable();
  T fs = FGR_S<T>(regs.cop0.status, FS(instr));
  T ft = FGR_T<T>(regs.cop0.status, FT(instr));
  checknanregs(fs, ft);
  fcr31.compare = fs == ft;
}

template <typename T>
void Cop1::cngl(u32 instr) {
  CheckFPUUsable();
  T fs = FGR_S<T>(regs.cop0.status, FS(instr));
  T ft = FGR_T<T>(regs.cop0.status, FT(instr));
  checknanregs(fs, ft);
  fcr31.compare = fs == ft || any_unordered(fs, ft);
}

template <typename T>
void Cop1::clt(u32 instr) {
  CheckFPUUsable();
  T fs = FGR_S<T>(regs.cop0.status, FS(instr));
  T ft = FGR_T<T>(regs.cop0.status, FT(instr));
  checknanregs(fs, ft);
  fcr31.compare = fs < ft;
}

template <typename T>
void Cop1::cnge(u32 instr) {
  CheckFPUUsable();
  T fs = FGR_S<T>(regs.cop0.status, FS(instr));
  T ft = FGR_T<T>(regs.cop0.status, FT(instr));
  checknanregs(fs, ft);
  fcr31.compare = fs < ft || any_unordered(fs, ft);
}

template <typename T>
void Cop1::cle(u32 instr) {
  CheckFPUUsable();
  T fs = FGR_S<T>(regs.cop0.status, FS(instr));
  T ft = FGR_T<T>(regs.cop0.status, FT(instr));
  checknanregs(fs, ft);
  fcr31.compare = fs <= ft;
}

template <typename T>
void Cop1::cngt(u32 instr) {
  CheckFPUUsable();
  T fs = FGR_S<T>(regs.cop0.status, FS(instr));
  T ft = FGR_T<T>(regs.cop0.status, FT(instr));
  checknanregs(fs, ft);
  fcr31.compare = fs <= ft || any_unordered(fs, ft);
}

template void Cop1::cf<float>(u32 instr);
template void Cop1::cun<float>(u32 instr);
template void Cop1::ceq<float>(u32 instr);
template void Cop1::cueq<float>(u32 instr);
template void Cop1::colt<float>(u32 instr);
template void Cop1::cult<float>(u32 instr);
template void Cop1::cole<float>(u32 instr);
template void Cop1::cule<float>(u32 instr);
template void Cop1::csf<float>(u32 instr);
template void Cop1::cngle<float>(u32 instr);
template void Cop1::cseq<float>(u32 instr);
template void Cop1::cngl<float>(u32 instr);
template void Cop1::clt<float>(u32 instr);
template void Cop1::cnge<float>(u32 instr);
template void Cop1::cle<float>(u32 instr);
template void Cop1::cngt<float>(u32 instr);
template void Cop1::cf<double>(u32 instr);
template void Cop1::cun<double>(u32 instr);
template void Cop1::ceq<double>(u32 instr);
template void Cop1::cueq<double>(u32 instr);
template void Cop1::colt<double>(u32 instr);
template void Cop1::cult<double>(u32 instr);
template void Cop1::cole<double>(u32 instr);
template void Cop1::cule<double>(u32 instr);
template void Cop1::csf<double>(u32 instr);
template void Cop1::cngle<double>(u32 instr);
template void Cop1::cseq<double>(u32 instr);
template void Cop1::cngl<double>(u32 instr);
template void Cop1::clt<double>(u32 instr);
template void Cop1::cnge<double>(u32 instr);
template void Cop1::cle<double>(u32 instr);
template void Cop1::cngt<double>(u32 instr);

void Cop1::divs(u32 instr) {
  OP(float, fs / ft);
}

void Cop1::divd(u32 instr) {
  OP(double, fs / ft);
}

void Cop1::muls(u32 instr) {
  OP(float, fs * ft);
}

void Cop1::muld(u32 instr) {
  OP(double, fs * ft);
}

void Cop1::subs(u32 instr) {
  OP(float, fs - ft);
}

void Cop1::subd(u32 instr) {
  OP(double, fs - ft);
}

void Cop1::movs(u32 instr) {
  CheckFPUUsable_PreserveCause();
  auto val = FGR_S<u64>(regs.cop0.status, FS(instr));
  FGR_D<u64>(regs.cop0.status, FD(instr)) = val;
}

void Cop1::movd(u32 instr) {
  CheckFPUUsable_PreserveCause();
  auto val = FGR_S<double>(regs.cop0.status, FS(instr));
  FGR_D<double>(regs.cop0.status, FD(instr)) = val;
}

void Cop1::negs(u32 instr) {
  OP(float, -fs);
}

void Cop1::negd(u32 instr) {
  OP(double, -fs);
}

void Cop1::sqrts(u32 instr) {
  OP(float, std::sqrt(fs));
}

void Cop1::sqrtd(u32 instr) {
  OP(double, std::sqrt(fs));
}

void Cop1::roundls(u32 instr) {
  CheckFPUUsable();
  auto fs = FGR_S<float>(regs.cop0.status, FS(instr));
  CheckLCVTArg(fs);
  s64 result;
  CVT_OP_CheckExcept({ result = std::nearbyint(fs); });
  CheckRound(fs, result);
  FGR_D<s64>(regs.cop0.status, FD(instr)) = result;
}

void Cop1::roundld(u32 instr) {
  CheckFPUUsable();
  auto fs = FGR_S<double>(regs.cop0.status, FS(instr));
  CheckLCVTArg(fs);
  s64 result;
  CVT_OP_CheckExcept({ result = std::nearbyint(fs); });
  CheckRound(fs, result);
  FGR_D<s64>(regs.cop0.status, FD(instr)) = result;
}

void Cop1::roundws(u32 instr) {
  CheckFPUUsable();
  auto fs = FGR_S<float>(regs.cop0.status, FS(instr));
  CheckWCVTArg(fs);
  s32 result;
  CVT_OP_CheckExcept({ result = std::nearbyint(fs); });
  CheckRound(fs, result);
  FGR_D<s32>(regs.cop0.status, FD(instr)) = result;
}

void Cop1::roundwd(u32 instr) {
  CheckFPUUsable();
  auto fs = FGR_S<double>(regs.cop0.status, FS(instr));
  CheckWCVTArg(fs);
  s32 result;
  CVT_OP_CheckExcept({ result = std::nearbyint(fs); });
  CheckRound(fs, result);
  FGR_D<s32>(regs.cop0.status, FD(instr)) = result;
}

void Cop1::floorls(u32 instr) {
  CheckFPUUsable();
  auto fs = FGR_S<float>(regs.cop0.status, FS(instr));
  CheckLCVTArg(fs);
  s64 result;
  CVT_OP_CheckExcept({ result = std::floor(fs); });
  CheckRound(fs, result);
  FGR_D<s64>(regs.cop0.status, FD(instr)) = result;
}

void Cop1::floorld(u32 instr) {
  CheckFPUUsable();
  auto fs = FGR_S<double>(regs.cop0.status, FS(instr));
  CheckLCVTArg(fs);
  s64 result;
  CVT_OP_CheckExcept({ result = std::floor(fs); });
  CheckRound(fs, result);
  FGR_D<s64>(regs.cop0.status, FD(instr)) = result;
}

void Cop1::floorws(u32 instr) {
  CheckFPUUsable();
  auto fs = FGR_S<float>(regs.cop0.status, FS(instr));
  CheckWCVTArg(fs);
  s32 result;
  CVT_OP_CheckExcept({ result = std::floor(fs); });
  CheckRound(fs, result);
  FGR_D<s32>(regs.cop0.status, FD(instr)) = result;
}

void Cop1::floorwd(u32 instr) {
  CheckFPUUsable();
  auto fs = FGR_S<double>(regs.cop0.status, FS(instr));
  CheckWCVTArg(fs);
  s32 result;
  CVT_OP_CheckExcept({ result = std::floor(fs); });
  CheckRound(fs, result);
  FGR_D<s32>(regs.cop0.status, FD(instr)) = result;
}

void Cop1::truncws(u32 instr) {
  CheckFPUUsable();
  auto fs = FGR_S<float>(regs.cop0.status, FS(instr));
  CheckWCVTArg(fs);
  s32 result;
  CVT_OP_CheckExcept({ result = std::trunc(fs); });
  CheckRound(fs, result);
  FGR_D<s32>(regs.cop0.status, FD(instr)) = result;
}

void Cop1::truncwd(u32 instr) {
  CheckFPUUsable();
  auto fs = FGR_S<double>(regs.cop0.status, FS(instr));
  CheckWCVTArg(fs);
  s32 result;
  CVT_OP_CheckExcept({ result = std::trunc(fs); });
  CheckRound(fs, result);
  FGR_D<s32>(regs.cop0.status, FD(instr)) = result;
}

void Cop1::truncls(u32 instr) {
  CheckFPUUsable();
  auto fs = FGR_S<float>(regs.cop0.status, FS(instr));
  CheckLCVTArg(fs);
  s64 result;
  CVT_OP_CheckExcept({ result = std::trunc(fs); });
  CheckRound(fs, result);
  FGR_D<s64>(regs.cop0.status, FD(instr)) = result;
}

void Cop1::truncld(u32 instr) {
  CheckFPUUsable();
  auto fs = FGR_S<double>(regs.cop0.status, FS(instr));
  CheckLCVTArg(fs);
  s64 result;
  CVT_OP_CheckExcept({ result = std::trunc(fs); });
  CheckRound(fs, result);
  FGR_D<s64>(regs.cop0.status, FD(instr)) = result;
}

template<class T>
void Cop1::lwc1(T &cpu, Mem &mem, u32 instr) {
  if constexpr(std::is_same_v<decltype(cpu), Interpreter&>) {
    CheckFPUUsable_PreserveCause();
    lwc1Interp(mem, instr);
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
    CheckFPUUsable_PreserveCause();
    swc1Interp(mem, instr);
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
    CheckFPUUsable_PreserveCause();
    ldc1Interp(mem, instr);
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
    CheckFPUUsable_PreserveCause();
    sdc1Interp(mem, instr);
  } else if constexpr (std::is_same_v<decltype(cpu), JIT&>) {
    sdc1JIT(cpu, mem, instr);
  } else {
    Util::panic("What the fuck did you just give me?!!");
  }
}

template void Cop1::sdc1<Interpreter>(Interpreter&, Mem&, u32);
template void Cop1::sdc1<JIT>(JIT&, Mem&, u32);

void Cop1::lwc1Interp(Mem& mem, u32 instr) {
  u64 addr = (s64)(s16)instr + regs.gpr[BASE(instr)];

  u32 physical;
  if(!regs.cop0.MapVAddr(Cop0::LOAD, addr, physical)) {
    regs.cop0.HandleTLBException(addr);
    regs.cop0.FireException(regs.cop0.GetTLBExceptionCode(regs.cop0.tlbError, Cop0::LOAD), 0, regs.oldPC);
  } else {
    u32 data = mem.Read<u32>(regs, physical);
    FGR_T<u32>(regs.cop0.status, FT(instr)) = data;
  }
}

void Cop1::swc1Interp(Mem& mem, u32 instr) {
  u64 addr = (s64)(s16)instr + regs.gpr[BASE(instr)];

  u32 physical;
  if(!regs.cop0.MapVAddr(Cop0::STORE, addr, physical)) {
    regs.cop0.HandleTLBException(addr);
    regs.cop0.FireException(regs.cop0.GetTLBExceptionCode(regs.cop0.tlbError, Cop0::STORE), 0, regs.oldPC);
  } else {
    mem.Write<u32>(regs, physical, FGR_T<u32>(regs.cop0.status, FT(instr)));
  }
}

void Cop1::unimplemented() {
  CheckFPUUsable();
  fcr31.cause_unimplemented_operation = true;
  FireException();
}

void Cop1::ldc1Interp(Mem& mem, u32 instr) {
  u64 addr = (s64)(s16)instr + regs.gpr[BASE(instr)];

  u32 physical;
  if(!regs.cop0.MapVAddr(Cop0::LOAD, addr, physical)) {
    regs.cop0.HandleTLBException(addr);
    regs.cop0.FireException(regs.cop0.GetTLBExceptionCode(regs.cop0.tlbError, Cop0::LOAD), 0, regs.oldPC);
  } else {
    u64 data = mem.Read<u64>(regs, physical);
    FGR_T<u64>(regs.cop0.status, FT(instr)) = data;
  }
}

void Cop1::sdc1Interp(Mem& mem, u32 instr) {
  u64 addr = (s64)(s16)instr + regs.gpr[BASE(instr)];

  u32 physical;
  if(!regs.cop0.MapVAddr(Cop0::STORE, addr, physical)) {
    regs.cop0.HandleTLBException(addr);
    regs.cop0.FireException(regs.cop0.GetTLBExceptionCode(regs.cop0.tlbError, Cop0::STORE), 0, regs.oldPC);
  } else {
    mem.Write(regs, physical, FGR_T<u64>(regs.cop0.status, FT(instr)));
  }
}

void Cop1::mfc1(u32 instr) {
  CheckFPUUsable_PreserveCause();
  regs.gpr[RT(instr)] = FGR_T<s32>(regs.cop0.status, FS(instr));
}

void Cop1::dmfc1(u32 instr) {
  CheckFPUUsable_PreserveCause();
  regs.gpr[RT(instr)] = FGR_S<s64>(regs.cop0.status, FS(instr));
}

void Cop1::mtc1(u32 instr) {
  CheckFPUUsable_PreserveCause();
  FGR_T<s32>(regs.cop0.status, FS(instr)) = regs.gpr[RT(instr)];
}

void Cop1::dmtc1(u32 instr) {
  CheckFPUUsable_PreserveCause();
  FGR_S<u64>(regs.cop0.status, FS(instr)) = regs.gpr[RT(instr)];
}

}
