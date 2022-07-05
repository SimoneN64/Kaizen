#include <n64/core/cpu/registers/Cop1.hpp>
#include <n64/core/cpu/Registers.hpp>
#include <n64/core/Mem.hpp>
#include <util.hpp>
#include <math.h>

namespace natsukashii::n64::core {
void Cop1::absd(Registers& regs, u32 instr) {
  double fs = GetCop1RegDouble(regs.cop0, FS(instr));
  SetCop1RegDouble(regs.cop0, FD(instr), fabs(fs));
}

void Cop1::abss(Registers& regs, u32 instr) {
  float fs = GetCop1RegFloat(regs.cop0, FS(instr));
  SetCop1RegFloat(regs.cop0, FD(instr), fabsf(fs));
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
  float fs = GetCop1RegFloat(regs.cop0, FS(instr));
  float ft = GetCop1RegFloat(regs.cop0, FT(instr));
  float result = fs + ft;
  SetCop1RegFloat(regs.cop0, FD(instr), result);
}

void Cop1::addd(Registers& regs, u32 instr) {
  double fs = GetCop1RegDouble(regs.cop0, FS(instr));
  double ft = GetCop1RegDouble(regs.cop0, FT(instr));
  double result = fs + ft;
  SetCop1RegDouble(regs.cop0, FD(instr), result);
}

void Cop1::ceills(Registers& regs, u32 instr) {
  float fs = GetCop1RegFloat(regs.cop0, FS(instr));
  s64 result = ceilf(fs);
  SetReg<u64>(regs.cop0, FD(instr), result);
}

void Cop1::ceilws(Registers& regs, u32 instr) {
  float fs = GetCop1RegFloat(regs.cop0, FS(instr));
  s32 result = ceilf(fs);
  SetReg<u32>(regs.cop0, FD(instr), result);
}

void Cop1::ceilld(Registers& regs, u32 instr) {
  double fs = GetCop1RegDouble(regs.cop0, FS(instr));
  s64 result = ceil(fs);
  SetReg<u64>(regs.cop0, FD(instr), result);
}

void Cop1::ceilwd(Registers& regs, u32 instr) {
  double fs = GetCop1RegDouble(regs.cop0, FS(instr));
  s32 result = ceil(fs);
  SetReg<u32>(regs.cop0, FD(instr), result);
}

void Cop1::cfc1(Registers& regs, u32 instr) {
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
    case 0: util::panic("CTC1 attempt to write to FCR0 which is read only!\n");
    case 31: {
      val &= 0x183ffff;
      fcr31.raw = val;
    } break;
    default: util::panic("Undefined CTC1 with rd != 0 or 31\n");
  }
}

void Cop1::cvtds(Registers& regs, u32 instr) {
  SetCop1RegDouble(
        regs.cop0,
    FD(instr),
    GetCop1RegFloat(
            regs.cop0,
      FS(instr)
    )
  );
}

void Cop1::cvtsd(Registers& regs, u32 instr) {
  SetCop1RegFloat(
        regs.cop0,
    FD(instr),
    GetCop1RegDouble(
            regs.cop0,
      FS(instr)
    )
  );
}

void Cop1::cvtwd(Registers& regs, u32 instr) {
  SetReg<u32>(
        regs.cop0,
    FD(instr),
    GetCop1RegDouble(
            regs.cop0,
      FS(instr)
    )
  );
}

void Cop1::cvtws(Registers& regs, u32 instr) {
  SetReg<u32>(
        regs.cop0,
    FD(instr),
    GetCop1RegFloat(
            regs.cop0,
      FS(instr)
    )
  );
}

void Cop1::cvtls(Registers& regs, u32 instr) {
  SetReg<u64>(
        regs.cop0,
    FD(instr),
    GetCop1RegFloat(
            regs.cop0,
      FS(instr)
    )
  );
}

void Cop1::cvtsl(Registers& regs, u32 instr) {
  SetCop1RegFloat(
        regs.cop0,
    FD(instr),
    (s64)GetReg<u64>(
            regs.cop0,
      FS(instr)
    )
  );
}

void Cop1::cvtdw(Registers& regs, u32 instr) {
  SetCop1RegDouble(
    regs.cop0,
    FD(instr),
    (s32)GetReg<u32>(
      regs.cop0,
      FS(instr)
    )
  );
}

