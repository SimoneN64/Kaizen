#pragma once
#include <JIT.hpp>
#include <Cop1.hpp>

namespace n64 {
void absd(JIT&, u32 instr);
void abss(JIT&, u32 instr);
void absw(JIT&, u32 instr);
void absl(JIT&, u32 instr);
void adds(JIT&, u32 instr);
void addd(JIT&, u32 instr);
void subs(JIT&, u32 instr);
void subd(JIT&, u32 instr);
void subw(JIT&, u32 instr);
void subl(JIT&, u32 instr);
void ceills(JIT&, u32 instr);
void ceilws(JIT&, u32 instr);
void ceilld(JIT&, u32 instr);
void ceilwd(JIT&, u32 instr);
void cfc1(JIT&, u32 instr);
void ctc1(JIT&, u32 instr);
void roundls(JIT&, u32 instr);
void roundld(JIT&, u32 instr);
void roundws(JIT&, u32 instr);
void roundwd(JIT&, u32 instr);
void floorls(JIT&, u32 instr);
void floorld(JIT&, u32 instr);
void floorws(JIT&, u32 instr);
void floorwd(JIT&, u32 instr);
void cvtls(JIT&, u32 instr);
void cvtws(JIT&, u32 instr);
void cvtds(JIT&, u32 instr);
void cvtsw(JIT&, u32 instr);
void cvtdw(JIT&, u32 instr);
void cvtsd(JIT&, u32 instr);
void cvtwd(JIT&, u32 instr);
void cvtld(JIT&, u32 instr);
void cvtdl(JIT&, u32 instr);
void cvtsl(JIT&, u32 instr);
template <typename T>
void ccond(JIT&, u32 instr, CompConds);
void divs(JIT&, u32 instr);
void divd(JIT&, u32 instr);
void muls(JIT&, u32 instr);
void muld(JIT&, u32 instr);
void mulw(JIT&, u32 instr);
void mull(JIT&, u32 instr);
void movs(JIT&, u32 instr);
void movd(JIT&, u32 instr);
void movw(JIT&, u32 instr);
void movl(JIT&, u32 instr);
void negs(JIT&, u32 instr);
void negd(JIT&, u32 instr);
void sqrts(JIT&, u32 instr);
void sqrtd(JIT&, u32 instr);
void lwc1(JIT&, u32 instr);
void swc1(JIT&, u32 instr);
void ldc1(JIT&, u32 instr);
void mfc1(JIT&, u32 instr);
void dmfc1(JIT&, u32 instr);
void mtc1(JIT&, u32 instr);
void dmtc1(JIT&, u32 instr);
void sdc1(JIT&, u32 instr);
void truncws(JIT&, u32 instr);
void truncwd(JIT&, u32 instr);
void truncls(JIT&, u32 instr);
void truncld(JIT&, u32 instr);
}