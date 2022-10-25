#include <n64/core/cpu/registers/Cop1.hpp>
#include <n64/core/cpu/Registers.hpp>
#include <n64/core/Mem.hpp>
#include <util.hpp>
#include <cmath>
#include <cfenv>

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

#define PUSHROUNDINGMODE int og = PushRoundingMode(fcr31)
#define POPROUNDINGMODE fesetround(og)

void Cop1::absd(Registers& regs, u32 instr) {
  double fs = GetCop1Reg<double>(regs.cop0, FS(instr));
  SetCop1Reg<double>(regs.cop0, FD(instr), fabs(fs));
}

void Cop1::abss(Registers& regs, u32 instr) {
  float fs = GetCop1Reg<float>(regs.cop0, FS(instr));
  SetCop1Reg<float>(regs.cop0, FD(instr), fabsf(fs));
}

void Cop1::absw(Registers& regs, u32 instr) {
  s32 fs = GetReg<s32>(regs.cop0, FS(instr));
  SetReg<u32>(regs.cop0, FD(instr), abs(fs));
}

void Cop1::absl(Registers& regs, u32 instr) {
  s64 fs = GetReg<s64>(regs.cop0, FS(instr));
  SetReg(regs.cop0, FD(instr), labs(fs));
}

void Cop1::adds(Registers& regs, u32 instr) {
  float fs = GetCop1Reg<float>(regs.cop0, FS(instr));
  float ft = GetCop1Reg<float>(regs.cop0, FT(instr));
  float result = fs + ft;
  SetCop1Reg<float>(regs.cop0, FD(instr), result);
}

void Cop1::addd(Registers& regs, u32 instr) {
  double fs = GetCop1Reg<double>(regs.cop0, FS(instr));
  double ft = GetCop1Reg<double>(regs.cop0, FT(instr));
  double result = fs + ft;
  SetCop1Reg<double>(regs.cop0, FD(instr), result);
}

void Cop1::ceills(Registers& regs, u32 instr) {
  float fs = GetCop1Reg<float>(regs.cop0, FS(instr));
  s64 result = ceilf(fs);
  SetReg<u64>(regs.cop0, FD(instr), result);
}

void Cop1::ceilws(Registers& regs, u32 instr) {
  float fs = GetCop1Reg<float>(regs.cop0, FS(instr));
  s32 result = ceilf(fs);
  SetReg<u32>(regs.cop0, FD(instr), result);
}

void Cop1::ceilld(Registers& regs, u32 instr) {
  double fs = GetCop1Reg<double>(regs.cop0, FS(instr));
  s64 result = ceil(fs);
  SetReg<u64>(regs.cop0, FD(instr), result);
}

void Cop1::ceilwd(Registers& regs, u32 instr) {
  double fs = GetCop1Reg<double>(regs.cop0, FS(instr));
  s32 result = ceil(fs);
  SetReg<u32>(regs.cop0, FD(instr), result);
}

void Cop1::cfc1(Registers& regs, u32 instr) const {
  u8 fd = FD(instr);
  s32 val = 0;
  switch(fd) {
    case 0: val = (s32)fcr0; break;
    case 31: val = (s32)fcr31.raw; break;
    default: util::panic("Undefined CFC1 with rd != 0 or 31\n");
  }
  regs.gpr[RT(instr)] = val;
}

void Cop1::ctc1(Registers& regs, u32 instr) {
  u8 fs = FS(instr);
  u32 val = regs.gpr[RT(instr)];
  switch(fs) {
    case 0: break;
    case 31: {
      val &= 0x183ffff;
      fcr31.raw = val;
    } break;
    default: util::panic("Undefined CTC1 with rd != 0 or 31\n");
  }
}

void Cop1::cvtds(Registers& regs, u32 instr) {
  SetCop1Reg<double>(
        regs.cop0,
    FD(instr),
    GetCop1Reg<float>(
            regs.cop0,
      FS(instr)
    )
  );
}

void Cop1::cvtsd(Registers& regs, u32 instr) {
  SetCop1Reg<float>(
        regs.cop0,
    FD(instr),
    GetCop1Reg<double>(
            regs.cop0,
      FS(instr)
    )
  );
}

void Cop1::cvtwd(Registers& regs, u32 instr) {
  SetReg<u32>(
        regs.cop0,
    FD(instr),
    GetCop1Reg<double>(
            regs.cop0,
      FS(instr)
    )
  );
}

