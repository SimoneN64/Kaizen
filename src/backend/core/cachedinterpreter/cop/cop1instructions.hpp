#pragma once
#include <CachedInterpreter.hpp>
#include <Cop1.hpp>

namespace n64 {
void absd(CachedInterpreter&, u32 instr);
void abss(CachedInterpreter&, u32 instr);
void absw(CachedInterpreter&, u32 instr);
void absl(CachedInterpreter&, u32 instr);
void adds(CachedInterpreter&, u32 instr);
void addd(CachedInterpreter&, u32 instr);
void subs(CachedInterpreter&, u32 instr);
void subd(CachedInterpreter&, u32 instr);
void subw(CachedInterpreter&, u32 instr);
void subl(CachedInterpreter&, u32 instr);
void ceills(CachedInterpreter&, u32 instr);
void ceilws(CachedInterpreter&, u32 instr);
void ceilld(CachedInterpreter&, u32 instr);
void ceilwd(CachedInterpreter&, u32 instr);
void cfc1(CachedInterpreter&, u32 instr);
void ctc1(CachedInterpreter&, u32 instr);
void roundls(CachedInterpreter&, u32 instr);
void roundld(CachedInterpreter&, u32 instr);
void roundws(CachedInterpreter&, u32 instr);
void roundwd(CachedInterpreter&, u32 instr);
void floorls(CachedInterpreter&, u32 instr);
void floorld(CachedInterpreter&, u32 instr);
void floorws(CachedInterpreter&, u32 instr);
void floorwd(CachedInterpreter&, u32 instr);
void cvtls(CachedInterpreter&, u32 instr);
void cvtws(CachedInterpreter&, u32 instr);
void cvtds(CachedInterpreter&, u32 instr);
void cvtsw(CachedInterpreter&, u32 instr);
void cvtdw(CachedInterpreter&, u32 instr);
void cvtsd(CachedInterpreter&, u32 instr);
void cvtwd(CachedInterpreter&, u32 instr);
void cvtld(CachedInterpreter&, u32 instr);
void cvtdl(CachedInterpreter&, u32 instr);
void cvtsl(CachedInterpreter&, u32 instr);
template <typename T>
void ccond(CachedInterpreter&, u32 instr, CompConds);
void divs(CachedInterpreter&, u32 instr);
void divd(CachedInterpreter&, u32 instr);
void muls(CachedInterpreter&, u32 instr);
void muld(CachedInterpreter&, u32 instr);
void mulw(CachedInterpreter&, u32 instr);
void mull(CachedInterpreter&, u32 instr);
void movs(CachedInterpreter&, u32 instr);
void movd(CachedInterpreter&, u32 instr);
void movw(CachedInterpreter&, u32 instr);
void movl(CachedInterpreter&, u32 instr);
void negs(CachedInterpreter&, u32 instr);
void negd(CachedInterpreter&, u32 instr);
void sqrts(CachedInterpreter&, u32 instr);
void sqrtd(CachedInterpreter&, u32 instr);
void lwc1(CachedInterpreter&, u32 instr);
void swc1(CachedInterpreter&, u32 instr);
void ldc1(CachedInterpreter&, u32 instr);
void mfc1(CachedInterpreter&, u32 instr);
void dmfc1(CachedInterpreter&, u32 instr);
void mtc1(CachedInterpreter&, u32 instr);
void dmtc1(CachedInterpreter&, u32 instr);
void sdc1(CachedInterpreter&, u32 instr);
void truncws(CachedInterpreter&, u32 instr);
void truncwd(CachedInterpreter&, u32 instr);
void truncls(CachedInterpreter&, u32 instr);
void truncld(CachedInterpreter&, u32 instr);
}