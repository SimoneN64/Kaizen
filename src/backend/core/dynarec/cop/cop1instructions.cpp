#include <dynarec/cop/cop1instructions.hpp>
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

void absd(Dynarec& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  auto fs = regs.cop1.GetCop1Reg<double>(regs.cop0, FS(instr));
  regs.cop1.SetCop1Reg<double>(regs.cop0, FD(instr), std::abs(fs));
}

void abss(Dynarec& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  auto fs = regs.cop1.GetCop1Reg<float>(regs.cop0, FS(instr));
  regs.cop1.SetCop1Reg<float>(regs.cop0, FD(instr), std::abs(fs));
}

void absw(Dynarec& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  s32 fs = regs.cop1.GetReg<s32>(regs.cop0, FS(instr));
  regs.cop1.SetReg<u32>(regs.cop0, FD(instr), std::abs(fs));
}

void absl(Dynarec& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  s64 fs = regs.cop1.GetReg<s64>(regs.cop0, FS(instr));
  regs.cop1.SetReg(regs.cop0, FD(instr), std::abs(fs));
}

void adds(Dynarec& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  auto fs = regs.cop1.GetCop1Reg<float>(regs.cop0, FS(instr));
  auto ft = regs.cop1.GetCop1Reg<float>(regs.cop0, FT(instr));
  checknanregs(fs, ft);
  float result = fs + ft;
  regs.cop1.SetCop1Reg<float>(regs.cop0, FD(instr), result);
}

void addd(Dynarec& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  auto fs = regs.cop1.GetCop1Reg<double>(regs.cop0, FS(instr));
  auto ft = regs.cop1.GetCop1Reg<double>(regs.cop0, FT(instr));
  checknanregs(fs, ft);
  double result = fs + ft;
  regs.cop1.SetCop1Reg<double>(regs.cop0, FD(instr), result);
}

void ceills(Dynarec& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  auto fs = regs.cop1.GetCop1Reg<float>(regs.cop0, FS(instr));
  s64 result = std::ceil(fs);
  regs.cop1.SetReg<u64>(regs.cop0, FD(instr), result);
}

void ceilws(Dynarec& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  auto fs = regs.cop1.GetCop1Reg<float>(regs.cop0, FS(instr));
  s32 result = std::ceil(fs);
  regs.cop1.SetReg<u32>(regs.cop0, FD(instr), result);
}

void ceilld(Dynarec& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  auto fs = regs.cop1.GetCop1Reg<double>(regs.cop0, FS(instr));
  s64 result = std::ceil(fs);
  regs.cop1.SetReg<u64>(regs.cop0, FD(instr), result);
}

void ceilwd(Dynarec& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  auto fs = regs.cop1.GetCop1Reg<double>(regs.cop0, FS(instr));
  s32 result = std::ceil(fs);
  regs.cop1.SetReg<u32>(regs.cop0, FD(instr), result);
}

void cfc1(Dynarec& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  u8 fd = RD(instr);
  s32 val = 0;
  switch(fd) {
    case 0: val = regs.cop1.fcr0; break;
    case 31: val = regs.cop1.fcr31.raw; break;
    default: Util::panic("Undefined CFC1 with rd != 0 or 31\n");
  }
  regs.gpr[RT(instr)] = val;
}

