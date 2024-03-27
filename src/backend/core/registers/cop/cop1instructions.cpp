#include <core/registers/Cop1.hpp>
#include <core/registers/Registers.hpp>
#include <core/Interpreter.hpp>
#include <core/Mem.hpp>
#include <Float.hpp>

namespace n64 {
template <bool update_flags = true>
bool Cop1::UpdateCause(u8 cause) {
  fcr31.cause |= cause;
  bool unimplemented = cause & 0x20;
  if (unimplemented || (cause & fcr31.enable)) {
    fesetround(system_rounding);
    FireException(regs, ExceptionCode::FloatingPointError, 0, regs.oldPC);
    return false;
  } else {
    if constexpr (update_flags) {
      fcr31.flag = cause & 0x1f;
    }

    return true;
  }
}

template <AnyFloat Float, bool check_inf>
bool Cop1::CheckInput(Float value) {
  if (value.is_nan()) {
    auto signaling = value.is_signaling();
    if (signaling) {
      return UpdateCause(FpeUnimplemented);
    } else {
      if constexpr (check_inf) {
        return UpdateCause(FpeUnimplemented);
      } else {
        return UpdateCause(FpeInvalid);
      }
    }
  } else if (value.is_subnormal()) {
    return UpdateCause(FpeUnimplemented);
  } else if (check_inf && value.is_infinite()) {
    return UpdateCause(FpeUnimplemented);
  }

  return true;
}

template <typename ...Args, bool check_inf>
bool Cop1::CheckInput(Args ...values) {
  return (CheckInput(values), ...);
}

void Cop1::BeginOp() {
  int round_mode = fcr31.rounding_mode;
  switch (round_mode) {
  case 0b00: round_mode = FE_TONEAREST; break;
  case 0b01: round_mode = FE_TOWARDZERO; break;
  case 0b10: round_mode = FE_UPWARD; break;
  case 0b11: round_mode = FE_DOWNWARD; break;
  }

  std::fesetround(round_mode);
  std::feclearexcept(FE_ALL_EXCEPT);

  fcr31.cause &= 0x20;
}

Cop1::CheckResult Cop1::CheckExceptions() {
  u8 cause;
  auto exceptions = std::fetestexcept(FE_ALL_EXCEPT);

  if(exceptions & FE_INEXACT) {
    cause |= FpeInexact;
  }

  if(exceptions & FE_INVALID) {
    cause |= FpeInvalid;
  }

  if(exceptions & FE_OVERFLOW) {
    cause |= FpeOverflow;
  }

  if(exceptions & FE_DIVBYZERO) {
    cause |= FpeDivByZero;
  }

  bool is_underflow = (exceptions & FE_UNDERFLOW);

  return {cause, is_underflow};
}

template <typename T, AnyFloat Float>
T Cop1::EndCVTOp(Float f) {
  auto [cause, _] = CheckExceptions();
  auto value = (s64)f.to_bits();

  auto min_value = 0.0;
  auto max_value = 0.0;

  if constexpr (std::is_same_v<T, u64>) {
    min_value = f.to_bits(((53 + 1023) << 52) | (1 << 63));
    max_value = f.to_bits((53 + 1023) << 52);

    if (f >= max_value || f <= min_value) {
      cause = FpeUnimplemented;
      value = (2 << 63) - 1;
    }
  } else {
    if (value < -(1 << 31) || value > ((1 << 31) - 1)) {
      cause = FpeUnimplemented;
      value = (2 << 31) - 1;
    }
  }

  if (cause) {
    UpdateCause<true>(cause);
  }

  std::fesetround(system_rounding);

  return value;
}

template <AnyFloat Float>
Float Cop1::EndOp(Float f) {
  Float retval = f;
  auto [cause, is_underflow] = CheckExceptions();
  if(f.is_nan()) {
    retval = f.nan();
  } else if(f.is_subnormal() || is_underflow) {
    if(!fcr31.fs || fcr31.enable_underflow || fcr31.enable_inexact_operation) {
      cause = FpeUnimplemented;
    } else {
      cause = FpeInexact | FpeUnderflow;
      fcr31.cause |= cause;
      switch(fcr31.rounding_mode) {
        case 0: case 1:
          if(!f.is_neg()) {
            retval = 0.0;
          } else {
            retval = -0.0;
          }
          break;
        case 2:
          if(!f.is_neg()) {
            retval = f.from_bits(0x00800000);
          } else {
            retval = -0.0;
          }
          break;
        case 3:
          if(!f.is_neg()) {
            retval = 0.0;
          } else {
            retval = -f.from_bits(0x00800000);
          }
          break;
      }
    }
  }

  if(cause) {
    if(!UpdateCause<true>(cause)) return retval;
  }

  if(fcr31.cause & FpeInvalid) {
    retval = f.nan();
  }

  std::fesetround(system_rounding);

  return retval;
}

template <AnyFloat Float, typename F, typename ...Args>
void Cop1::DoCVTOp(u32 instr, F op, Args... args) {
  BeginOp();
  if (!CheckInput<true>(args...)) return;
  auto result = EndCVTOp(op(args...));
  if constexpr (std::is_same_v<Float, f64>) {
    fgr[FD(instr)].as_f64 = result;
  }
  else if (std::is_same_v<Float, f32>) {
    fgr[FD(instr)].as_f32 = result;
  }
}

template <AnyFloat Float, typename F, typename ...Args>
void Cop1::DoOp(u32 instr, F op, Args... args) {
  BeginOp();
  if(!CheckInput(args...)) return;
  auto result = EndOp(op(args...));
  if constexpr (std::is_same_v<Float, f64>) {
    fgr[FD(instr)].as_f64 = result;
  } else if(std::is_same_v<Float, f32>) {
    fgr[FD(instr)].as_f32 = result;
  }
}

void Cop1::absd(u32 instr) {
  DoOp<f64>(instr, [](f64 f) { return f64(std::abs(f.operator double())); }, fgr[FS(instr)].as_f64);
}

void Cop1::abss(u32 instr) {
  DoOp<f32>(instr, [](f32 f) { return f32(std::abs(f.operator float())); }, fgr[FS(instr)].as_f32);
}

void Cop1::adds(u32 instr) {
  DoOp<f32>(
    instr,
    [](f32 p, f32 t) { return p + t; },
    fgr[FS(instr)].as_f32,
    fgr[FT(instr)].as_f32
  );
}

void Cop1::addd(u32 instr) {
  DoOp<f64>(
    instr,
    [](f64 p, f64 t) { return p + t; },
    fgr[FS(instr)].as_f64,
    fgr[FT(instr)].as_f64
  );
}

void Cop1::ceills(u32 instr) {
  std::fesetround(FE_UPWARD);
  DoCVTOp<f64>(
    instr,
    [](f64 f) { return f64(std::rint(f.operator double())); },
    f64(fgr[FS(instr)].as_f32.operator float())
  );
}

void Cop1::ceilws(u32 instr) {
  std::fesetround(FE_UPWARD);
  DoCVTOp<f64>(
    instr,
    [](f64 f) { return f64(std::rint(f.operator double())); },
    f64(fgr[FS(instr)].as_f32.operator float())
  );
}

void Cop1::ceilld(u32 instr) {
  std::fesetround(FE_UPWARD);
  DoCVTOp<f64>(
    instr,
    [](f64 f) { return f64(std::rint(f.operator double())); },
    fgr[FS(instr)].as_f64
  );
}

void Cop1::ceilwd(u32 instr) {
  std::fesetround(FE_UPWARD);
  DoCVTOp<f64>(
    instr,
    [](f64 f) { return f64(std::rint(f.operator double())); },
    fgr[FS(instr)].as_f64
  );
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
  auto fs = FGR<f32>(regs.cop0.status, FS(instr));
  CheckArg(fs);
  f64 result;
  OP_CheckExcept({ result = double(fs.operator float()); });
  CheckResult(result);
  FGR<f64>(regs.cop0.status, FD(instr)) = result;
}

void Cop1::cvtsd(u32 instr) {
  CheckFPUUsable();
  auto fs = FGR<f64>(regs.cop0.status, FS(instr));
  CheckArg(fs);
  f32 result;
  OP_CheckExcept({ result = float(fs.operator double()); });
  CheckResult(result);
  FGR<f32>(regs.cop0.status, FD(instr)) = result;
}

void Cop1::cvtsw(u32 instr) {
  CheckFPUUsable();
  auto fs = FGR<s32>(regs.cop0.status, FS(instr));
  f32 result;
  OP_CheckExcept({ result = float(fs); });
  CheckResult(result);
  FGR<f32>(regs.cop0.status, FD(instr)) = result;
}

void Cop1::cvtsl(u32 instr) {
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

void Cop1::cvtwd(u32 instr) {
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

void Cop1::cvtws(u32 instr) {
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

void Cop1::cvtls(u32 instr) {
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

void Cop1::cvtdw(u32 instr) {
  CheckFPUUsable();
  auto fs = FGR<s32>(regs.cop0.status, FS(instr));
  f64 result;
  OP_CheckExcept({ result = double(fs); });
  CheckResult(result);
  FGR<f64>(regs.cop0.status, FD(instr)) = result;
}

void Cop1::cvtdl(u32 instr) {
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

void Cop1::cvtld(u32 instr) {
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
void Cop1::cf(u32 instr) {
  CheckFPUUsable();
  T fs = FGR<T>(regs.cop0.status, FS(instr));
  T ft = FGR<T>(regs.cop0.status, FT(instr));
  checkqnanregs(fs, ft);
  fcr31.compare = false;
}

template <typename T>
void Cop1::cun(u32 instr) {
  CheckFPUUsable();
  T fs = FGR<T>(regs.cop0.status, FS(instr));
  T ft = FGR<T>(regs.cop0.status, FT(instr));
  checkqnanregs(fs, ft);
  fcr31.compare = any_unordered(fs, ft);
}

template <typename T>
void Cop1::ceq(u32 instr) {
  CheckFPUUsable();
  T fs = FGR<T>(regs.cop0.status, FS(instr));
  T ft = FGR<T>(regs.cop0.status, FT(instr));
  checkqnanregs(fs, ft);
  fcr31.compare = fs == ft;
}

template <typename T>
void Cop1::cueq(u32 instr) {
  CheckFPUUsable();
  T fs = FGR<T>(regs.cop0.status, FS(instr));
  T ft = FGR<T>(regs.cop0.status, FT(instr));
  checkqnanregs(fs, ft);
  fcr31.compare = fs == ft || any_unordered(fs, ft);
}

template <typename T>
void Cop1::colt(u32 instr) {
  CheckFPUUsable();
  T fs = FGR<T>(regs.cop0.status, FS(instr));
  T ft = FGR<T>(regs.cop0.status, FT(instr));
  checkqnanregs(fs, ft);
  fcr31.compare = fs < ft;
}

template <typename T>
void Cop1::cult(u32 instr) {
  CheckFPUUsable();
  T fs = FGR<T>(regs.cop0.status, FS(instr));
  T ft = FGR<T>(regs.cop0.status, FT(instr));
  checkqnanregs(fs, ft);
  fcr31.compare = fs < ft || any_unordered(fs, ft);
}

template <typename T>
void Cop1::cole(u32 instr) {
  CheckFPUUsable();
  T fs = FGR<T>(regs.cop0.status, FS(instr));
  T ft = FGR<T>(regs.cop0.status, FT(instr));
  checkqnanregs(fs, ft);
  fcr31.compare = fs <= ft;
}

template <typename T>
void Cop1::cule(u32 instr) {
  CheckFPUUsable();
  T fs = FGR<T>(regs.cop0.status, FS(instr));
  T ft = FGR<T>(regs.cop0.status, FT(instr));
  checkqnanregs(fs, ft);
  fcr31.compare = fs <= ft || any_unordered(fs, ft);
}

template <typename T>
void Cop1::csf(u32 instr) {
  CheckFPUUsable();
  T fs = FGR<T>(regs.cop0.status, FS(instr));
  T ft = FGR<T>(regs.cop0.status, FT(instr));
  checknanregs(fs, ft);
  fcr31.compare = false;
}

template <typename T>
void Cop1::cngle(u32 instr) {
  CheckFPUUsable();
  T fs = FGR<T>(regs.cop0.status, FS(instr));
  T ft = FGR<T>(regs.cop0.status, FT(instr));
  checknanregs(fs, ft);
  fcr31.compare = any_unordered(fs, ft);
}

template <typename T>
void Cop1::cseq(u32 instr) {
  CheckFPUUsable();
  T fs = FGR<T>(regs.cop0.status, FS(instr));
  T ft = FGR<T>(regs.cop0.status, FT(instr));
  checknanregs(fs, ft);
  fcr31.compare = fs == ft;
}

template <typename T>
void Cop1::cngl(u32 instr) {
  CheckFPUUsable();
  T fs = FGR<T>(regs.cop0.status, FS(instr));
  T ft = FGR<T>(regs.cop0.status, FT(instr));
  checknanregs(fs, ft);
  fcr31.compare = fs == ft || any_unordered(fs, ft);
}

template <typename T>
void Cop1::clt(u32 instr) {
  CheckFPUUsable();
  T fs = FGR<T>(regs.cop0.status, FS(instr));
  T ft = FGR<T>(regs.cop0.status, FT(instr));
  checknanregs(fs, ft);
  fcr31.compare = fs < ft;
}

template <typename T>
void Cop1::cnge(u32 instr) {
  CheckFPUUsable();
  T fs = FGR<T>(regs.cop0.status, FS(instr));
  T ft = FGR<T>(regs.cop0.status, FT(instr));
  checknanregs(fs, ft);
  fcr31.compare = fs < ft || any_unordered(fs, ft);
}

template <typename T>
void Cop1::cle(u32 instr) {
  CheckFPUUsable();
  T fs = FGR<T>(regs.cop0.status, FS(instr));
  T ft = FGR<T>(regs.cop0.status, FT(instr));
  checknanregs(fs, ft);
  fcr31.compare = fs <= ft;
}

template <typename T>
void Cop1::cngt(u32 instr) {
  CheckFPUUsable();
  T fs = FGR<T>(regs.cop0.status, FS(instr));
  T ft = FGR<T>(regs.cop0.status, FT(instr));
  checknanregs(fs, ft);
  fcr31.compare = fs <= ft || any_unordered(fs, ft);
}

template void Cop1::cf<f32>(u32 instr);
template void Cop1::cun<f32>(u32 instr);
template void Cop1::ceq<f32>(u32 instr);
template void Cop1::cueq<f32>(u32 instr);
template void Cop1::colt<f32>(u32 instr);
template void Cop1::cult<f32>(u32 instr);
template void Cop1::cole<f32>(u32 instr);
template void Cop1::cule<f32>(u32 instr);
template void Cop1::csf<f32>(u32 instr);
template void Cop1::cngle<f32>(u32 instr);
template void Cop1::cseq<f32>(u32 instr);
template void Cop1::cngl<f32>(u32 instr);
template void Cop1::clt<f32>(u32 instr);
template void Cop1::cnge<f32>(u32 instr);
template void Cop1::cle<f32>(u32 instr);
template void Cop1::cngt<f32>(u32 instr);
template void Cop1::cf<f64>(u32 instr);
template void Cop1::cun<f64>(u32 instr);
template void Cop1::ceq<f64>(u32 instr);
template void Cop1::cueq<f64>(u32 instr);
template void Cop1::colt<f64>(u32 instr);
template void Cop1::cult<f64>(u32 instr);
template void Cop1::cole<f64>(u32 instr);
template void Cop1::cule<f64>(u32 instr);
template void Cop1::csf<f64>(u32 instr);
template void Cop1::cngle<f64>(u32 instr);
template void Cop1::cseq<f64>(u32 instr);
template void Cop1::cngl<f64>(u32 instr);
template void Cop1::clt<f64>(u32 instr);
template void Cop1::cnge<f64>(u32 instr);
template void Cop1::cle<f64>(u32 instr);
template void Cop1::cngt<f64>(u32 instr);

void Cop1::divs(u32 instr) {
  OP(f32, fs / ft);
}

void Cop1::divd(u32 instr) {
  OP(f64, fs / ft);
}

void Cop1::muls(u32 instr) {
  OP(f32, fs * ft);
}

void Cop1::muld(u32 instr) {
  OP(f64, fs * ft);
}

void Cop1::subs(u32 instr) {
  OP(f32, fs - ft);
}

void Cop1::subd(u32 instr) {
  OP(f64, fs - ft);
}

void Cop1::movs(u32 instr) {
  CheckFPUUsable_PreserveCause();
  auto val = FGR<u64>(regs.cop0.status, FS(instr));
  FGR<u64>(regs.cop0.status, FD(instr)) = val;
}

void Cop1::movd(u32 instr) {
  CheckFPUUsable_PreserveCause();
  auto val = FGR<f64>(regs.cop0.status, FS(instr));
  FGR<f64>(regs.cop0.status, FD(instr)) = val;
}

void Cop1::negs(u32 instr) {
  OP(f32, -fs.operator float());
}

void Cop1::negd(u32 instr) {
  OP(f64, -fs.operator double());
}

void Cop1::sqrts(u32 instr) {
  OP(f32, std::sqrt(fs.operator float()));
}

void Cop1::sqrtd(u32 instr) {
  OP(f64, std::sqrt(fs.operator double()));
}

void Cop1::roundls(u32 instr) {
  CheckFPUUsable();
  auto fs = FGR<f32>(regs.cop0.status, FS(instr));
  CheckLCVTArg(fs);
  s64 result;
  CVT_OP_CheckExcept({ result = std::nearbyint(fs.operator float()); });
  CheckRound(fs.to_bits(), result);
  FGR<s64>(regs.cop0.status, FD(instr)) = result;
}

void Cop1::roundld(u32 instr) {
  CheckFPUUsable();
  auto fs = FGR<f64>(regs.cop0.status, FS(instr));
  CheckLCVTArg(fs);
  s64 result;
  CVT_OP_CheckExcept({ result = std::nearbyint(fs.operator double()); });
  CheckRound(fs.to_bits(), result);
  FGR<s64>(regs.cop0.status, FD(instr)) = result;
}

void Cop1::roundws(u32 instr) {
  CheckFPUUsable();
  auto fs = FGR<f32>(regs.cop0.status, FS(instr));
  CheckWCVTArg(fs);
  s32 result;
  CVT_OP_CheckExcept({ result = std::nearbyint(fs.operator float()); });
  CheckRound(fs.to_bits(), result);
  FGR<s32>(regs.cop0.status, FD(instr)) = result;
}

void Cop1::roundwd(u32 instr) {
  CheckFPUUsable();
  auto fs = FGR<f64>(regs.cop0.status, FS(instr));
  CheckWCVTArg(fs);
  s32 result;
  CVT_OP_CheckExcept({ result = std::nearbyint(fs.operator double()); });
  CheckRound(fs.to_bits(), result);
  FGR<s32>(regs.cop0.status, FD(instr)) = result;
}

void Cop1::floorls(u32 instr) {
  CheckFPUUsable();
  auto fs = FGR<f32>(regs.cop0.status, FS(instr));
  CheckLCVTArg(fs);
  s64 result;
  CVT_OP_CheckExcept({ result = std::floor(fs.operator float()); });
  CheckRound(fs.to_bits(), result);
  FGR<s64>(regs.cop0.status, FD(instr)) = result;
}

void Cop1::floorld(u32 instr) {
  CheckFPUUsable();
  auto fs = FGR<f64>(regs.cop0.status, FS(instr));
  CheckLCVTArg(fs);
  s64 result;
  CVT_OP_CheckExcept({ result = std::floor(fs.operator double()); });
  CheckRound(fs.to_bits(), result);
  FGR<s64>(regs.cop0.status, FD(instr)) = result;
}

void Cop1::floorws(u32 instr) {
  CheckFPUUsable();
  auto fs = FGR<f32>(regs.cop0.status, FS(instr));
  CheckWCVTArg(fs);
  s32 result;
  CVT_OP_CheckExcept({ result = std::floor(fs.operator float()); });
  CheckRound(fs.to_bits(), result);
  FGR<s32>(regs.cop0.status, FD(instr)) = result;
}

void Cop1::floorwd(u32 instr) {
  CheckFPUUsable();
  auto fs = FGR<f64>(regs.cop0.status, FS(instr));
  CheckWCVTArg(fs);
  s32 result;
  CVT_OP_CheckExcept({ result = std::floor(fs.operator double()); });
  CheckRound(fs.to_bits(), result);
  FGR<s32>(regs.cop0.status, FD(instr)) = result;
}

void Cop1::truncws(u32 instr) {
  CheckFPUUsable();
  auto fs = FGR<f32>(regs.cop0.status, FS(instr));
  CheckWCVTArg(fs);
  s32 result;
  CVT_OP_CheckExcept({ result = std::trunc(fs.operator float()); });
  CheckRound(fs.to_bits(), result);
  FGR<s32>(regs.cop0.status, FD(instr)) = result;
}

void Cop1::truncwd(u32 instr) {
  CheckFPUUsable();
  auto fs = FGR<f64>(regs.cop0.status, FS(instr));
  CheckWCVTArg(fs);
  s32 result;
  CVT_OP_CheckExcept({ result = std::trunc(fs.operator double()); });
  CheckRound(fs.to_bits(), result);
  FGR<s32>(regs.cop0.status, FD(instr)) = result;
}

void Cop1::truncls(u32 instr) {
  CheckFPUUsable();
  auto fs = FGR<f32>(regs.cop0.status, FS(instr));
  CheckLCVTArg(fs);
  s64 result;
  CVT_OP_CheckExcept({ result = std::trunc(fs.operator float()); });
  CheckRound(fs.to_bits(), result);
  FGR<s64>(regs.cop0.status, FD(instr)) = result;
}

void Cop1::truncld(u32 instr) {
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
  if(!MapVAddr(regs, LOAD, addr, physical)) {
    HandleTLBException(regs, addr);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, LOAD), 0, regs.oldPC);
  } else {
    u32 data = mem.Read<u32>(regs, physical);
    FGR<u32>(regs.cop0.status, FT(instr)) = data;
  }
}

void Cop1::swc1Interp(Mem& mem, u32 instr) {
  u64 addr = (s64)(s16)instr + regs.gpr[BASE(instr)];

  u32 physical;
  if(!MapVAddr(regs, STORE, addr, physical)) {
    HandleTLBException(regs, addr);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, STORE), 0, regs.oldPC);
  } else {
    mem.Write<u32>(regs, physical, FGR<u32>(regs.cop0.status, FT(instr)));
  }
}

void Cop1::unimplemented() {
  fcr31.cause_unimplemented_operation = true;
  fcr31.cause &= ~0x1f;
  UpdateCause(0b100000);
}

void Cop1::ldc1Interp(Mem& mem, u32 instr) {
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

void Cop1::sdc1Interp(Mem& mem, u32 instr) {
  u64 addr = (s64)(s16)instr + regs.gpr[BASE(instr)];

  u32 physical;
  if(!MapVAddr(regs, STORE, addr, physical)) {
    HandleTLBException(regs, addr);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, STORE), 0, regs.oldPC);
  } else {
    mem.Write(regs, physical, FGR<u64>(regs.cop0.status, FT(instr)));
  }
}

void Cop1::mfc1(u32 instr) {
  CheckFPUUsable_PreserveCause();
  regs.gpr[RT(instr)] = FGR<s32>(regs.cop0.status, FS(instr));
}

void Cop1::dmfc1(u32 instr) {
  CheckFPUUsable_PreserveCause();
  regs.gpr[RT(instr)] = FGR<s64>(regs.cop0.status, FS(instr));
}

void Cop1::mtc1(u32 instr) {
  CheckFPUUsable_PreserveCause();
  FGR<u32>(regs.cop0.status, FS(instr)) = regs.gpr[RT(instr)];
}

void Cop1::dmtc1(u32 instr) {
  CheckFPUUsable_PreserveCause();
  FGR<u64>(regs.cop0.status, FS(instr)) = regs.gpr[RT(instr)];
}
}
