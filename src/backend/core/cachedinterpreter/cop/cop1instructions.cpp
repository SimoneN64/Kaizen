#include <cachedinterpreter/cop/cop1instructions.hpp>
#include <cfenv>
#include <cmath>
#include <Cop1.hpp>
#include <Registers.hpp>

namespace n64 {
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

void absd(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  double fs = regs.cop1.GetCop1Reg<double>(regs.cop0, FS(instr));
  regs.cop1.SetCop1Reg<double>(regs.cop0, FD(instr), std::abs(fs));
}

void abss(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  float fs = regs.cop1.GetCop1Reg<float>(regs.cop0, FS(instr));
  regs.cop1.SetCop1Reg<float>(regs.cop0, FD(instr), std::abs(fs));
}

void absw(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  s32 fs = regs.cop1.GetReg<s32>(regs.cop0, FS(instr));
  regs.cop1.SetReg<u32>(regs.cop0, FD(instr), std::abs(fs));
}

void absl(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  s64 fs = regs.cop1.GetReg<s64>(regs.cop0, FS(instr));
  regs.cop1.SetReg(regs.cop0, FD(instr), std::abs(fs));
}

void adds(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  float fs = regs.cop1.GetCop1Reg<float>(regs.cop0, FS(instr));
  float ft = regs.cop1.GetCop1Reg<float>(regs.cop0, FT(instr));
  checknanregs(fs, ft);
  float result = fs + ft;
  regs.cop1.SetCop1Reg<float>(regs.cop0, FD(instr), result);
}

void addd(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  double fs = regs.cop1.GetCop1Reg<double>(regs.cop0, FS(instr));
  double ft = regs.cop1.GetCop1Reg<double>(regs.cop0, FT(instr));
  checknanregs(fs, ft);
  double result = fs + ft;
  regs.cop1.SetCop1Reg<double>(regs.cop0, FD(instr), result);
}

void ceills(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  float fs = regs.cop1.GetCop1Reg<float>(regs.cop0, FS(instr));
  s64 result = std::ceil(fs);
  regs.cop1.SetReg<u64>(regs.cop0, FD(instr), result);
}

void ceilws(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  float fs = regs.cop1.GetCop1Reg<float>(regs.cop0, FS(instr));
  s32 result = std::ceil(fs);
  regs.cop1.SetReg<u32>(regs.cop0, FD(instr), result);
}

void ceilld(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  double fs = regs.cop1.GetCop1Reg<double>(regs.cop0, FS(instr));
  s64 result = std::ceil(fs);
  regs.cop1.SetReg<u64>(regs.cop0, FD(instr), result);
}

void ceilwd(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  double fs = regs.cop1.GetCop1Reg<double>(regs.cop0, FS(instr));
  s32 result = std::ceil(fs);
  regs.cop1.SetReg<u32>(regs.cop0, FD(instr), result);
}

void cfc1(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  u8 fd = RD(instr);
  s32 val = 0;
  switch(fd) {
    case 0: val = regs.cop1.fcr0; break;
    case 31:
      val = regs.cop1.fcr31.raw;
      break;
    default: Util::panic("Undefined CFC1 with rd != 0 or 31");
  }
  regs.gpr[RT(instr)] = val;
}

void ctc1(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  u8 fs = RD(instr);
  u32 val = regs.gpr[RT(instr)];
  switch(fs) {
    case 0: break;
    case 31: {
      val &= 0x183ffff;
      regs.cop1.fcr31.raw = val;
    } break;
    default: Util::panic("Undefined CTC1 with rd != 0 or 31");
  }
}

void cvtds(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  regs.cop1.SetCop1Reg<double>(
    regs.cop0,
    FD(instr),
    regs.cop1.GetCop1Reg<float>(
      regs.cop0,
      FS(instr)
    )
  );
}

void cvtsd(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  regs.cop1.SetCop1Reg<float>(
    regs.cop0,
    FD(instr),
    regs.cop1.GetCop1Reg<double>(
      regs.cop0,
      FS(instr)
    )
  );
}

void cvtwd(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  regs.cop1.SetReg<u32>(
    regs.cop0,
    FD(instr),
    regs.cop1.GetCop1Reg<double>(
      regs.cop0,
      FS(instr)
    )
  );
}

void cvtws(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  regs.cop1.SetReg<u32>(
    regs.cop0,
    FD(instr),
    regs.cop1.GetCop1Reg<float>(
      regs.cop0,
      FS(instr)
    )
  );
}

void cvtls(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  regs.cop1.SetReg<u64>(
    regs.cop0,
    FD(instr),
    regs.cop1.GetCop1Reg<float>(
      regs.cop0,
      FS(instr)
    )
  );
}

void cvtsl(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  regs.cop1.SetCop1Reg<float>(
    regs.cop0,
    FD(instr),
    (s64)regs.cop1.GetReg<u64>(
      regs.cop0,
      FS(instr)
    )
  );
}

void cvtdw(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  regs.cop1.SetCop1Reg<double>(
    regs.cop0,
    FD(instr),
    (s32)regs.cop1.GetReg<u32>(
      regs.cop0,
      FS(instr)
    )
  );
}

void cvtsw(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  regs.cop1.SetCop1Reg<float>(
    regs.cop0,
    FD(instr),
    (s32)regs.cop1.GetReg<u32>(
      regs.cop0,
      FS(instr)
    )
  );
}

void cvtdl(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  regs.cop1.SetCop1Reg<double>(
    regs.cop0,
    FD(instr),
    (s64)regs.cop1.GetReg<u64>(
      regs.cop0,
      FS(instr)
    )
  );
}

void cvtld(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
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
inline bool CalculateCondition(Registers& regs, T fs, T ft, CompConds cond) {
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
void ccond(CachedInterpreter& cpu, u32 instr, CompConds cond) {
  Registers& regs = cpu.regs;
  T fs = regs.cop1.GetCop1Reg<T>(regs.cop0, FS(instr));
  T ft = regs.cop1.GetCop1Reg<T>(regs.cop0, FT(instr));

  regs.cop1.fcr31.compare = CalculateCondition(regs, fs, ft, cond);
}

template void ccond<float>(CachedInterpreter& cpu, u32 instr, CompConds cond);
template void ccond<double>(CachedInterpreter& cpu, u32 instr, CompConds cond);

void divs(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  float fs = regs.cop1.GetCop1Reg<float>(regs.cop0, FS(instr));
  float ft = regs.cop1.GetCop1Reg<float>(regs.cop0, FT(instr));
  regs.cop1.SetCop1Reg<float>(regs.cop0, FD(instr), fs / ft);
}

void divd(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  double fs = regs.cop1.GetCop1Reg<double>(regs.cop0, FS(instr));
  double ft = regs.cop1.GetCop1Reg<double>(regs.cop0, FT(instr));
  regs.cop1.SetCop1Reg<double>(regs.cop0, FD(instr), fs / ft);
}

void muls(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  float fs = regs.cop1.GetCop1Reg<float>(regs.cop0, FS(instr));
  float ft = regs.cop1.GetCop1Reg<float>(regs.cop0, FT(instr));
  regs.cop1.SetCop1Reg<float>(regs.cop0, FD(instr), fs * ft);
}

void muld(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  double fs = regs.cop1.GetCop1Reg<double>(regs.cop0, FS(instr));
  double ft = regs.cop1.GetCop1Reg<double>(regs.cop0, FT(instr));
  regs.cop1.SetCop1Reg<double>(regs.cop0, FD(instr), fs * ft);
}

void mulw(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  u32 fs = regs.cop1.GetReg<u32>(regs.cop0, FS(instr));
  u32 ft = regs.cop1.GetReg<u32>(regs.cop0, FT(instr));
  regs.cop1.SetReg<u32>(regs.cop0, FD(instr), fs * ft);
}

void mull(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  u64 fs = regs.cop1.GetReg<u64>(regs.cop0, FS(instr));
  u64 ft = regs.cop1.GetReg<u64>(regs.cop0, FT(instr));
  regs.cop1.SetReg<u64>(regs.cop0, FD(instr), fs * ft);
}

void subs(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  float fs = regs.cop1.GetCop1Reg<float>(regs.cop0, FS(instr));
  float ft = regs.cop1.GetCop1Reg<float>(regs.cop0, FT(instr));
  regs.cop1.SetCop1Reg<float>(regs.cop0, FD(instr), fs - ft);
}

void subd(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  double fs = regs.cop1.GetCop1Reg<double>(regs.cop0, FS(instr));
  double ft = regs.cop1.GetCop1Reg<double>(regs.cop0, FT(instr));
  regs.cop1.SetCop1Reg<double>(regs.cop0, FD(instr), fs - ft);
}

void subw(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  u32 fs = regs.cop1.GetReg<u32>(regs.cop0, FS(instr));
  u32 ft = regs.cop1.GetReg<u32>(regs.cop0, FT(instr));
  regs.cop1.SetReg<u32>(regs.cop0, FD(instr), fs - ft);
}

void subl(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  u64 fs = regs.cop1.GetReg<u64>(regs.cop0, FS(instr));
  u64 ft = regs.cop1.GetReg<u64>(regs.cop0, FT(instr));
  regs.cop1.SetReg<u64>(regs.cop0, FD(instr), fs - ft);
}

void movs(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  regs.cop1.SetCop1Reg<float>(
    regs.cop0,
    FD(instr),
    regs.cop1.GetCop1Reg<float>(
      regs.cop0,
      FS(instr)
    )
  );
}

void movd(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  regs.cop1.SetCop1Reg<double>(
    regs.cop0,
    FD(instr),
    regs.cop1.GetCop1Reg<double>(
      regs.cop0,
      FS(instr)
    )
  );
}

void movw(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  regs.cop1.SetReg<u32>(
    regs.cop0,
    FD(instr),
    regs.cop1.GetReg<u32>(
      regs.cop0,
      FS(instr)
    )
  );
}

void movl(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  regs.cop1.SetReg<u64>(
    regs.cop0,
    FD(instr),
    regs.cop1.GetReg<u64>(
      regs.cop0,
      FS(instr)
    )
  );
}

void negs(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  regs.cop1.SetCop1Reg<float>(
    regs.cop0,
    FD(instr),
    -regs.cop1.GetCop1Reg<float>(
      regs.cop0,
      FS(instr)
    )
  );
}

void negd(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  regs.cop1.SetCop1Reg<double>(
    regs.cop0,
    FD(instr),
    -regs.cop1.GetCop1Reg<double>(
      regs.cop0,
      FS(instr)
    )
  );
}

void sqrts(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  float fs = regs.cop1.GetCop1Reg<float>(regs.cop0, FS(instr));
  regs.cop1.SetCop1Reg<float>(regs.cop0, FD(instr), std::sqrt(fs));
}

void sqrtd(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  double fs = regs.cop1.GetCop1Reg<double>(regs.cop0, FS(instr));
  regs.cop1.SetCop1Reg<double>(regs.cop0, FD(instr), std::sqrt(fs));
}

void roundls(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  float fs = regs.cop1.GetCop1Reg<float>(regs.cop0, FS(instr));
  PUSHROUNDINGMODE;
  regs.cop1.SetReg<u64>(regs.cop0, FD(instr), (s32)std::nearbyint(fs));
  POPROUNDINGMODE;
}

void roundld(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  double fs = regs.cop1.GetCop1Reg<double>(regs.cop0, FS(instr));
  PUSHROUNDINGMODE;
  regs.cop1.SetReg<u64>(regs.cop0, FD(instr), (s64)std::nearbyint(fs));
  POPROUNDINGMODE;
}

void roundws(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  float fs = regs.cop1.GetCop1Reg<float>(regs.cop0, FS(instr));
  PUSHROUNDINGMODE;
  regs.cop1.SetReg<u32>(regs.cop0, FD(instr), (s32)std::nearbyint(fs));
  POPROUNDINGMODE;
}

void roundwd(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  double fs = regs.cop1.GetCop1Reg<double>(regs.cop0, FS(instr));
  PUSHROUNDINGMODE;
  regs.cop1.SetReg<u32>(regs.cop0, FD(instr), (s32)std::nearbyint(fs));
  POPROUNDINGMODE;
}

void floorls(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  float fs = regs.cop1.GetCop1Reg<float>(regs.cop0, FS(instr));
  regs.cop1.SetReg<u64>(regs.cop0, FD(instr), (s64)std::floor(fs));
}

void floorld(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  double fs = regs.cop1.GetCop1Reg<double>(regs.cop0, FS(instr));
  regs.cop1.SetReg<u64>(regs.cop0, FD(instr), (s64)std::floor(fs));
}

void floorws(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  float fs = regs.cop1.GetCop1Reg<float>(regs.cop0, FS(instr));
  regs.cop1.SetReg<u32>(regs.cop0, FD(instr), (s64)std::floor(fs));
}

void floorwd(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  double fs = regs.cop1.GetCop1Reg<double>(regs.cop0, FS(instr));
  regs.cop1.SetReg<u32>(regs.cop0, FD(instr), (s64)std::floor(fs));
}

void lwc1(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  if(!regs.cop0.status.cu1) {
    FireException(regs, ExceptionCode::CoprocessorUnusable, 1, true);
    return;
  }

  u64 addr = (s64)(s16)instr + regs.gpr[BASE(instr)];

  u32 physical;
  if(!MapVAddr(regs, LOAD, addr, physical)) {
    HandleTLBException(regs, addr);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, LOAD), 0, true);
  } else {
    u32 data = cpu.mem.Read32(regs, physical);
    regs.cop1.SetReg<u32>(regs.cop0, FT(instr), data);
  }
}

void swc1(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  if(!regs.cop0.status.cu1) {
    FireException(regs, ExceptionCode::CoprocessorUnusable, 1, true);
    return;
  }

  u64 addr = (s64)(s16)instr + regs.gpr[BASE(instr)];

  u32 physical;
  if(!MapVAddr(regs, STORE, addr, physical)) {
    HandleTLBException(regs, addr);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, STORE), 0, true);
  } else {
    cpu.mem.Write32(regs, cpu, physical, regs.cop1.GetReg<u32>(regs.cop0, FT(instr)));
  }
}

