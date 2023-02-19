#pragma once
#include <Dynarec.hpp>
#include <Cop1.hpp>

namespace n64 {
void absd(Dynarec&, u32 instr);
void abss(Dynarec&, u32 instr);
void absw(Dynarec&, u32 instr);
void absl(Dynarec&, u32 instr);
void adds(Dynarec&, u32 instr);
void addd(Dynarec&, u32 instr);
void subs(Dynarec&, u32 instr);
void subd(Dynarec&, u32 instr);
void subw(Dynarec&, u32 instr);
void subl(Dynarec&, u32 instr);
void ceills(Dynarec&, u32 instr);
void ceilws(Dynarec&, u32 instr);
void ceilld(Dynarec&, u32 instr);
void ceilwd(Dynarec&, u32 instr);
void cfc1(Dynarec&, u32 instr);
void ctc1(Dynarec&, u32 instr);
void roundls(Dynarec&, u32 instr);
void roundld(Dynarec&, u32 instr);
void roundws(Dynarec&, u32 instr);
void roundwd(Dynarec&, u32 instr);
void floorls(Dynarec&, u32 instr);
void floorld(Dynarec&, u32 instr);
void floorws(Dynarec&, u32 instr);
void floorwd(Dynarec&, u32 instr);
void cvtls(Dynarec&, u32 instr);
void cvtws(Dynarec&, u32 instr);
void cvtds(Dynarec&, u32 instr);
void cvtsw(Dynarec&, u32 instr);
void cvtdw(Dynarec&, u32 instr);
void cvtsd(Dynarec&, u32 instr);
void cvtwd(Dynarec&, u32 instr);
void cvtld(Dynarec&, u32 instr);
void cvtdl(Dynarec&, u32 instr);
void cvtsl(Dynarec&, u32 instr);
template <typename T>
void ccond(Dynarec&, u32 instr, CompConds);
void divs(Dynarec&, u32 instr);
void divd(Dynarec&, u32 instr);
void muls(Dynarec&, u32 instr);
void muld(Dynarec&, u32 instr);
void mulw(Dynarec&, u32 instr);
void mull(Dynarec&, u32 instr);
void movs(Dynarec&, u32 instr);
void movd(Dynarec&, u32 instr);
void movw(Dynarec&, u32 instr);
void movl(Dynarec&, u32 instr);
void negs(Dynarec&, u32 instr);
void negd(Dynarec&, u32 instr);
void sqrts(Dynarec&, u32 instr);
void sqrtd(Dynarec&, u32 instr);
void lwc1(Dynarec&, u32 instr);
void swc1(Dynarec&, u32 instr);
void ldc1(Dynarec&, u32 instr);
void mfc1(Dynarec&, u32 instr);
void dmfc1(Dynarec&, u32 instr);
void mtc1(Dynarec&, u32 instr);
void dmtc1(Dynarec&, u32 instr);
void sdc1(Dynarec&, u32 instr);
void truncws(Dynarec&, u32 instr);
void truncwd(Dynarec&, u32 instr);
void truncls(Dynarec&, u32 instr);
void truncld(Dynarec&, u32 instr);
}