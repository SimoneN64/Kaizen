#pragma once
#include <Dynarec.hpp>
#include <Cop1.hpp>

namespace n64::JIT {
void absd(n64::Registers&, u32 instr);
void abss(n64::Registers&, u32 instr);
void absw(n64::Registers&, u32 instr);
void absl(n64::Registers&, u32 instr);
void adds(n64::Registers&, u32 instr);
void addd(n64::Registers&, u32 instr);
void subs(n64::Registers&, u32 instr);
void subd(n64::Registers&, u32 instr);
void subw(n64::Registers&, u32 instr);
void subl(n64::Registers&, u32 instr);
void ceills(n64::Registers&, u32 instr);
void ceilws(n64::Registers&, u32 instr);
void ceilld(n64::Registers&, u32 instr);
void ceilwd(n64::Registers&, u32 instr);
void cfc1(n64::Registers&, u32 instr);
void ctc1(n64::Registers&, u32 instr);
void roundls(n64::Registers&, u32 instr);
void roundld(n64::Registers&, u32 instr);
void roundws(n64::Registers&, u32 instr);
void roundwd(n64::Registers&, u32 instr);
void floorls(n64::Registers&, u32 instr);
void floorld(n64::Registers&, u32 instr);
void floorws(n64::Registers&, u32 instr);
void floorwd(n64::Registers&, u32 instr);
void cvtls(n64::Registers&, u32 instr);
void cvtws(n64::Registers&, u32 instr);
void cvtds(n64::Registers&, u32 instr);
void cvtsw(n64::Registers&, u32 instr);
void cvtdw(n64::Registers&, u32 instr);
void cvtsd(n64::Registers&, u32 instr);
void cvtwd(n64::Registers&, u32 instr);
void cvtld(n64::Registers&, u32 instr);
void cvtdl(n64::Registers&, u32 instr);
void cvtsl(n64::Registers&, u32 instr);
template <typename T>
void ccond(n64::Registers&, u32 instr, CompConds);
void divs(n64::Registers&, u32 instr);
void divd(n64::Registers&, u32 instr);
void muls(n64::Registers&, u32 instr);
void muld(n64::Registers&, u32 instr);
void mulw(n64::Registers&, u32 instr);
void mull(n64::Registers&, u32 instr);
void movs(n64::Registers&, u32 instr);
void movd(n64::Registers&, u32 instr);
void movw(n64::Registers&, u32 instr);
void movl(n64::Registers&, u32 instr);
void negs(n64::Registers&, u32 instr);
void negd(n64::Registers&, u32 instr);
void sqrts(n64::Registers&, u32 instr);
void sqrtd(n64::Registers&, u32 instr);
void lwc1(n64::Registers&, Mem&, u32 instr);
void swc1(n64::Registers&, Mem&, u32 instr);
void ldc1(n64::Registers&, Mem&, u32 instr);
void mfc1(n64::Registers&, u32 instr);
void dmfc1(n64::Registers&, u32 instr);
void mtc1(n64::Registers&, u32 instr);
void dmtc1(n64::Registers&, u32 instr);
void sdc1(n64::Registers&, Mem&, u32 instr);
void truncws(n64::Registers&, u32 instr);
void truncwd(n64::Registers&, u32 instr);
void truncls(n64::Registers&, u32 instr);
void truncld(n64::Registers&, u32 instr);
}