void ldc1(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  if(!regs.cop0.status.cu1) {
    FireException(regs, ExceptionCode::CoprocessorUnusable, 1, true);
    return;
  }

  u64 addr = (s64)(s16)instr + regs.gpr[BASE(instr)];

  u32 physical;
  if(!MapVAddr(regs, LOAD, addr, physical)) {
    HandleTLBException(regs, addr);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, LOAD), 0, true);
  } else {
    u64 data = cpu.mem.Read64(regs, physical);
    regs.cop1.SetReg<u64>(regs.cop0, FT(instr), data);
  }
}

void sdc1(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  if(!regs.cop0.status.cu1) {
    FireException(regs, ExceptionCode::CoprocessorUnusable, 1, true);
    return;
  }

  u64 addr = (s64)(s16)instr + regs.gpr[BASE(instr)];

  u32 physical;
  if(!MapVAddr(regs, STORE, addr, physical)) {
    HandleTLBException(regs, addr);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, STORE), 0, true);
  } else {
    cpu.mem.Write64(regs, cpu, physical, regs.cop1.GetReg<u64>(regs.cop0, FT(instr)));
  }
}

void truncws(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  float fs = regs.cop1.GetCop1Reg<float>(regs.cop0, FS(instr));
  s32 result = (s32)std::trunc(fs);
  regs.cop1.SetReg<u32>(regs.cop0, FD(instr), result);
}