void Cop1::cvtws(Registers& regs, u32 instr) {
  SetReg<u32>(
        regs.cop0,
    FD(instr),
    GetCop1Reg<float>(
            regs.cop0,
      FS(instr)
    )
  );
}

void Cop1::cvtls(Registers& regs, u32 instr) {
  SetReg<u64>(
        regs.cop0,
    FD(instr),
    GetCop1Reg<float>(
            regs.cop0,
      FS(instr)
    )
  );
}

void Cop1::cvtsl(Registers& regs, u32 instr) {
  SetCop1Reg<float>(
        regs.cop0,
    FD(instr),
    (s64)GetReg<u64>(
            regs.cop0,
      FS(instr)
    )
  );
}

void Cop1::cvtdw(Registers& regs, u32 instr) {
  SetCop1Reg<double>(
    regs.cop0,
    FD(instr),
    (s32)GetReg<u32>(
      regs.cop0,
      FS(instr)
    )
  );
}

void Cop1::cvtsw(Registers& regs, u32 instr) {
  SetCop1Reg<float>(
        regs.cop0,
    FD(instr),
    (s32)GetReg<u32>(
            regs.cop0,
      FS(instr)
    )
  );
}

void Cop1::cvtdl(Registers& regs, u32 instr) {
  SetCop1Reg<double>(
        regs.cop0,
    FD(instr),
    (s64)GetReg<u64>(
            regs.cop0,
      FS(instr)
    )
  );
}

void Cop1::cvtld(Registers& regs, u32 instr) {
  SetReg<u64>(
    regs.cop0,
    FD(instr),
    GetCop1Reg<double>(
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
        FireException(regs, ExceptionCode::FloatingPointError, 0, regs.oldPC);
      }

      return CalculateCondition(regs, fs, ft, static_cast<CompConds>(cond - 8));
  }
}

template <typename T>
void Cop1::ccond(Registers& regs, u32 instr, CompConds cond) {
  T fs = GetCop1Reg<T>(regs.cop0, FS(instr));
  T ft = GetCop1Reg<T>(regs.cop0, FT(instr));

  fcr31.compare = CalculateCondition(regs, fs, ft, cond);
}

template void Cop1::ccond<float>(Registers& regs, u32 instr, CompConds cond);
template void Cop1::ccond<double>(Registers& regs, u32 instr, CompConds cond);

void Cop1::divs(Registers &regs, u32 instr) {
  float fs = GetCop1Reg<float>(regs.cop0, FS(instr));
  float ft = GetCop1Reg<float>(regs.cop0, FT(instr));
  SetCop1Reg<float>(regs.cop0, FD(instr), fs / ft);
}

void Cop1::divd(Registers &regs, u32 instr) {
  double fs = GetCop1Reg<double>(regs.cop0, FS(instr));
  double ft = GetCop1Reg<double>(regs.cop0, FT(instr));
  SetCop1Reg<double>(regs.cop0, FD(instr), fs / ft);
}

void Cop1::muls(Registers &regs, u32 instr) {
  float fs = GetCop1Reg<float>(regs.cop0, FS(instr));
  float ft = GetCop1Reg<float>(regs.cop0, FT(instr));
  SetCop1Reg<float>(regs.cop0, FD(instr), fs * ft);
}

void Cop1::muld(Registers& regs, u32 instr) {
  double fs = GetCop1Reg<double>(regs.cop0, FS(instr));
  double ft = GetCop1Reg<double>(regs.cop0, FT(instr));
  SetCop1Reg<double>(regs.cop0, FD(instr), fs * ft);
}

void Cop1::mulw(Registers &regs, u32 instr) {
  u32 fs = GetReg<u32>(regs.cop0, FS(instr));
  u32 ft = GetReg<u32>(regs.cop0, FT(instr));
  SetReg<u32>(regs.cop0, FD(instr), fs * ft);
}

void Cop1::mull(Registers &regs, u32 instr) {
  u64 fs = GetReg<u64>(regs.cop0, FS(instr));
  u64 ft = GetReg<u64>(regs.cop0, FT(instr));
  SetReg<u64>(regs.cop0, FD(instr), fs * ft);
}

void Cop1::subs(Registers &regs, u32 instr) {
  float fs = GetCop1Reg<float>(regs.cop0, FS(instr));
  float ft = GetCop1Reg<float>(regs.cop0, FT(instr));
  SetCop1Reg<float>(regs.cop0, FD(instr), fs - ft);
}

