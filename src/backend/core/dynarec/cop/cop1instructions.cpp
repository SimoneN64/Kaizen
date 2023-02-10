#include <dynarec/cop/cop1instructions.hpp>
#include <cfenv>
#include <cmath>
#include <Cop1.hpp>
#include <Registers.hpp>

namespace n64::JIT {
inline int PushRoundingMode(const FCR31& fcr31) {
  int og = fegetround();
  switch(fcr31.rounding_mode) {
    case 0: fesetround(FE_TONEAREST); break;
    case 1: fesetround(FE_TOWARDZERO); break;
    case 2: fesetround(FE_UPWARD); break;
    case 3: fesetround(FE_DOWNWARD); break;
  }

  return og;
}

#define PUSHROUNDINGMODE int og = PushRoundingMode(regs.cop1.fcr31)
#define POPROUNDINGMODE fesetround(og)

#define checknanregs(fs, ft) do { \
  if(std::isnan(fs) || std::isnan(ft)) { \
    regs.cop1.fcr31.flag_invalid_operation = true; \
    regs.cop1.fcr31.cause_invalid_operation = true; \
    FireException(regs, ExceptionCode::FloatingPointError, 1, true); \
    return; \
  }                               \
} while(0)

void absd(n64::Registers& regs, u32 instr) {
  double fs = regs.cop1.GetCop1Reg<double>(regs.cop0, FS(instr));
  regs.cop1.SetCop1Reg<double>(regs.cop0, FD(instr), std::abs(fs));
}

void abss(n64::Registers& regs, u32 instr) {
  float fs = regs.cop1.GetCop1Reg<float>(regs.cop0, FS(instr));
  regs.cop1.SetCop1Reg<float>(regs.cop0, FD(instr), std::abs(fs));
}

void absw(n64::Registers& regs, u32 instr) {
  s32 fs = regs.cop1.GetReg<s32>(regs.cop0, FS(instr));
  regs.cop1.SetReg<u32>(regs.cop0, FD(instr), std::abs(fs));
}

void absl(n64::Registers& regs, u32 instr) {
  s64 fs = regs.cop1.GetReg<s64>(regs.cop0, FS(instr));
  regs.cop1.SetReg(regs.cop0, FD(instr), std::abs(fs));
}

void adds(n64::Registers& regs, u32 instr) {
  float fs = regs.cop1.GetCop1Reg<float>(regs.cop0, FS(instr));
  float ft = regs.cop1.GetCop1Reg<float>(regs.cop0, FT(instr));
  checknanregs(fs, ft);
  float result = fs + ft;
  regs.cop1.SetCop1Reg<float>(regs.cop0, FD(instr), result);
}

void addd(n64::Registers& regs, u32 instr) {
  double fs = regs.cop1.GetCop1Reg<double>(regs.cop0, FS(instr));
  double ft = regs.cop1.GetCop1Reg<double>(regs.cop0, FT(instr));
  checknanregs(fs, ft);
  double result = fs + ft;
  regs.cop1.SetCop1Reg<double>(regs.cop0, FD(instr), result);
}

void ceills(n64::Registers& regs, u32 instr) {
  float fs = regs.cop1.GetCop1Reg<float>(regs.cop0, FS(instr));
  s64 result = std::ceil(fs);
  regs.cop1.SetReg<u64>(regs.cop0, FD(instr), result);
}

void ceilws(n64::Registers& regs, u32 instr) {
  float fs = regs.cop1.GetCop1Reg<float>(regs.cop0, FS(instr));
  s32 result = std::ceil(fs);
  regs.cop1.SetReg<u32>(regs.cop0, FD(instr), result);
}

void ceilld(n64::Registers& regs, u32 instr) {
  double fs = regs.cop1.GetCop1Reg<double>(regs.cop0, FS(instr));
  s64 result = std::ceil(fs);
  regs.cop1.SetReg<u64>(regs.cop0, FD(instr), result);
}

void ceilwd(n64::Registers& regs, u32 instr) {
  double fs = regs.cop1.GetCop1Reg<double>(regs.cop0, FS(instr));
  s32 result = std::ceil(fs);
  regs.cop1.SetReg<u32>(regs.cop0, FD(instr), result);
}

void cfc1(n64::Registers& regs, u32 instr) {
  u8 fd = RD(instr);
  s32 val = 0;
  switch(fd) {
    case 0: val = regs.cop1.fcr0; break;
    case 31: val = regs.cop1.fcr31.raw; break;
    default: Util::panic("Undefined CFC1 with rd != 0 or 31\n");
  }
  regs.gpr[RT(instr)] = val;
}