void truncwd(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  double fs = regs.cop1.GetCop1Reg<double>(regs.cop0, FS(instr));
  s32 result = (s32)std::trunc(fs);
  regs.cop1.SetReg<u32>(regs.cop0, FD(instr), result);
}

void truncls(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  float fs = regs.cop1.GetCop1Reg<float>(regs.cop0, FS(instr));
  s64 result = (s64)std::trunc(fs);
  regs.cop1.SetReg<u64>(regs.cop0, FD(instr), result);
}

void truncld(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  double fs = regs.cop1.GetCop1Reg<double>(regs.cop0, FS(instr));
  s64 result = (s64)std::trunc(fs);
  regs.cop1.SetReg<u64>(regs.cop0, FD(instr), result);
}

void mfc1(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  regs.gpr[RT(instr)] = (s32)regs.cop1.GetReg<u32>(regs.cop0, FS(instr));
}

void dmfc1(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  regs.gpr[RT(instr)] = (s64)regs.cop1.GetReg<u64>(regs.cop0, FS(instr));
}

void mtc1(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  regs.cop1.SetReg<u32>(regs.cop0, FS(instr), regs.gpr[RT(instr)]);
}

void dmtc1(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  regs.cop1.SetReg<u64>(regs.cop0, FS(instr), regs.gpr[RT(instr)]);
}

}