void Cop1::cvtsw(Registers& regs, u32 instr) {
  SetCop1RegFloat(
        regs.cop0,
    FD(instr),
    (s32)GetReg<u32>(
            regs.cop0,
      FS(instr)
    )
  );
}

void Cop1::cvtdl(Registers& regs, u32 instr) {
  SetCop1RegDouble(
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
    GetCop1RegDouble(
            regs.cop0,
      FS(instr)
    )
  );
}

void Cop1::ccondd(Registers& regs, u32 instr, CompConds cond) {
  double fs = GetCop1RegDouble(regs.cop0, FS(instr));
  double ft = GetCop1RegDouble(regs.cop0, FT(instr));

  bool less, equal, unordered;
  if(isnan(fs) || isnan(ft)) {
    less = false;
    equal = false;
    unordered = true;
  } else {
    less = fs < ft;
    equal = fs == ft;
    unordered = false;
  }

  bool condition = ((cond >> 2) && less) || ((cond >> 1) && equal) || ((cond & 1) && unordered);

  fcr31.compare = condition;
}

void Cop1::cconds(Registers& regs, u32 instr, CompConds cond) {
  float fs = GetCop1RegFloat(regs.cop0, FS(instr));
  float ft = GetCop1RegFloat(regs.cop0, FT(instr));

  bool less, equal, unordered;
  if(isnan(fs) || isnan(ft)) {
    less = false;
    equal = false;
    unordered = true;
  } else {
    less = fs < ft;
    equal = fs == ft;
    unordered = false;
  }

  bool condition = ((cond >> 2) && less) || ((cond >> 1) && equal) || ((cond & 1) && unordered);

  fcr31.compare = condition;
}

void Cop1::divs(Registers &regs, u32 instr) {
  float fs = GetCop1RegFloat(regs.cop0, FS(instr));
  float ft = GetCop1RegFloat(regs.cop0, FT(instr));
  SetCop1RegFloat(regs.cop0, FD(instr), fs / ft);
}

void Cop1::divd(Registers &regs, u32 instr) {
  double fs = GetCop1RegDouble(regs.cop0, FS(instr));
  double ft = GetCop1RegDouble(regs.cop0, FT(instr));
  SetCop1RegDouble(regs.cop0, FD(instr), fs / ft);
}

void Cop1::muls(Registers &regs, u32 instr) {
  float fs = GetCop1RegFloat(regs.cop0, FS(instr));
  float ft = GetCop1RegFloat(regs.cop0, FT(instr));
  SetCop1RegFloat(regs.cop0, FD(instr), fs * ft);
}

void Cop1::muld(Registers& regs, u32 instr) {
  double fs = GetCop1RegDouble(regs.cop0, FS(instr));
  double ft = GetCop1RegDouble(regs.cop0, FT(instr));
  SetCop1RegDouble(regs.cop0, FD(instr), fs * ft);
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
  float fs = GetCop1RegFloat(regs.cop0, FS(instr));
  float ft = GetCop1RegFloat(regs.cop0, FT(instr));
  SetCop1RegFloat(regs.cop0, FD(instr), fs - ft);
}

void Cop1::subd(Registers &regs, u32 instr) {
  double fs = GetCop1RegDouble(regs.cop0, FS(instr));
  double ft = GetCop1RegDouble(regs.cop0, FT(instr));
  SetCop1RegDouble(regs.cop0, FD(instr), fs - ft);
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
  SetCop1RegFloat(
    regs.cop0,
    FD(instr),
    GetCop1RegFloat(
      regs.cop0,
      FS(instr)
    )
  );
}