void ctc1(n64::Registers& regs, u32 instr) {
  u8 fs = FS(instr);
  u32 val = regs.gpr[RT(instr)];
  switch(fs) {
    case 0: break;
    case 31: {
      val &= 0x183ffff;
      regs.cop1.fcr31.raw = val;
    } break;
    default: Util::panic("Undefined CTC1 with rd != 0 or 31\n");
  }
}

void cvtds(n64::Registers& regs, u32 instr) {
  regs.cop1.SetCop1Reg<double>(
    regs.cop0,
    FD(instr),
    regs.cop1.GetCop1Reg<float>(
      regs.cop0,
      FS(instr)
    )
  );
}

void cvtsd(n64::Registers& regs, u32 instr) {
  regs.cop1.SetCop1Reg<float>(
    regs.cop0,
    FD(instr),
    regs.cop1.GetCop1Reg<double>(
      regs.cop0,
      FS(instr)
    )
  );
}

void cvtwd(n64::Registers& regs, u32 instr) {
  regs.cop1.SetReg<u32>(
    regs.cop0,
    FD(instr),
    regs.cop1.GetCop1Reg<double>(
      regs.cop0,
      FS(instr)
    )
  );
}

void cvtws(n64::Registers& regs, u32 instr) {
  regs.cop1.SetReg<u32>(
    regs.cop0,
    FD(instr),
    regs.cop1.GetCop1Reg<float>(
      regs.cop0,
      FS(instr)
    )
  );
}

void cvtls(n64::Registers& regs, u32 instr) {
  regs.cop1.SetReg<u64>(
    regs.cop0,
    FD(instr),
    regs.cop1.GetCop1Reg<float>(
      regs.cop0,
      FS(instr)
    )
  );
}

void cvtsl(n64::Registers& regs, u32 instr) {
  regs.cop1.SetCop1Reg<float>(
    regs.cop0,
    FD(instr),
    (s64)regs.cop1.GetReg<u64>(
      regs.cop0,
      FS(instr)
    )
  );
}

void cvtdw(n64::Registers& regs, u32 instr) {
  regs.cop1.SetCop1Reg<double>(
    regs.cop0,
    FD(instr),
    (s32)regs.cop1.GetReg<u32>(
      regs.cop0,
      FS(instr)
    )
  );
}

void cvtsw(n64::Registers& regs, u32 instr) {
  regs.cop1.SetCop1Reg<float>(
    regs.cop0,
    FD(instr),
    (s32)regs.cop1.GetReg<u32>(
      regs.cop0,
      FS(instr)
    )
  );
}

void cvtdl(n64::Registers& regs, u32 instr) {
  regs.cop1.SetCop1Reg<double>(
    regs.cop0,
    FD(instr),
    (s64)regs.cop1.GetReg<u64>(
      regs.cop0,
      FS(instr)
    )
  );
}

void cvtld(n64::Registers& regs, u32 instr) {
  regs.cop1.SetReg<u64>(
    regs.cop0,
    FD(instr),
    regs.cop1.GetCop1Reg<double>(
      regs.cop0,
      FS(instr)
    )
  );
}

template <typename T>
inline bool CalculateCondition(n64::Registers& regs, T fs, T ft, CompConds cond) {
  switch(cond) {
    case F: return false;
    case UN: return std::isnan(fs) || std::isnan(ft);
    case EQ: return fs == ft;
    case UEQ: return (std::isnan(fs) || std::isnan(ft)) || (fs == ft);
    case OLT: return (!std::isnan(fs) && !std::isnan(ft)) && (fs < ft);
    case ULT: return (std::isnan(fs) || std::isnan(ft)) || (fs < ft);
    case OLE: return (!std::isnan(fs) && !std::isnan(ft)) && (fs <= ft);
    case ULE: return (std::isnan(fs) || std::isnan(ft)) || (fs <= ft);
    default:
      if(std::isnan(fs) || std::isnan(ft)) {
        regs.cop1.fcr31.flag_invalid_operation = true;
        regs.cop1.fcr31.cause_invalid_operation = true;
        FireException(regs, ExceptionCode::FloatingPointError, 1, true);
        return false;
      }

      return CalculateCondition(regs, fs, ft, static_cast<CompConds>(cond - 8));
  }
}