void ctc1(Dynarec& cpu, u32 instr) {
  Registers& regs = cpu.regs;
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

void cvtds(Dynarec& cpu, u32 instr) {
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

void cvtsd(Dynarec& cpu, u32 instr) {
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

void cvtwd(Dynarec& cpu, u32 instr) {
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

void cvtws(Dynarec& cpu, u32 instr) {
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

void cvtls(Dynarec& cpu, u32 instr) {
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

void cvtsl(Dynarec& cpu, u32 instr) {
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

void cvtdw(Dynarec& cpu, u32 instr) {
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

void cvtsw(Dynarec& cpu, u32 instr) {
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

void cvtdl(Dynarec& cpu, u32 instr) {
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

void cvtld(Dynarec& cpu, u32 instr) {
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
inline bool CalculateCondition(Dynarec& cpu, T fs, T ft, CompConds cond) {
  Registers& regs = cpu.regs;
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

      return CalculateCondition(cpu, fs, ft, static_cast<CompConds>(cond - 8));
  }
}

template <typename T>
void ccond(Dynarec& cpu, u32 instr, CompConds cond) {
  Registers& regs = cpu.regs;
  T fs = regs.cop1.GetCop1Reg<T>(regs.cop0, FS(instr));
  T ft = regs.cop1.GetCop1Reg<T>(regs.cop0, FT(instr));

  regs.cop1.fcr31.compare = CalculateCondition(cpu, fs, ft, cond);
}

template void ccond<float>(Dynarec& cpu, u32 instr, CompConds cond);
template void ccond<double>(Dynarec& cpu, u32 instr, CompConds cond);

void divs(Dynarec& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  float fs = regs.cop1.GetCop1Reg<float>(regs.cop0, FS(instr));
  float ft = regs.cop1.GetCop1Reg<float>(regs.cop0, FT(instr));
  regs.cop1.SetCop1Reg<float>(regs.cop0, FD(instr), fs / ft);
}

void divd(Dynarec& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  double fs = regs.cop1.GetCop1Reg<double>(regs.cop0, FS(instr));
  double ft = regs.cop1.GetCop1Reg<double>(regs.cop0, FT(instr));
  regs.cop1.SetCop1Reg<double>(regs.cop0, FD(instr), fs / ft);
}

void muls(Dynarec& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  float fs = regs.cop1.GetCop1Reg<float>(regs.cop0, FS(instr));
  float ft = regs.cop1.GetCop1Reg<float>(regs.cop0, FT(instr));
  regs.cop1.SetCop1Reg<float>(regs.cop0, FD(instr), fs * ft);
}

void muld(Dynarec& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  double fs = regs.cop1.GetCop1Reg<double>(regs.cop0, FS(instr));
  double ft = regs.cop1.GetCop1Reg<double>(regs.cop0, FT(instr));
  regs.cop1.SetCop1Reg<double>(regs.cop0, FD(instr), fs * ft);
}

void mulw(Dynarec& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  u32 fs = regs.cop1.GetReg<u32>(regs.cop0, FS(instr));
  u32 ft = regs.cop1.GetReg<u32>(regs.cop0, FT(instr));
  regs.cop1.SetReg<u32>(regs.cop0, FD(instr), fs * ft);
}

void mull(Dynarec& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  u64 fs = regs.cop1.GetReg<u64>(regs.cop0, FS(instr));
  u64 ft = regs.cop1.GetReg<u64>(regs.cop0, FT(instr));
  regs.cop1.SetReg<u64>(regs.cop0, FD(instr), fs * ft);
}

void subs(Dynarec& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  float fs = regs.cop1.GetCop1Reg<float>(regs.cop0, FS(instr));
  float ft = regs.cop1.GetCop1Reg<float>(regs.cop0, FT(instr));
  regs.cop1.SetCop1Reg<float>(regs.cop0, FD(instr), fs - ft);
}

void subd(Dynarec& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  double fs = regs.cop1.GetCop1Reg<double>(regs.cop0, FS(instr));
  double ft = regs.cop1.GetCop1Reg<double>(regs.cop0, FT(instr));
  regs.cop1.SetCop1Reg<double>(regs.cop0, FD(instr), fs - ft);
}

void subw(Dynarec& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  u32 fs = regs.cop1.GetReg<u32>(regs.cop0, FS(instr));
  u32 ft = regs.cop1.GetReg<u32>(regs.cop0, FT(instr));
  regs.cop1.SetReg<u32>(regs.cop0, FD(instr), fs - ft);
}

void subl(Dynarec& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  u64 fs = regs.cop1.GetReg<u64>(regs.cop0, FS(instr));
  u64 ft = regs.cop1.GetReg<u64>(regs.cop0, FT(instr));
  regs.cop1.SetReg<u64>(regs.cop0, FD(instr), fs - ft);
}

void movs(Dynarec& cpu, u32 instr) {
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

void movd(Dynarec& cpu, u32 instr) {
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

void movw(Dynarec& cpu, u32 instr) {
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

void movl(Dynarec& cpu, u32 instr) {
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

void negs(Dynarec& cpu, u32 instr) {
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

void negd(Dynarec& cpu, u32 instr) {
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

void sqrts(Dynarec& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  float fs = regs.cop1.GetCop1Reg<float>(regs.cop0, FS(instr));
  regs.cop1.SetCop1Reg<float>(regs.cop0, FD(instr), std::sqrt(fs));
}

void sqrtd(Dynarec& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  double fs = regs.cop1.GetCop1Reg<double>(regs.cop0, FS(instr));
  regs.cop1.SetCop1Reg<double>(regs.cop0, FD(instr), std::sqrt(fs));
}

void roundls(Dynarec& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  float fs = regs.cop1.GetCop1Reg<float>(regs.cop0, FS(instr));
  PUSHROUNDINGMODE;
  regs.cop1.SetReg<u64>(regs.cop0, FD(instr), (s32)std::nearbyint(fs));
  POPROUNDINGMODE;
}

void roundld(Dynarec& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  double fs = regs.cop1.GetCop1Reg<double>(regs.cop0, FS(instr));
  PUSHROUNDINGMODE;
  regs.cop1.SetReg<u64>(regs.cop0, FD(instr), (s64)std::nearbyint(fs));
  POPROUNDINGMODE;
}

void roundws(Dynarec& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  float fs = regs.cop1.GetCop1Reg<float>(regs.cop0, FS(instr));
  PUSHROUNDINGMODE;
  regs.cop1.SetReg<u32>(regs.cop0, FD(instr), (s32)std::nearbyint(fs));
  POPROUNDINGMODE;
}

void roundwd(Dynarec& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  double fs = regs.cop1.GetCop1Reg<double>(regs.cop0, FS(instr));
  PUSHROUNDINGMODE;
  regs.cop1.SetReg<u32>(regs.cop0, FD(instr), (s32)std::nearbyint(fs));
  POPROUNDINGMODE;
}

void floorls(Dynarec& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  float fs = regs.cop1.GetCop1Reg<float>(regs.cop0, FS(instr));
  regs.cop1.SetReg<u64>(regs.cop0, FD(instr), (s64)std::floor(fs));
}

void floorld(Dynarec& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  double fs = regs.cop1.GetCop1Reg<double>(regs.cop0, FS(instr));
  regs.cop1.SetReg<u64>(regs.cop0, FD(instr), (s64)std::floor(fs));
}

void floorws(Dynarec& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  float fs = regs.cop1.GetCop1Reg<float>(regs.cop0, FS(instr));
  regs.cop1.SetReg<u32>(regs.cop0, FD(instr), (s64)std::floor(fs));
}

void floorwd(Dynarec& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  double fs = regs.cop1.GetCop1Reg<double>(regs.cop0, FS(instr));
  regs.cop1.SetReg<u32>(regs.cop0, FD(instr), (s64)std::floor(fs));
}

void lwc1(Dynarec& cpu, u32 instr) {
  Mem& mem = cpu.mem;
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
    u32 data = mem.Read32(regs, physical);
    regs.cop1.SetReg<u32>(regs.cop0, FT(instr), data);
  }
}

void swc1(Dynarec& cpu, u32 instr) {
  Mem& mem = cpu.mem;
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
    mem.Write32(regs, physical, regs.cop1.GetReg<u32>(regs.cop0, FT(instr)));
  }
}

void ldc1(Dynarec& cpu, u32 instr) {
  Mem& mem = cpu.mem;
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
    u64 data = mem.Read64(regs, physical);
    regs.cop1.SetReg<u64>(regs.cop0, FT(instr), data);
  }
}

void sdc1(Dynarec& cpu, u32 instr) {
  Mem& mem = cpu.mem;
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
    mem.Write64(regs, physical, regs.cop1.GetReg<u64>(regs.cop0, FT(instr)));
  }
}

void truncws(Dynarec& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  float fs = regs.cop1.GetCop1Reg<float>(regs.cop0, FS(instr));
  s32 result = (s32)std::trunc(fs);
  regs.cop1.SetReg<u32>(regs.cop0, FD(instr), result);
}

void truncwd(Dynarec& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  double fs = regs.cop1.GetCop1Reg<double>(regs.cop0, FS(instr));
  s32 result = (s32)std::trunc(fs);
  regs.cop1.SetReg<u32>(regs.cop0, FD(instr), result);
}

void truncls(Dynarec& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  float fs = regs.cop1.GetCop1Reg<float>(regs.cop0, FS(instr));
  s64 result = (s64)std::trunc(fs);
  regs.cop1.SetReg<u64>(regs.cop0, FD(instr), result);
}

void truncld(Dynarec& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  double fs = regs.cop1.GetCop1Reg<double>(regs.cop0, FS(instr));
  s64 result = (s64)std::trunc(fs);
  regs.cop1.SetReg<u64>(regs.cop0, FD(instr), result);
}

void mfc1(Dynarec& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  regs.gpr[RT(instr)] = (s32)regs.cop1.GetReg<u32>(regs.cop0, FS(instr));
}

void dmfc1(Dynarec& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  regs.gpr[RT(instr)] = (s64)regs.cop1.GetReg<u64>(regs.cop0, FS(instr));
}

void mtc1(Dynarec& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  regs.cop1.SetReg<u32>(regs.cop0, FS(instr), regs.gpr[RT(instr)]);
}

void dmtc1(Dynarec& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  regs.cop1.SetReg<u64>(regs.cop0, FS(instr), regs.gpr[RT(instr)]);
}
}