void Cop1::movd(Registers& regs, u32 instr) {
  SetCop1RegDouble(
    regs.cop0,
    FD(instr),
    GetCop1RegDouble(
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
  SetCop1RegFloat(
    regs.cop0,
    FD(instr),
    -GetCop1RegFloat(
      regs.cop0,
      FS(instr)
    )
  );
}

void Cop1::negd(Registers &regs, u32 instr) {
  SetCop1RegDouble(
    regs.cop0,
    FD(instr),
    -GetCop1RegDouble(
      regs.cop0,
      FS(instr)
    )
  );
}

void Cop1::sqrts(Registers &regs, u32 instr) {
  float fs = GetCop1RegFloat(regs.cop0, FS(instr));
  SetCop1RegFloat(regs.cop0, FD(instr), sqrtf(fs));
}

void Cop1::sqrtd(Registers &regs, u32 instr) {
  double fs = GetCop1RegDouble(regs.cop0, FS(instr));
  SetCop1RegDouble(regs.cop0, FD(instr), sqrt(fs));
}

void Cop1::roundls(Registers& regs, u32 instr) {
  float fs = GetCop1RegFloat(regs.cop0, FS(instr));
  SetReg<u64>(regs.cop0, FD(instr), (s32)roundf(fs));
}

void Cop1::roundld(Registers& regs, u32 instr) {
  double fs = GetCop1RegDouble(regs.cop0, FS(instr));
  SetReg<u64>(regs.cop0, FD(instr), (s64)round(fs));
}

void Cop1::roundws(Registers& regs, u32 instr) {
  float fs = GetCop1RegFloat(regs.cop0, FS(instr));
  SetReg<u32>(regs.cop0, FD(instr), (s32)roundf(fs));
}

void Cop1::roundwd(Registers& regs, u32 instr) {
  double fs = GetCop1RegDouble(regs.cop0, FS(instr));
  SetReg<u32>(regs.cop0, FD(instr), (s64)round(fs));
}

void Cop1::floorls(Registers& regs, u32 instr) {
  float fs = GetCop1RegFloat(regs.cop0, FS(instr));
  SetReg<u64>(regs.cop0, FD(instr), (s64)floorf(fs));
}

void Cop1::floorld(Registers& regs, u32 instr) {
  double fs = GetCop1RegDouble(regs.cop0, FS(instr));
  SetReg<u64>(regs.cop0, FD(instr), (s64)floor(fs));
}

void Cop1::floorws(Registers& regs, u32 instr) {
  float fs = GetCop1RegFloat(regs.cop0, FS(instr));
  SetReg<u32>(regs.cop0, FD(instr), (s64)floorf(fs));
}

void Cop1::floorwd(Registers& regs, u32 instr) {
  double fs = GetCop1RegDouble(regs.cop0, FS(instr));
  SetReg<u32>(regs.cop0, FD(instr), (s64)floor(fs));
}

void Cop1::lwc1(Registers& regs, Mem& mem, u32 instr) {
  u32 addr = (s64)(s16)instr + regs.gpr[BASE(instr)];
  u32 data = mem.Read<u32>(regs, addr, regs.oldPC);
  SetReg<u32>(regs.cop0, FT(instr), data);
}

void Cop1::swc1(Registers& regs, Mem& mem, u32 instr) {
  u32 addr = (s64)(s16)instr + regs.gpr[BASE(instr)];
  mem.Write<u32>(regs, addr, GetReg<u32>(regs.cop0, FT(instr)), regs.oldPC);
}

void Cop1::ldc1(Registers& regs, Mem& mem, u32 instr) {
  u32 addr = (s64)(s16)instr + regs.gpr[BASE(instr)];
  u64 data = mem.Read<u64>(regs, addr, regs.oldPC);
  SetReg<u64>(regs.cop0, FT(instr), data);
}

void Cop1::truncws(Registers& regs, u32 instr) {
  float fs = GetCop1RegFloat(regs.cop0, FS(instr));
  s32 result = (s32)truncf(fs);
  SetReg<u32>(regs.cop0, FD(instr), result);
}

void Cop1::truncwd(Registers& regs, u32 instr) {
  double fs = GetCop1RegDouble(regs.cop0, FS(instr));
  s32 result = (s32)trunc(fs);
  SetReg<u32>(regs.cop0, FD(instr), result);
}

void Cop1::truncls(Registers& regs, u32 instr) {
  float fs = GetCop1RegFloat(regs.cop0, FS(instr));
  s64 result = (s64)truncf(fs);
  SetReg<u64>(regs.cop0, FD(instr), result);
}

void Cop1::truncld(Registers& regs, u32 instr) {
  double fs = GetCop1RegDouble(regs.cop0, FS(instr));
  s64 result = (s64)trunc(fs);
  SetReg<u64>(regs.cop0, FD(instr), result);
}

void Cop1::sdc1(Registers& regs, Mem& mem, u32 instr) {
  u32 addr = (s64)(s16)instr + regs.gpr[BASE(instr)];
  mem.Write<u64>(regs, addr, GetReg<u64>(regs.cop0, FT(instr)), regs.oldPC);
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