template <typename T>
void ccond(n64::Registers& regs, u32 instr, CompConds cond) {
  T fs = regs.cop1.GetCop1Reg<T>(regs.cop0, FS(instr));
  T ft = regs.cop1.GetCop1Reg<T>(regs.cop0, FT(instr));

  regs.cop1.fcr31.compare = CalculateCondition(regs, fs, ft, cond);
}

template void ccond<float>(n64::Registers& regs, u32 instr, CompConds cond);
template void ccond<double>(n64::Registers& regs, u32 instr, CompConds cond);

void divs(Registers &regs, u32 instr) {
  float fs = regs.cop1.GetCop1Reg<float>(regs.cop0, FS(instr));
  float ft = regs.cop1.GetCop1Reg<float>(regs.cop0, FT(instr));
  regs.cop1.SetCop1Reg<float>(regs.cop0, FD(instr), fs / ft);
}

void divd(Registers &regs, u32 instr) {
  double fs = regs.cop1.GetCop1Reg<double>(regs.cop0, FS(instr));
  double ft = regs.cop1.GetCop1Reg<double>(regs.cop0, FT(instr));
  regs.cop1.SetCop1Reg<double>(regs.cop0, FD(instr), fs / ft);
}

void muls(Registers &regs, u32 instr) {
  float fs = regs.cop1.GetCop1Reg<float>(regs.cop0, FS(instr));
  float ft = regs.cop1.GetCop1Reg<float>(regs.cop0, FT(instr));
  regs.cop1.SetCop1Reg<float>(regs.cop0, FD(instr), fs * ft);
}

void muld(n64::Registers& regs, u32 instr) {
  double fs = regs.cop1.GetCop1Reg<double>(regs.cop0, FS(instr));
  double ft = regs.cop1.GetCop1Reg<double>(regs.cop0, FT(instr));
  regs.cop1.SetCop1Reg<double>(regs.cop0, FD(instr), fs * ft);
}

void mulw(Registers &regs, u32 instr) {
  u32 fs = regs.cop1.GetReg<u32>(regs.cop0, FS(instr));
  u32 ft = regs.cop1.GetReg<u32>(regs.cop0, FT(instr));
  regs.cop1.SetReg<u32>(regs.cop0, FD(instr), fs * ft);
}

void mull(Registers &regs, u32 instr) {
  u64 fs = regs.cop1.GetReg<u64>(regs.cop0, FS(instr));
  u64 ft = regs.cop1.GetReg<u64>(regs.cop0, FT(instr));
  regs.cop1.SetReg<u64>(regs.cop0, FD(instr), fs * ft);
}

void subs(Registers &regs, u32 instr) {
  float fs = regs.cop1.GetCop1Reg<float>(regs.cop0, FS(instr));
  float ft = regs.cop1.GetCop1Reg<float>(regs.cop0, FT(instr));
  regs.cop1.SetCop1Reg<float>(regs.cop0, FD(instr), fs - ft);
}

void subd(Registers &regs, u32 instr) {
  double fs = regs.cop1.GetCop1Reg<double>(regs.cop0, FS(instr));
  double ft = regs.cop1.GetCop1Reg<double>(regs.cop0, FT(instr));
  regs.cop1.SetCop1Reg<double>(regs.cop0, FD(instr), fs - ft);
}

void subw(Registers &regs, u32 instr) {
  u32 fs = regs.cop1.GetReg<u32>(regs.cop0, FS(instr));
  u32 ft = regs.cop1.GetReg<u32>(regs.cop0, FT(instr));
  regs.cop1.SetReg<u32>(regs.cop0, FD(instr), fs - ft);
}

void subl(Registers &regs, u32 instr) {
  u64 fs = regs.cop1.GetReg<u64>(regs.cop0, FS(instr));
  u64 ft = regs.cop1.GetReg<u64>(regs.cop0, FT(instr));
  regs.cop1.SetReg<u64>(regs.cop0, FD(instr), fs - ft);
}

void movs(n64::Registers& regs, u32 instr) {
  regs.cop1.SetCop1Reg<float>(
    regs.cop0,
    FD(instr),
    regs.cop1.GetCop1Reg<float>(
      regs.cop0,
      FS(instr)
    )
  );
}

void movd(n64::Registers& regs, u32 instr) {
  regs.cop1.SetCop1Reg<double>(
    regs.cop0,
    FD(instr),
    regs.cop1.GetCop1Reg<double>(
      regs.cop0,
      FS(instr)
    )
  );
}