void Cop1::subd(Registers &regs, u32 instr) {
  double fs = GetCop1Reg<double>(regs.cop0, FS(instr));
  double ft = GetCop1Reg<double>(regs.cop0, FT(instr));
  SetCop1Reg<double>(regs.cop0, FD(instr), fs - ft);
}

void Cop1::subw(Registers &regs, u32 instr) {
  u32 fs = GetReg<u32>(regs.cop0, FS(instr));
  u32 ft = GetReg<u32>(regs.cop0, FT(instr));
  SetReg<u32>(regs.cop0, FD(instr), fs - ft);
}

void Cop1::subl(Registers &regs, u32 instr) {
  u64 fs = GetReg<u64>(regs.cop0, FS(instr));
  u64 ft = GetReg<u64>(regs.cop0, FT(instr));
  SetReg<u64>(regs.cop0, FD(instr), fs - ft);
}

void Cop1::movs(Registers& regs, u32 instr) {
  SetCop1Reg<float>(
    regs.cop0,
    FD(instr),
    GetCop1Reg<float>(
      regs.cop0,
      FS(instr)
    )
  );
}

void Cop1::movd(Registers& regs, u32 instr) {
  SetCop1Reg<double>(
    regs.cop0,
    FD(instr),
    GetCop1Reg<double>(
      regs.cop0,
      FS(instr)
    )
  );
}

void Cop1::movw(Registers& regs, u32 instr) {
  SetReg<u32>(
    regs.cop0,
    FD(instr),
    GetReg<u32>(
      regs.cop0,
      FS(instr)
    )
  );
}

void Cop1::movl(Registers& regs, u32 instr) {
  SetReg<u64>(
    regs.cop0,
    FD(instr),
    GetReg<u64>(
      regs.cop0,
      FS(instr)
    )
  );
}

void Cop1::negs(Registers &regs, u32 instr) {
  SetCop1Reg<float>(
    regs.cop0,
    FD(instr),
    -GetCop1Reg<float>(
      regs.cop0,
      FS(instr)
    )
  );
}

void Cop1::negd(Registers &regs, u32 instr) {
  SetCop1Reg<double>(
    regs.cop0,
    FD(instr),
    -GetCop1Reg<double>(
      regs.cop0,
      FS(instr)
    )
  );
}

void Cop1::sqrts(Registers &regs, u32 instr) {
  float fs = GetCop1Reg<float>(regs.cop0, FS(instr));
  SetCop1Reg<float>(regs.cop0, FD(instr), sqrtf(fs));
}

void Cop1::sqrtd(Registers &regs, u32 instr) {
  double fs = GetCop1Reg<double>(regs.cop0, FS(instr));
  SetCop1Reg<double>(regs.cop0, FD(instr), sqrt(fs));
}

void Cop1::roundls(Registers& regs, u32 instr) {
  float fs = GetCop1Reg<float>(regs.cop0, FS(instr));
  PUSHROUNDINGMODE;
  SetReg<u64>(regs.cop0, FD(instr), (s32)nearbyintf(fs));
  POPROUNDINGMODE;
}

void Cop1::roundld(Registers& regs, u32 instr) {
  double fs = GetCop1Reg<double>(regs.cop0, FS(instr));
  PUSHROUNDINGMODE;
  SetReg<u64>(regs.cop0, FD(instr), (s64)nearbyint(fs));
  POPROUNDINGMODE;
}

void Cop1::roundws(Registers& regs, u32 instr) {
  float fs = GetCop1Reg<float>(regs.cop0, FS(instr));
  PUSHROUNDINGMODE;
  SetReg<u32>(regs.cop0, FD(instr), (s32)nearbyintf(fs));
  POPROUNDINGMODE;
}

void Cop1::roundwd(Registers& regs, u32 instr) {
  double fs = GetCop1Reg<double>(regs.cop0, FS(instr));
  PUSHROUNDINGMODE;
  SetReg<u32>(regs.cop0, FD(instr), (s32)nearbyint(fs));
  POPROUNDINGMODE;
}

void Cop1::floorls(Registers& regs, u32 instr) {
  float fs = GetCop1Reg<float>(regs.cop0, FS(instr));
  SetReg<u64>(regs.cop0, FD(instr), (s64)floorf(fs));
}

