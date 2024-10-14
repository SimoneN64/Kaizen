#include <cfenv>
#include <cmath>
#include <core/Interpreter.hpp>
#include <core/registers/Cop1.hpp>
#include <core/registers/Registers.hpp>
#include <utils/FloatingPoint.hpp>

namespace n64 {
template <>
auto Cop1::FGR_T<s32>(Cop0Status &status, u32 index) -> s32 & {
  if (status.fr) {
    return fgr[index].int32;
  } else if (index & 1) {
    return fgr[index & ~1].int32h;
  } else {
    return fgr[index & ~1].int32;
  }
}

template <>
auto Cop1::FGR_T<u32>(Cop0Status &status, u32 index) -> u32 & {
  return (u32 &)FGR_T<s32>(status, index);
}

template <>
auto Cop1::FGR_T<float>(Cop0Status &, u32 index) -> float & {
  return fgr[index].float32;
}

template <>
auto Cop1::FGR_T<s64>(Cop0Status &status, u32 index) -> s64 & {
  if (status.fr) {
    return fgr[index].int64;
  } else {
    return fgr[index & ~1].int64;
  }
}

template <>
auto Cop1::FGR_T<u64>(Cop0Status &status, u32 index) -> u64 & {
  return (u64 &)FGR_T<s64>(status, index);
}

template <>
auto Cop1::FGR_T<double>(Cop0Status &, u32 index) -> double & {
  return fgr[index].float64;
}

template <>
auto Cop1::FGR_S<s32>(Cop0Status &status, u32 index) -> s32 & {
  if (status.fr) {
    return fgr[index].int32;
  } else {
    return fgr[index & ~1].int32;
  }
}

template <>
auto Cop1::FGR_S<u32>(Cop0Status &status, u32 index) -> u32 & {
  return (u32 &)FGR_S<s32>(status, index);
}

template <>
auto Cop1::FGR_S<float>(Cop0Status &status, u32 index) -> float & {
  if (status.fr) {
    return fgr[index].float32;
  } else {
    return fgr[index & ~1].float32;
  }
}

template <>
auto Cop1::FGR_S<s64>(Cop0Status &status, u32 index) -> s64 & {
  return FGR_T<s64>(status, index);
}

template <>
auto Cop1::FGR_S<u64>(Cop0Status &status, u32 index) -> u64 & {
  return (u64 &)FGR_S<s64>(status, index);
}

template <>
auto Cop1::FGR_S<double>(Cop0Status &status, u32 index) -> double & {
  if (status.fr) {
    return fgr[index].float64;
  } else {
    return fgr[index & ~1].float64;
  }
}

template <>
auto Cop1::FGR_D<s32>(Cop0Status &, u32 index) -> s32 & {
  fgr[index].int32h = 0;
  return fgr[index].int32;
}

template <>
auto Cop1::FGR_D<u32>(Cop0Status &status, u32 index) -> u32 & {
  return (u32 &)FGR_D<s32>(status, index);
}

template <>
auto Cop1::FGR_D<float>(Cop0Status &, u32 index) -> float & {
  fgr[index].float32h = 0;
  return fgr[index].float32;
}

template <>
auto Cop1::FGR_D<s64>(Cop0Status &, u32 index) -> s64 & {
  return fgr[index].int64;
}

template <>
auto Cop1::FGR_D<u64>(Cop0Status &status, u32 index) -> u64 & {
  return (u64 &)FGR_D<s64>(status, index);
}

template <>
auto Cop1::FGR_D<double>(Cop0Status &status, u32 index) -> double & {
  return FGR_T<double>(status, index);
}

template <typename To, typename From>
To floatIntegerCast(From f) {
  return std::bit_cast<To, From>(f);
}

template <>
bool Cop1::isqnan<float>(float f) {
  return (floatIntegerCast<u32>(f) >> 22) & 1;
}

template <>
bool Cop1::isqnan<double>(double f) {
  return (floatIntegerCast<u64>(f) >> 51) & 1;
}

template <>
bool Cop1::CheckCVTArg<s32>(float &f) {
  switch (std::fpclassify(f)) {
  case FP_SUBNORMAL:
  case FP_INFINITE:
  case FP_NAN:
    SetCauseUnimplemented();
    regs.cop0.FireException(ExceptionCode::FloatingPointError, 0, regs.oldPC);
    return false;
  }

  if (f >= 0x1p+31f || f < -0x1p+31f) {
    SetCauseUnimplemented();
    regs.cop0.FireException(ExceptionCode::FloatingPointError, 0, regs.oldPC);
    return false;
  }

  return true;
}

template <>
bool Cop1::CheckCVTArg<s32>(double &f) {
  switch (std::fpclassify(f)) {
  case FP_SUBNORMAL:
  case FP_INFINITE:
  case FP_NAN:
    SetCauseUnimplemented();
    regs.cop0.FireException(ExceptionCode::FloatingPointError, 0, regs.oldPC);
    return false;
  }

  if ((f >= 0x1p+31 || f < -0x1p+31)) {
    SetCauseUnimplemented();
    regs.cop0.FireException(ExceptionCode::FloatingPointError, 0, regs.oldPC);
    return false;
  }

  return true;
}

template <>
bool Cop1::CheckCVTArg<s64>(float &f) {
  switch (std::fpclassify(f)) {
  case FP_SUBNORMAL:
  case FP_INFINITE:
  case FP_NAN:
    SetCauseUnimplemented();
    regs.cop0.FireException(ExceptionCode::FloatingPointError, 0, regs.oldPC);
    return false;
  }

  if ((f >= 0x1p+53f || f <= -0x1p+53f)) {
    SetCauseUnimplemented();
    regs.cop0.FireException(ExceptionCode::FloatingPointError, 0, regs.oldPC);
    return false;
  }

  return true;
}

template <>
bool Cop1::CheckCVTArg<s64>(double &f) {
  switch (std::fpclassify(f)) {
  case FP_SUBNORMAL:
  case FP_INFINITE:
  case FP_NAN:
    SetCauseUnimplemented();
    regs.cop0.FireException(ExceptionCode::FloatingPointError, 0, regs.oldPC);
    return false;
  }

  if ((f >= 0x1p+53 || f <= -0x1p+53)) {
    SetCauseUnimplemented();
    regs.cop0.FireException(ExceptionCode::FloatingPointError, 0, regs.oldPC);
    return false;
  }

  return true;
}

template <typename T>
bool Cop1::CheckArg(T &f) {
  switch (std::fpclassify(f)) {
  case FP_SUBNORMAL:
    SetCauseUnimplemented();
    regs.cop0.FireException(ExceptionCode::FloatingPointError, 0, regs.oldPC);
    return false;
  case FP_NAN:
    if (isqnan(f) ? SetCauseInvalid() : (SetCauseUnimplemented(), true)) {
      regs.cop0.FireException(ExceptionCode::FloatingPointError, 0, regs.oldPC);
      return false;
    }
    return true;
  }
  return true;
}

template <typename T>
bool Cop1::CheckArgs(T &f1, T &f2) {
  auto class1 = std::fpclassify(f1), class2 = std::fpclassify(f2);
  if ((class1 == FP_NAN && !isqnan(f1)) || (class2 == FP_NAN && !isqnan(f2))) {
    SetCauseUnimplemented();
    regs.cop0.FireException(ExceptionCode::FloatingPointError, 0, regs.oldPC);
    return false;
  }

  if (class1 == FP_SUBNORMAL || class2 == FP_SUBNORMAL) {
    SetCauseUnimplemented();
    regs.cop0.FireException(ExceptionCode::FloatingPointError, 0, regs.oldPC);
    return false;
  }

  if ((class1 == FP_NAN && isqnan(f1)) || (class2 == FP_NAN && isqnan(f2))) {
    if (SetCauseInvalid()) {
      regs.cop0.FireException(ExceptionCode::FloatingPointError, 0, regs.oldPC);
      return false;
    }
  }

  return true;
}

template <bool preserveCause>
bool Cop1::CheckFPUUsable() {
  if constexpr (preserveCause) {
    if (!regs.cop0.status.cu1) {
      regs.cop0.FireException(ExceptionCode::CoprocessorUnusable, 1, regs.oldPC);
      return false;
    }
  } else {
    if (!CheckFPUUsable<true>())
      return false;
    fcr31.cause = {};
  }

  return true;
}

template bool Cop1::CheckFPUUsable<true>();
template bool Cop1::CheckFPUUsable<false>();

template <typename T>
FORCE_INLINE T FlushResult(T f, u32 round) {
  switch (round) {
  case FE_TONEAREST:
  case FE_TOWARDZERO:
    return std::copysign(T(), f);
  case FE_UPWARD:
    return std::signbit(f) ? -T() : std::numeric_limits<T>::min();
  case FE_DOWNWARD:
    return std::signbit(f) ? -std::numeric_limits<T>::min() : T();
  default:
    __builtin_unreachable();
  }
}

template <>
bool Cop1::CheckResult<float>(float &f) {
  switch (std::fpclassify(f)) {
  case FP_SUBNORMAL:
    if (!fcr31.fs || fcr31.enable.underflow || fcr31.enable.inexact_operation) {
      SetCauseUnimplemented();
      regs.cop0.FireException(ExceptionCode::FloatingPointError, 0, regs.oldPC);
      return false;
    }
    SetCauseUnderflow();
    SetCauseInexact();
    f = FlushResult(f, std::fegetround());
    return true;
  case FP_NAN:
    {
      f = std::bit_cast<float, u32>(0x7fbf'ffff);
      return true;
    }
  }
  return true;
}

template <>
bool Cop1::CheckResult<double>(double &f) {
  switch (std::fpclassify(f)) {
  case FP_SUBNORMAL:
    if (!fcr31.fs || fcr31.enable.underflow || fcr31.enable.inexact_operation) {
      SetCauseUnimplemented();
      regs.cop0.FireException(ExceptionCode::FloatingPointError, 0, regs.oldPC);
      return false;
    }
    SetCauseUnderflow();
    SetCauseInexact();
    f = FlushResult(f, fegetround());
    return true;
  case FP_NAN:
    {
      f = std::bit_cast<double, u64>(0x7ff7'ffff'ffff'ffff);
      return true;
    }
  }
  return true;
}

template <bool cvt>
bool Cop1::TestExceptions() {
  u32 exc = std::fetestexcept(FE_ALL_EXCEPT);

  if (!exc)
    return false;

  if constexpr (cvt) {
    if (exc & FE_INVALID) {
      SetCauseUnimplemented();
      regs.cop0.FireException(ExceptionCode::FloatingPointError, 0, regs.oldPC);
      return true;
    }
  }

  if (exc & FE_UNDERFLOW) {
    if (!fcr31.fs || fcr31.enable.underflow || fcr31.enable.inexact_operation) {
      SetCauseUnimplemented();
      regs.cop0.FireException(ExceptionCode::FloatingPointError, 0, regs.oldPC);
      return true;
    }
  }

  bool raise = false;
  if (exc & FE_DIVBYZERO)
    raise |= SetCauseDivisionByZero();
  if (exc & FE_INEXACT) {
    raise |= SetCauseInexact();
  }
  if (exc & FE_UNDERFLOW)
    raise |= SetCauseUnderflow();
  if (exc & FE_OVERFLOW)
    raise |= SetCauseOverflow();
  if (exc & FE_INVALID)
    raise |= SetCauseInvalid();
  if (raise)
    regs.cop0.FireException(ExceptionCode::FloatingPointError, 0, regs.oldPC);
  return raise;
}

template bool Cop1::TestExceptions<false>();
template bool Cop1::TestExceptions<true>();

void Cop1::SetCauseUnimplemented() { fcr31.cause.unimplemented_operation = true; }

bool Cop1::SetCauseUnderflow() {
  fcr31.cause.underflow = true;
  if (fcr31.enable.underflow)
    return true;
  fcr31.flag.underflow = true;
  return false;
}

bool Cop1::SetCauseInexact() {
  fcr31.cause.inexact_operation = true;
  if (fcr31.enable.inexact_operation)
    return true;
  fcr31.flag.inexact_operation = true;
  return false;
}

bool Cop1::SetCauseDivisionByZero() {
  fcr31.cause.division_by_zero = true;
  if (fcr31.enable.division_by_zero)
    return true;
  fcr31.flag.division_by_zero = true;
  return false;
}

bool Cop1::SetCauseOverflow() {
  fcr31.cause.overflow = true;
  if (fcr31.enable.overflow)
    return true;
  fcr31.flag.overflow = true;
  return false;
}

bool Cop1::SetCauseInvalid() {
  fcr31.cause.invalid_operation = true;
  if (fcr31.enable.invalid_operation)
    return true;
  fcr31.flag.invalid_operation = true;
  return false;
}

#define CHECK_FPE_IMPL(type, res, operation, convert)                                                                  \
  feclearexcept(FE_ALL_EXCEPT);                                                                                        \
  volatile type v##res = [&]() -> type { return operation; }();                                                        \
  if (TestExceptions<convert>())                                                                                       \
    return;                                                                                                            \
  type res = v##res;

#define CHECK_FPE(type, res, operation) CHECK_FPE_IMPL(type, res, operation, false)
#define CHECK_FPE_CONV(type, res, operation) CHECK_FPE_IMPL(type, res, operation, true)

void Cop1::absd(u32 instr) {
  if (!CheckFPUUsable())
    return;
  auto fs = FGR_S<double>(regs.cop0.status, FS(instr));
  if (!CheckArg(fs))
    return;
  auto fd = std::abs(fs);
  if (!CheckResult(fd))
    return;
  FGR_D<double>(regs.cop0.status, FD(instr)) = fd;
}

void Cop1::abss(u32 instr) {
  if (!CheckFPUUsable())
    return;
  auto fs = FGR_S<float>(regs.cop0.status, FS(instr));
  if (!CheckArg(fs))
    return;
  auto fd = std::abs(fs);
  if (!CheckResult(fd))
    return;
  FGR_D<float>(regs.cop0.status, FD(instr)) = fd;
}

void Cop1::adds(u32 instr) {
  if (!CheckFPUUsable())
    return;
  auto fs = FGR_S<float>(regs.cop0.status, FS(instr));
  auto ft = FGR_T<float>(regs.cop0.status, FT(instr));
  if (!CheckArgs(fs, ft))
    return;
  CHECK_FPE(float, fd, fs + ft)
  if (!CheckResult(fd))
    return;
  FGR_D<float>(regs.cop0.status, FD(instr)) = fd;
}

void Cop1::addd(u32 instr) {
  if (!CheckFPUUsable())
    return;
  auto fs = FGR_S<double>(regs.cop0.status, FS(instr));
  auto ft = FGR_T<double>(regs.cop0.status, FT(instr));
  if (!CheckArgs(fs, ft))
    return;
  CHECK_FPE(double, fd, fs + ft)
  if (!CheckResult(fd))
    return;
  FGR_D<double>(regs.cop0.status, FD(instr)) = fd;
}

void Cop1::ceills(u32 instr) {
  if (!CheckFPUUsable())
    return;
  auto fs = FGR_S<float>(regs.cop0.status, FS(instr));
  if (!CheckCVTArg<s64>(fs))
    return;
  CHECK_FPE(s64, fd, Util::roundCeil<s64>(fs))
  FGR_D<s64>(regs.cop0.status, FD(instr)) = fd;
}

void Cop1::ceilld(u32 instr) {
  if (!CheckFPUUsable())
    return;
  auto fs = FGR_S<double>(regs.cop0.status, FS(instr));
  if (!CheckCVTArg<s64>(fs))
    return;
  CHECK_FPE(s64, fd, Util::roundCeil<s64>(fs))
  FGR_D<s64>(regs.cop0.status, FD(instr)) = fd;
}

void Cop1::ceilws(u32 instr) {
  if (!CheckFPUUsable())
    return;
  auto fs = FGR_S<float>(regs.cop0.status, FS(instr));
  if (!CheckCVTArg<s32>(fs))
    return;
  CHECK_FPE_CONV(s32, fd, Util::roundCeil<s32>(fs))
  FGR_D<s32>(regs.cop0.status, FD(instr)) = fd;
}

void Cop1::ceilwd(u32 instr) {
  if (!CheckFPUUsable())
    return;
  auto fs = FGR_S<double>(regs.cop0.status, FS(instr));
  if (!CheckCVTArg<s32>(fs))
    return;
  CHECK_FPE_CONV(s32, fd, Util::roundCeil<s32>(fs))
  FGR_D<s32>(regs.cop0.status, FD(instr)) = fd;
}

void Cop1::cfc1(u32 instr) {
  if (!CheckFPUUsable<true>())
    return;
  u8 fd = RD(instr);
  s32 val = 0;
  switch (fd) {
  case 0:
    val = fcr0;
    break;
  case 31:
    val = fcr31.read();
    break;
  default:
    Util::panic("Undefined CFC1 with rd != 0 or 31");
  }
  regs.Write(RT(instr), val);
}

void Cop1::ctc1(u32 instr) {
  if (!CheckFPUUsable<true>())
    return;
  u8 fs = RD(instr);
  u32 val = regs.Read<s64>(RT(instr));
  switch (fs) {
  case 0:
    break;
  case 31:
    {
      u32 prevRound = fcr31.rounding_mode;
      fcr31.write(val);
      if (prevRound != fcr31.rounding_mode) {
        switch (fcr31.rounding_mode) {
        case 0:
          fesetround(FE_TONEAREST);
          break;
        case 1:
          fesetround(FE_TOWARDZERO);
          break;
        case 2:
          fesetround(FE_UPWARD);
          break;
        case 3:
          fesetround(FE_DOWNWARD);
          break;
        }
      }
      if (fcr31.cause.inexact_operation && fcr31.enable.inexact_operation)
        regs.cop0.FireException(ExceptionCode::FloatingPointError, 0, regs.oldPC);
      if (fcr31.cause.underflow && fcr31.enable.underflow)
        regs.cop0.FireException(ExceptionCode::FloatingPointError, 0, regs.oldPC);
      if (fcr31.cause.overflow && fcr31.enable.overflow)
        regs.cop0.FireException(ExceptionCode::FloatingPointError, 0, regs.oldPC);
      if (fcr31.cause.division_by_zero && fcr31.enable.division_by_zero)
        regs.cop0.FireException(ExceptionCode::FloatingPointError, 0, regs.oldPC);
      if (fcr31.cause.invalid_operation && fcr31.enable.invalid_operation)
        regs.cop0.FireException(ExceptionCode::FloatingPointError, 0, regs.oldPC);
      if (fcr31.cause.unimplemented_operation)
        regs.cop0.FireException(ExceptionCode::FloatingPointError, 0, regs.oldPC);
    }
    break;
  default:
    Util::panic("Undefined CTC1 with rd != 0 or 31");
  }
}

void Cop1::cvtds(u32 instr) {
  if (!CheckFPUUsable())
    return;
  auto fs = FGR_S<float>(regs.cop0.status, FS(instr));
  if (!CheckArg(fs))
    return;
  CHECK_FPE(double, fd, fs)
  if (!CheckResult(fd))
    return;
  FGR_D<double>(regs.cop0.status, FD(instr)) = fd;
}

void Cop1::cvtsd(u32 instr) {
  if (!CheckFPUUsable())
    return;
  auto fs = FGR_S<double>(regs.cop0.status, FS(instr));
  if (!CheckArg(fs))
    return;
  CHECK_FPE(float, fd, (float)fs)
  if (!CheckResult(fd))
    return;
  FGR_D<float>(regs.cop0.status, FD(instr)) = fd;
}

void Cop1::cvtsw(u32 instr) {
  if (!CheckFPUUsable())
    return;
  auto fs = FGR_S<s32>(regs.cop0.status, FS(instr));
  CHECK_FPE(float, fd, fs)
  if (!CheckResult(fd))
    return;
  FGR_D<float>(regs.cop0.status, FD(instr)) = fd;
}

void Cop1::cvtsl(u32 instr) {
  if (!CheckFPUUsable())
    return;
  auto fs = FGR_S<s64>(regs.cop0.status, FS(instr));
  if (fs >= s64(0x0080000000000000) || fs < s64(0xff80000000000000)) {
    SetCauseUnimplemented();
    regs.cop0.FireException(ExceptionCode::FloatingPointError, 0, regs.oldPC);
    return;
  }
  CHECK_FPE(float, fd, fs)
  if (!CheckResult(fd))
    return;
  FGR_D<float>(regs.cop0.status, FD(instr)) = fd;
}

void Cop1::cvtwd(u32 instr) {
  if (!CheckFPUUsable())
    return;
  auto fs = FGR_S<double>(regs.cop0.status, FS(instr));
  if (!CheckCVTArg<s32>(fs))
    return;
  CHECK_FPE_CONV(s32, fd, Util::roundCurrent<s32>(fs))
  FGR_D<s32>(regs.cop0.status, FD(instr)) = fd;
}

void Cop1::cvtws(u32 instr) {
  if (!CheckFPUUsable())
    return;
  auto fs = FGR_S<float>(regs.cop0.status, FS(instr));
  if (!CheckCVTArg<s32>(fs))
    return;
  CHECK_FPE_CONV(s32, fd, Util::roundCurrent<s32>(fs))
  FGR_D<s32>(regs.cop0.status, FD(instr)) = fd;
}

void Cop1::cvtls(u32 instr) {
  if (!CheckFPUUsable())
    return;
  auto fs = FGR_S<float>(regs.cop0.status, FS(instr));
  if (!CheckCVTArg<s64>(fs))
    return;
  CHECK_FPE(s64, fd, Util::roundCurrent<s64>(fs))
  FGR_D<s64>(regs.cop0.status, FD(instr)) = fd;
}

void Cop1::cvtdw(u32 instr) {
  if (!CheckFPUUsable())
    return;
  auto fs = FGR_S<s32>(regs.cop0.status, FS(instr));
  CHECK_FPE(double, fd, fs)
  if (!CheckResult(fd))
    return;
  FGR_D<double>(regs.cop0.status, FD(instr)) = fd;
}

void Cop1::cvtdl(u32 instr) {
  if (!CheckFPUUsable())
    return;
  auto fs = FGR_S<s64>(regs.cop0.status, FS(instr));

  if (fs >= s64(0x0080000000000000) || fs < s64(0xff80000000000000)) {
    SetCauseUnimplemented();
    regs.cop0.FireException(ExceptionCode::FloatingPointError, 0, regs.oldPC);
    return;
  }
  CHECK_FPE(double, fd, fs)
  if (!CheckResult(fd))
    return;
  FGR_D<double>(regs.cop0.status, FD(instr)) = fd;
}

void Cop1::cvtld(u32 instr) {
  if (!CheckFPUUsable())
    return;
  auto fs = FGR_S<double>(regs.cop0.status, FS(instr));
  if (!CheckCVTArg<s64>(fs))
    return;
  CHECK_FPE(s64, fd, Util::roundCurrent<s64>(fs))
  FGR_D<s64>(regs.cop0.status, FD(instr)) = fd;
}

template <typename T, bool quiet, bool cf>
bool Cop1::XORDERED(T fs, T ft) {
  if (std::isnan(fs) || std::isnan(ft)) {
    if (std::isnan(fs) && (!quiet || isqnan(fs)) && SetCauseInvalid()) {
      regs.cop0.FireException(ExceptionCode::FloatingPointError, 0, regs.oldPC);
      return false;
    }
    if (std::isnan(ft) && (!quiet || isqnan(ft)) && SetCauseInvalid()) {
      regs.cop0.FireException(ExceptionCode::FloatingPointError, 0, regs.oldPC);
      return false;
    }
    fcr31.compare = cf;
    return false;
  }

  return true;
}

#define ORDERED(type, cf) XORDERED<type, 0, cf>
#define UNORDERED(type, cf) XORDERED<type, 1, cf>

template <typename T>
void Cop1::cf(u32 instr) {
  if (!CheckFPUUsable())
    return;
  T fs = FGR_S<T>(regs.cop0.status, FS(instr));
  T ft = FGR_T<T>(regs.cop0.status, FT(instr));
  if (!UNORDERED(T, 0)(fs, ft))
    return;
  fcr31.compare = 0;
}

template <typename T>
void Cop1::cun(u32 instr) {
  if (!CheckFPUUsable())
    return;
  T fs = FGR_S<T>(regs.cop0.status, FS(instr));
  T ft = FGR_T<T>(regs.cop0.status, FT(instr));
  if (!UNORDERED(T, 1)(fs, ft))
    return;
  fcr31.compare = 0;
}

template <typename T>
void Cop1::ceq(u32 instr) {
  if (!CheckFPUUsable())
    return;
  T fs = FGR_S<T>(regs.cop0.status, FS(instr));
  T ft = FGR_T<T>(regs.cop0.status, FT(instr));
  if (!UNORDERED(T, 0)(fs, ft))
    return;
  fcr31.compare = fs == ft;
}

template <typename T>
void Cop1::cueq(u32 instr) {
  if (!CheckFPUUsable())
    return;
  T fs = FGR_S<T>(regs.cop0.status, FS(instr));
  T ft = FGR_T<T>(regs.cop0.status, FT(instr));
  if (!UNORDERED(T, 1)(fs, ft))
    return;
  fcr31.compare = fs == ft;
}

template <typename T>
void Cop1::colt(u32 instr) {
  if (!CheckFPUUsable())
    return;
  T fs = FGR_S<T>(regs.cop0.status, FS(instr));
  T ft = FGR_T<T>(regs.cop0.status, FT(instr));
  if (!UNORDERED(T, 0)(fs, ft))
    return;
  fcr31.compare = fs < ft;
}

template <typename T>
void Cop1::cult(u32 instr) {
  if (!CheckFPUUsable())
    return;
  T fs = FGR_S<T>(regs.cop0.status, FS(instr));
  T ft = FGR_T<T>(regs.cop0.status, FT(instr));
  if (!UNORDERED(T, 1)(fs, ft))
    return;
  fcr31.compare = fs < ft;
}

template <typename T>
void Cop1::cole(u32 instr) {
  if (!CheckFPUUsable())
    return;
  T fs = FGR_S<T>(regs.cop0.status, FS(instr));
  T ft = FGR_T<T>(regs.cop0.status, FT(instr));
  if (!UNORDERED(T, 0)(fs, ft))
    return;
  fcr31.compare = fs <= ft;
}

template <typename T>
void Cop1::cule(u32 instr) {
  if (!CheckFPUUsable())
    return;
  T fs = FGR_S<T>(regs.cop0.status, FS(instr));
  T ft = FGR_T<T>(regs.cop0.status, FT(instr));
  if (!UNORDERED(T, 1)(fs, ft))
    return;
  fcr31.compare = fs <= ft;
}

template <typename T>
void Cop1::csf(u32 instr) {
  if (!CheckFPUUsable())
    return;
  T fs = FGR_S<T>(regs.cop0.status, FS(instr));
  T ft = FGR_T<T>(regs.cop0.status, FT(instr));
  if (!ORDERED(T, 0)(fs, ft))
    return;
  fcr31.compare = 0;
}

template <typename T>
void Cop1::cngle(u32 instr) {
  if (!CheckFPUUsable())
    return;
  T fs = FGR_S<T>(regs.cop0.status, FS(instr));
  T ft = FGR_T<T>(regs.cop0.status, FT(instr));
  if (!ORDERED(T, 1)(fs, ft))
    return;
  fcr31.compare = 0;
}

template <typename T>
void Cop1::cseq(u32 instr) {
  if (!CheckFPUUsable())
    return;
  T fs = FGR_S<T>(regs.cop0.status, FS(instr));
  T ft = FGR_T<T>(regs.cop0.status, FT(instr));
  if (!ORDERED(T, 0)(fs, ft))
    return;
  fcr31.compare = fs == ft;
}

template <typename T>
void Cop1::cngl(u32 instr) {
  if (!CheckFPUUsable())
    return;
  T fs = FGR_S<T>(regs.cop0.status, FS(instr));
  T ft = FGR_T<T>(regs.cop0.status, FT(instr));
  if (!ORDERED(T, 1)(fs, ft))
    return;
  fcr31.compare = fs == ft;
}

template <typename T>
void Cop1::clt(u32 instr) {
  if (!CheckFPUUsable())
    return;
  T fs = FGR_S<T>(regs.cop0.status, FS(instr));
  T ft = FGR_T<T>(regs.cop0.status, FT(instr));
  if (!ORDERED(T, 0)(fs, ft))
    return;
  fcr31.compare = fs < ft;
}

template <typename T>
void Cop1::cnge(u32 instr) {
  if (!CheckFPUUsable())
    return;
  T fs = FGR_S<T>(regs.cop0.status, FS(instr));
  T ft = FGR_T<T>(regs.cop0.status, FT(instr));
  if (!ORDERED(T, 1)(fs, ft))
    return;
  fcr31.compare = fs < ft;
}

template <typename T>
void Cop1::cle(u32 instr) {
  if (!CheckFPUUsable())
    return;
  T fs = FGR_S<T>(regs.cop0.status, FS(instr));
  T ft = FGR_T<T>(regs.cop0.status, FT(instr));
  if (!ORDERED(T, 0)(fs, ft))
    return;
  fcr31.compare = fs <= ft;
}

template <typename T>
void Cop1::cngt(u32 instr) {
  if (!CheckFPUUsable())
    return;
  T fs = FGR_S<T>(regs.cop0.status, FS(instr));
  T ft = FGR_T<T>(regs.cop0.status, FT(instr));
  if (!ORDERED(T, 1)(fs, ft))
    return;
  fcr31.compare = fs <= ft;
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
  if (!CheckFPUUsable())
    return;
  auto fs = FGR_S<float>(regs.cop0.status, FS(instr));
  auto ft = FGR_T<float>(regs.cop0.status, FT(instr));
  if (!CheckArgs(fs, ft))
    return;
  CHECK_FPE(float, fd, fs / ft)
  if (!CheckResult(fd))
    return;
  FGR_D<float>(regs.cop0.status, FD(instr)) = fd;
}

void Cop1::divd(u32 instr) {
  if (!CheckFPUUsable())
    return;
  auto fs = FGR_S<double>(regs.cop0.status, FS(instr));
  auto ft = FGR_T<double>(regs.cop0.status, FT(instr));
  if (!CheckArgs(fs, ft))
    return;
  CHECK_FPE(double, fd, fs / ft)
  if (!CheckResult(fd))
    return;
  FGR_D<double>(regs.cop0.status, FD(instr)) = fd;
}

void Cop1::muls(u32 instr) {
  if (!CheckFPUUsable())
    return;
  auto fs = FGR_S<float>(regs.cop0.status, FS(instr));
  auto ft = FGR_T<float>(regs.cop0.status, FT(instr));
  if (!CheckArgs(fs, ft))
    return;
  CHECK_FPE(float, fd, fs *ft)
  if (!CheckResult(fd))
    return;
  FGR_D<float>(regs.cop0.status, FD(instr)) = fd;
}

void Cop1::muld(u32 instr) {
  if (!CheckFPUUsable())
    return;
  auto fs = FGR_S<double>(regs.cop0.status, FS(instr));
  auto ft = FGR_T<double>(regs.cop0.status, FT(instr));
  if (!CheckArgs(fs, ft))
    return;
  CHECK_FPE(double, fd, fs *ft)
  if (!CheckResult(fd))
    return;
  FGR_D<double>(regs.cop0.status, FD(instr)) = fd;
}

void Cop1::subs(u32 instr) {
  if (!CheckFPUUsable())
    return;
  auto fs = FGR_S<float>(regs.cop0.status, FS(instr));
  auto ft = FGR_T<float>(regs.cop0.status, FT(instr));
  if (!CheckArgs(fs, ft))
    return;
  CHECK_FPE(float, fd, fs - ft)
  if (!CheckResult(fd))
    return;
  FGR_D<float>(regs.cop0.status, FD(instr)) = fd;
}

void Cop1::subd(u32 instr) {
  if (!CheckFPUUsable())
    return;
  auto fs = FGR_S<double>(regs.cop0.status, FS(instr));
  auto ft = FGR_T<double>(regs.cop0.status, FT(instr));
  if (!CheckArgs(fs, ft))
    return;
  CHECK_FPE(double, fd, fs - ft)
  if (!CheckResult(fd))
    return;
  FGR_D<double>(regs.cop0.status, FD(instr)) = fd;
}

void Cop1::movs(u32 instr) { movd(instr); }

void Cop1::movd(u32 instr) {
  if (!CheckFPUUsable<true>())
    return;
  FGR_D<double>(regs.cop0.status, FD(instr)) = FGR_S<double>(regs.cop0.status, FS(instr));
}

void Cop1::negs(u32 instr) {
  if (!CheckFPUUsable())
    return;
  auto fs = FGR_S<float>(regs.cop0.status, FS(instr));
  if (!CheckArg(fs))
    return;
  CHECK_FPE(float, fd, -fs)
  if (!CheckResult(fd))
    return;
  FGR_D<float>(regs.cop0.status, FD(instr)) = fd;
}

void Cop1::negd(u32 instr) {
  if (!CheckFPUUsable())
    return;
  auto fs = FGR_S<double>(regs.cop0.status, FS(instr));
  if (!CheckArg(fs))
    return;
  CHECK_FPE(double, fd, -fs)
  if (!CheckResult(fd))
    return;
  FGR_D<double>(regs.cop0.status, FD(instr)) = fd;
}

void Cop1::sqrts(u32 instr) {
  if (!CheckFPUUsable())
    return;
  auto fs = FGR_S<float>(regs.cop0.status, FS(instr));
  if (!CheckArg(fs))
    return;
  CHECK_FPE(float, fd, sqrtf(fs))
  if (!CheckResult(fd))
    return;
  FGR_D<float>(regs.cop0.status, FD(instr)) = fd;
}

void Cop1::sqrtd(u32 instr) {
  if (!CheckFPUUsable())
    return;
  auto fs = FGR_S<double>(regs.cop0.status, FS(instr));
  if (!CheckArg(fs))
    return;
  CHECK_FPE(double, fd, sqrt(fs))
  if (!CheckResult(fd))
    return;
  FGR_D<double>(regs.cop0.status, FD(instr)) = fd;
}

void Cop1::roundls(u32 instr) {
  if (!CheckFPUUsable())
    return;
  auto fs = FGR_S<float>(regs.cop0.status, FS(instr));
  if (!CheckCVTArg<s64>(fs))
    return;
  CHECK_FPE(s64, fd, Util::roundNearest<s64>(fs))
  if (fd != fs && SetCauseInexact()) {
    regs.cop0.FireException(ExceptionCode::FloatingPointError, 0, regs.oldPC);
    return;
  }
  FGR_D<s64>(regs.cop0.status, FD(instr)) = fd;
}

void Cop1::roundld(u32 instr) {
  if (!CheckFPUUsable())
    return;
  auto fs = FGR_S<double>(regs.cop0.status, FS(instr));
  if (!CheckCVTArg<s64>(fs))
    return;
  CHECK_FPE(s64, fd, Util::roundNearest<s64>(fs))
  if (fd != fs && SetCauseInexact()) {
    regs.cop0.FireException(ExceptionCode::FloatingPointError, 0, regs.oldPC);
    return;
  }
  FGR_D<s64>(regs.cop0.status, FD(instr)) = fd;
}

void Cop1::roundws(u32 instr) {
  if (!CheckFPUUsable())
    return;
  auto fs = FGR_S<float>(regs.cop0.status, FS(instr));
  if (!CheckCVTArg<s32>(fs))
    return;
  CHECK_FPE_CONV(s32, fd, Util::roundNearest<s32>(fs))
  if (fd != fs && SetCauseInexact()) {
    regs.cop0.FireException(ExceptionCode::FloatingPointError, 0, regs.oldPC);
    return;
  }
  FGR_D<s32>(regs.cop0.status, FD(instr)) = fd;
}

void Cop1::roundwd(u32 instr) {
  if (!CheckFPUUsable())
    return;
  auto fs = FGR_S<double>(regs.cop0.status, FS(instr));
  if (!CheckCVTArg<s32>(fs))
    return;
  CHECK_FPE_CONV(s32, fd, Util::roundNearest<s32>(fs))
  if (fd != fs && SetCauseInexact()) {
    regs.cop0.FireException(ExceptionCode::FloatingPointError, 0, regs.oldPC);
    return;
  }
  FGR_D<s32>(regs.cop0.status, FD(instr)) = fd;
}

void Cop1::floorls(u32 instr) {
  if (!CheckFPUUsable())
    return;
  auto fs = FGR_S<float>(regs.cop0.status, FS(instr));
  if (!CheckCVTArg<s64>(fs))
    return;
  CHECK_FPE(s64, fd, Util::roundFloor<s64>(fs))
  FGR_D<s64>(regs.cop0.status, FD(instr)) = fd;
}

void Cop1::floorld(u32 instr) {
  if (!CheckFPUUsable())
    return;
  auto fs = FGR_S<double>(regs.cop0.status, FS(instr));
  if (!CheckCVTArg<s64>(fs))
    return;
  CHECK_FPE(s64, fd, Util::roundFloor<s64>(fs))
  FGR_D<s64>(regs.cop0.status, FD(instr)) = fd;
}

void Cop1::floorws(u32 instr) {
  if (!CheckFPUUsable())
    return;
  auto fs = FGR_S<float>(regs.cop0.status, FS(instr));
  if (!CheckCVTArg<s32>(fs))
    return;
  CHECK_FPE_CONV(s32, fd, Util::roundFloor<s32>(fs))
  FGR_D<s32>(regs.cop0.status, FD(instr)) = fd;
}

void Cop1::floorwd(u32 instr) {
  if (!CheckFPUUsable())
    return;
  auto fs = FGR_S<double>(regs.cop0.status, FS(instr));
  if (!CheckCVTArg<s32>(fs))
    return;
  CHECK_FPE_CONV(s32, fd, Util::roundFloor<s32>(fs))
  FGR_D<s32>(regs.cop0.status, FD(instr)) = fd;
}

void Cop1::truncws(u32 instr) {
  if (!CheckFPUUsable())
    return;
  auto fs = FGR_S<float>(regs.cop0.status, FS(instr));
  if (!CheckCVTArg<s32>(fs))
    return;
  CHECK_FPE_CONV(s32, fd, Util::roundTrunc<s32>(fs))
  if ((float)fd != fs && SetCauseInexact()) {
    regs.cop0.FireException(ExceptionCode::FloatingPointError, 0, regs.oldPC);
    return;
  }
  FGR_D<s32>(regs.cop0.status, FD(instr)) = fd;
}

void Cop1::truncwd(u32 instr) {
  if (!CheckFPUUsable())
    return;
  auto fs = FGR_S<double>(regs.cop0.status, FS(instr));
  if (!CheckCVTArg<s32>(fs))
    return;
  CHECK_FPE_CONV(s32, fd, Util::roundTrunc<s32>(fs))
  if ((double)fd != fs && SetCauseInexact()) {
    regs.cop0.FireException(ExceptionCode::FloatingPointError, 0, regs.oldPC);
    return;
  }
  FGR_D<s32>(regs.cop0.status, FD(instr)) = fd;
}

void Cop1::truncls(u32 instr) {
  if (!CheckFPUUsable())
    return;
  auto fs = FGR_S<float>(regs.cop0.status, FS(instr));
  if (!CheckCVTArg<s64>(fs))
    return;
  CHECK_FPE(s64, fd, Util::roundTrunc<s64>(fs))
  if ((float)fd != fs && SetCauseInexact()) {
    regs.cop0.FireException(ExceptionCode::FloatingPointError, 0, regs.oldPC);
    return;
  }
  FGR_D<s64>(regs.cop0.status, FD(instr)) = fd;
}

void Cop1::truncld(u32 instr) {
  if (!CheckFPUUsable())
    return;
  auto fs = FGR_S<double>(regs.cop0.status, FS(instr));
  if (!CheckCVTArg<s64>(fs))
    return;
  CHECK_FPE(s64, fd, Util::roundTrunc<s64>(fs))
  if ((double)fd != fs && SetCauseInexact()) {
    regs.cop0.FireException(ExceptionCode::FloatingPointError, 0, regs.oldPC);
    return;
  }
  FGR_D<s64>(regs.cop0.status, FD(instr)) = fd;
}

template <class T>
void Cop1::lwc1(T &cpu, Mem &mem, u32 instr) {
  if constexpr (std::is_same_v<decltype(cpu), Interpreter &>) {
    if (!CheckFPUUsable<true>())
      return;
    lwc1Interp(mem, instr);
  } else if constexpr (std::is_same_v<decltype(cpu), JIT &>) {
    lwc1JIT(cpu, mem, instr);
  } else {
    Util::panic("What the fuck did you just give me?!!");
  }
}

template void Cop1::lwc1<Interpreter>(Interpreter &, Mem &, u32);
template void Cop1::lwc1<JIT>(JIT &, Mem &, u32);

template <class T>
void Cop1::swc1(T &cpu, Mem &mem, u32 instr) {
  if constexpr (std::is_same_v<decltype(cpu), Interpreter &>) {
    if (!CheckFPUUsable<true>())
      return;
    swc1Interp(mem, instr);
  } else if constexpr (std::is_same_v<decltype(cpu), JIT &>) {
    swc1JIT(cpu, mem, instr);
  } else {
    Util::panic("What the fuck did you just give me?!!");
  }
}

template void Cop1::swc1<Interpreter>(Interpreter &, Mem &, u32);
template void Cop1::swc1<JIT>(JIT &, Mem &, u32);

template <class T>
void Cop1::ldc1(T &cpu, Mem &mem, u32 instr) {
  if constexpr (std::is_same_v<decltype(cpu), Interpreter &>) {
    if (!CheckFPUUsable<true>())
      return;
    ldc1Interp(mem, instr);
  } else if constexpr (std::is_same_v<decltype(cpu), JIT &>) {
    ldc1JIT(cpu, mem, instr);
  } else {
    Util::panic("What the fuck did you just give me?!!");
  }
}

template void Cop1::ldc1<Interpreter>(Interpreter &, Mem &, u32);
template void Cop1::ldc1<JIT>(JIT &, Mem &, u32);

template <class T>
void Cop1::sdc1(T &cpu, Mem &mem, u32 instr) {
  if constexpr (std::is_same_v<decltype(cpu), Interpreter &>) {
    if (!CheckFPUUsable<true>())
      return;
    sdc1Interp(mem, instr);
  } else if constexpr (std::is_same_v<decltype(cpu), JIT &>) {
    sdc1JIT(cpu, mem, instr);
  } else {
    Util::panic("What the fuck did you just give me?!!");
  }
}

template void Cop1::sdc1<Interpreter>(Interpreter &, Mem &, u32);
template void Cop1::sdc1<JIT>(JIT &, Mem &, u32);

void Cop1::lwc1Interp(Mem &mem, u32 instr) {
  u64 addr = (s64)(s16)instr + regs.Read<s64>(BASE(instr));

  u32 physical;
  if (!regs.cop0.MapVAddr(Cop0::LOAD, addr, physical)) {
    regs.cop0.HandleTLBException(addr);
    regs.cop0.FireException(regs.cop0.GetTLBExceptionCode(regs.cop0.tlbError, Cop0::LOAD), 0, regs.oldPC);
  } else {
    u32 data = mem.Read<u32>(regs, physical);
    FGR_T<u32>(regs.cop0.status, FT(instr)) = data;
  }
}

void Cop1::swc1Interp(Mem &mem, u32 instr) {
  u64 addr = (s64)(s16)instr + regs.Read<s64>(BASE(instr));

  u32 physical;
  if (!regs.cop0.MapVAddr(Cop0::STORE, addr, physical)) {
    regs.cop0.HandleTLBException(addr);
    regs.cop0.FireException(regs.cop0.GetTLBExceptionCode(regs.cop0.tlbError, Cop0::STORE), 0, regs.oldPC);
  } else {
    mem.Write<u32>(regs, physical, FGR_T<u32>(regs.cop0.status, FT(instr)));
  }
}

void Cop1::unimplemented() {
  if (!CheckFPUUsable())
    return;
  SetCauseUnimplemented();
  regs.cop0.FireException(ExceptionCode::FloatingPointError, 0, regs.oldPC);
}

void Cop1::ldc1Interp(Mem &mem, u32 instr) {
  u64 addr = (s64)(s16)instr + regs.Read<s64>(BASE(instr));

  u32 physical;
  if (!regs.cop0.MapVAddr(Cop0::LOAD, addr, physical)) {
    regs.cop0.HandleTLBException(addr);
    regs.cop0.FireException(regs.cop0.GetTLBExceptionCode(regs.cop0.tlbError, Cop0::LOAD), 0, regs.oldPC);
  } else {
    u64 data = mem.Read<u64>(regs, physical);
    FGR_T<u64>(regs.cop0.status, FT(instr)) = data;
  }
}

void Cop1::sdc1Interp(Mem &mem, u32 instr) {
  u64 addr = (s64)(s16)instr + regs.Read<s64>(BASE(instr));

  u32 physical;
  if (!regs.cop0.MapVAddr(Cop0::STORE, addr, physical)) {
    regs.cop0.HandleTLBException(addr);
    regs.cop0.FireException(regs.cop0.GetTLBExceptionCode(regs.cop0.tlbError, Cop0::STORE), 0, regs.oldPC);
  } else {
    mem.Write(regs, physical, FGR_T<u64>(regs.cop0.status, FT(instr)));
  }
}

void Cop1::mfc1(u32 instr) {
  if (!CheckFPUUsable<true>())
    return;
  regs.Write(RT(instr), FGR_T<s32>(regs.cop0.status, FS(instr)));
}

void Cop1::dmfc1(u32 instr) {
  if (!CheckFPUUsable<true>())
    return;
  regs.Write(RT(instr), FGR_S<s64>(regs.cop0.status, FS(instr)));
}

void Cop1::mtc1(u32 instr) {
  if (!CheckFPUUsable<true>())
    return;
  FGR_T<s32>(regs.cop0.status, FS(instr)) = regs.Read<s64>(RT(instr));
}

void Cop1::dmtc1(u32 instr) {
  if (!CheckFPUUsable<true>())
    return;
  FGR_S<u64>(regs.cop0.status, FS(instr)) = regs.Read<s64>(RT(instr));
}
} // namespace n64