void movw(n64::Registers& regs, u32 instr) {
  regs.cop1.SetReg<u32>(
    regs.cop0,
    FD(instr),
    regs.cop1.GetReg<u32>(
      regs.cop0,
      FS(instr)
    )
  );
}

void movl(n64::Registers& regs, u32 instr) {
  regs.cop1.SetReg<u64>(
    regs.cop0,
    FD(instr),
    regs.cop1.GetReg<u64>(
      regs.cop0,
      FS(instr)
    )
  );
}

void negs(Registers &regs, u32 instr) {
  regs.cop1.SetCop1Reg<float>(
    regs.cop0,
    FD(instr),
    -regs.cop1.GetCop1Reg<float>(
      regs.cop0,
      FS(instr)
    )
  );
}

void negd(Registers &regs, u32 instr) {
  regs.cop1.SetCop1Reg<double>(
    regs.cop0,
    FD(instr),
    -regs.cop1.GetCop1Reg<double>(
      regs.cop0,
      FS(instr)
    )
  );
}

void sqrts(Registers &regs, u32 instr) {
  float fs = regs.cop1.GetCop1Reg<float>(regs.cop0, FS(instr));
  regs.cop1.SetCop1Reg<float>(regs.cop0, FD(instr), std::sqrt(fs));
}

void sqrtd(Registers &regs, u32 instr) {
  double fs = regs.cop1.GetCop1Reg<double>(regs.cop0, FS(instr));
  regs.cop1.SetCop1Reg<double>(regs.cop0, FD(instr), std::sqrt(fs));
}

void roundls(n64::Registers& regs, u32 instr) {
  float fs = regs.cop1.GetCop1Reg<float>(regs.cop0, FS(instr));
  PUSHROUNDINGMODE;
  regs.cop1.SetReg<u64>(regs.cop0, FD(instr), (s32)std::nearbyint(fs));
  POPROUNDINGMODE;
}

void roundld(n64::Registers& regs, u32 instr) {
  double fs = regs.cop1.GetCop1Reg<double>(regs.cop0, FS(instr));
  PUSHROUNDINGMODE;
  regs.cop1.SetReg<u64>(regs.cop0, FD(instr), (s64)std::nearbyint(fs));
  POPROUNDINGMODE;
}

void roundws(n64::Registers& regs, u32 instr) {
  float fs = regs.cop1.GetCop1Reg<float>(regs.cop0, FS(instr));
  PUSHROUNDINGMODE;
  regs.cop1.SetReg<u32>(regs.cop0, FD(instr), (s32)std::nearbyint(fs));
  POPROUNDINGMODE;
}

void roundwd(n64::Registers& regs, u32 instr) {
  double fs = regs.cop1.GetCop1Reg<double>(regs.cop0, FS(instr));
  PUSHROUNDINGMODE;
  regs.cop1.SetReg<u32>(regs.cop0, FD(instr), (s32)std::nearbyint(fs));
  POPROUNDINGMODE;
}

void floorls(n64::Registers& regs, u32 instr) {
  float fs = regs.cop1.GetCop1Reg<float>(regs.cop0, FS(instr));
  regs.cop1.SetReg<u64>(regs.cop0, FD(instr), (s64)std::floor(fs));
}

void floorld(n64::Registers& regs, u32 instr) {
  double fs = regs.cop1.GetCop1Reg<double>(regs.cop0, FS(instr));
  regs.cop1.SetReg<u64>(regs.cop0, FD(instr), (s64)std::floor(fs));
}

void floorws(n64::Registers& regs, u32 instr) {
  float fs = regs.cop1.GetCop1Reg<float>(regs.cop0, FS(instr));
  regs.cop1.SetReg<u32>(regs.cop0, FD(instr), (s64)std::floor(fs));
}

void floorwd(n64::Registers& regs, u32 instr) {
  double fs = regs.cop1.GetCop1Reg<double>(regs.cop0, FS(instr));
  regs.cop1.SetReg<u32>(regs.cop0, FD(instr), (s64)std::floor(fs));
}

void lwc1(n64::Registers& regs, Mem& mem, u32 instr) {
  if(!regs.cop0.status.cu1) {
    FireException(regs, ExceptionCode::CoprocessorUnusable, 1, true);
    return;
  }

  u64 addr = (s64)(s16)instr + regs.gpr[BASE(instr)];
  if(addr & 3) {
    FireException(regs, ExceptionCode::AddressErrorLoad, 0, true);
  }

  u32 physical;
  if(!MapVAddr(regs, LOAD, addr, physical)) {
    HandleTLBException(regs, addr);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, LOAD), 0, true);
  } else {
    u32 data = mem.Read32<false>(regs, physical, regs.oldPC);
    regs.cop1.SetReg<u32>(regs.cop0, FT(instr), data);
  }
}