void Cop1::floorld(Registers& regs, u32 instr) {
  double fs = GetCop1Reg<double>(regs.cop0, FS(instr));
  SetReg<u64>(regs.cop0, FD(instr), (s64)floor(fs));
}

void Cop1::floorws(Registers& regs, u32 instr) {
  float fs = GetCop1Reg<float>(regs.cop0, FS(instr));
  SetReg<u32>(regs.cop0, FD(instr), (s64)floorf(fs));
}

void Cop1::floorwd(Registers& regs, u32 instr) {
  double fs = GetCop1Reg<double>(regs.cop0, FS(instr));
  SetReg<u32>(regs.cop0, FD(instr), (s64)floor(fs));
}

void Cop1::lwc1(Registers& regs, Mem& mem, u32 instr) {
  if(!regs.cop0.status.cu1) {
    FireException(regs, ExceptionCode::CoprocessorUnusable, 1, regs.oldPC);
    return;
  }

  u64 addr = (s64)(s16)instr + regs.gpr[BASE(instr)];
  if(addr & 3) {
    FireException(regs, ExceptionCode::AddressErrorLoad, 0, regs.oldPC);
  }

  u32 physical;
  if(!MapVAddr(regs, LOAD, addr, physical)) {
    HandleTLBException(regs, addr);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, LOAD), 0, regs.oldPC);
  } else {
    u32 data = mem.Read32<false>(regs, physical, regs.oldPC);
    SetReg<u32>(regs.cop0, FT(instr), data);
  }
}

void Cop1::swc1(Registers& regs, Mem& mem, u32 instr) {
  if(!regs.cop0.status.cu1) {
    FireException(regs, ExceptionCode::CoprocessorUnusable, 1, regs.oldPC);
    return;
  }

  u64 addr = (s64)(s16)instr + regs.gpr[BASE(instr)];
  if(addr & 3) {
    FireException(regs, ExceptionCode::AddressErrorStore, 0, regs.oldPC);
  }

  u32 physical;
  if(!MapVAddr(regs, STORE, addr, physical)) {
    HandleTLBException(regs, addr);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, STORE), 0, regs.oldPC);
  } else {
    mem.Write32<false>(regs, physical, GetReg<u32>(regs.cop0, FT(instr)), regs.oldPC);
  }
}

void Cop1::ldc1(Registers& regs, Mem& mem, u32 instr) {
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
    SetReg<u64>(regs.cop0, FT(instr), data);
  }
}

void Cop1::sdc1(Registers& regs, Mem& mem, u32 instr) {
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
    mem.Write64<false>(regs, physical, GetReg<u64>(regs.cop0, FT(instr)), regs.oldPC);
  }
}

void Cop1::truncws(Registers& regs, u32 instr) {
  float fs = GetCop1Reg<float>(regs.cop0, FS(instr));
  s32 result = (s32)truncf(fs);
  SetReg<u32>(regs.cop0, FD(instr), result);
}

void Cop1::truncwd(Registers& regs, u32 instr) {
  double fs = GetCop1Reg<double>(regs.cop0, FS(instr));
  s32 result = (s32)trunc(fs);
  SetReg<u32>(regs.cop0, FD(instr), result);
}

void Cop1::truncls(Registers& regs, u32 instr) {
  float fs = GetCop1Reg<float>(regs.cop0, FS(instr));
  s64 result = (s64)truncf(fs);
  SetReg<u64>(regs.cop0, FD(instr), result);
}

void Cop1::truncld(Registers& regs, u32 instr) {
  double fs = GetCop1Reg<double>(regs.cop0, FS(instr));
  s64 result = (s64)trunc(fs);
  SetReg<u64>(regs.cop0, FD(instr), result);
}

void Cop1::mfc1(Registers& regs, u32 instr) {
  regs.gpr[RT(instr)] = (s32)GetReg<u32>(regs.cop0, FS(instr));
}

void Cop1::dmfc1(Registers& regs, u32 instr) {
  regs.gpr[RT(instr)] = (s64)GetReg<u64>(regs.cop0, FS(instr));
}

void Cop1::mtc1(Registers& regs, u32 instr) {
  SetReg<u32>(regs.cop0, FS(instr), regs.gpr[RT(instr)]);
}

void Cop1::dmtc1(Registers& regs, u32 instr) {
  SetReg<u64>(regs.cop0, FS(instr), regs.gpr[RT(instr)]);
}

}