void swc1(n64::Registers& regs, Mem& mem, u32 instr) {
  if(!regs.cop0.status.cu1) {
    FireException(regs, ExceptionCode::CoprocessorUnusable, 1, true);
    return;
  }

  u64 addr = (s64)(s16)instr + regs.gpr[BASE(instr)];
  if(addr & 3) {
    FireException(regs, ExceptionCode::AddressErrorStore, 0, true);
  }

  u32 physical;
  if(!MapVAddr(regs, STORE, addr, physical)) {
    HandleTLBException(regs, addr);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, STORE), 0, regs.oldPC);
  } else {
    mem.Write32<false>(regs, physical, regs.cop1.GetReg<u32>(regs.cop0, FT(instr)), regs.oldPC);
  }
}

void ldc1(n64::Registers& regs, Mem& mem, u32 instr) {
  if(!regs.cop0.status.cu1) {
    FireException(regs, ExceptionCode::CoprocessorUnusable, 1, regs.oldPC);
    return;
  }

  u64 addr = (s64)(s16)instr + regs.gpr[BASE(instr)];
  if(addr & 7) {
    FireException(regs, ExceptionCode::AddressErrorLoad, 0, regs.oldPC);
  }

  u32 physical;
  if(!MapVAddr(regs, LOAD, addr, physical)) {
    HandleTLBException(regs, addr);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, LOAD), 0, regs.oldPC);
  } else {
    u64 data = mem.Read64<false>(regs, physical, regs.oldPC);
    regs.cop1.SetReg<u64>(regs.cop0, FT(instr), data);
  }
}

void sdc1(n64::Registers& regs, Mem& mem, u32 instr) {
  if(!regs.cop0.status.cu1) {
    FireException(regs, ExceptionCode::CoprocessorUnusable, 1, regs.oldPC);
    return;
  }

  u64 addr = (s64)(s16)instr + regs.gpr[BASE(instr)];
  if(addr & 7) {
    FireException(regs, ExceptionCode::AddressErrorStore, 0, regs.oldPC);
  }

  u32 physical;
  if(!MapVAddr(regs, STORE, addr, physical)) {
    HandleTLBException(regs, addr);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, STORE), 0, regs.oldPC);
  } else {
    mem.Write64<false>(regs, physical, regs.cop1.GetReg<u64>(regs.cop0, FT(instr)), regs.oldPC);
  }
}

void truncws(n64::Registers& regs, u32 instr) {
  float fs = regs.cop1.GetCop1Reg<float>(regs.cop0, FS(instr));
  s32 result = (s32)std::trunc(fs);
  regs.cop1.SetReg<u32>(regs.cop0, FD(instr), result);
}

void truncwd(n64::Registers& regs, u32 instr) {
  double fs = regs.cop1.GetCop1Reg<double>(regs.cop0, FS(instr));
  s32 result = (s32)std::trunc(fs);
  regs.cop1.SetReg<u32>(regs.cop0, FD(instr), result);
}

void truncls(n64::Registers& regs, u32 instr) {
  float fs = regs.cop1.GetCop1Reg<float>(regs.cop0, FS(instr));
  s64 result = (s64)std::trunc(fs);
  regs.cop1.SetReg<u64>(regs.cop0, FD(instr), result);
}

void truncld(n64::Registers& regs, u32 instr) {
  double fs = regs.cop1.GetCop1Reg<double>(regs.cop0, FS(instr));
  s64 result = (s64)std::trunc(fs);
  regs.cop1.SetReg<u64>(regs.cop0, FD(instr), result);
}

void mfc1(n64::Registers& regs, u32 instr) {
  regs.gpr[RT(instr)] = (s32)regs.cop1.GetReg<u32>(regs.cop0, FS(instr));
}

void dmfc1(n64::Registers& regs, u32 instr) {
  regs.gpr[RT(instr)] = (s64)regs.cop1.GetReg<u64>(regs.cop0, FS(instr));
}

void mtc1(n64::Registers& regs, u32 instr) {
  regs.cop1.SetReg<u32>(regs.cop0, FS(instr), regs.gpr[RT(instr)]);
}

void dmtc1(n64::Registers& regs, u32 instr) {
  regs.cop1.SetReg<u64>(regs.cop0, FS(instr), regs.gpr[RT(instr)]);
}
}