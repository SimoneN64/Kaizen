#include <n64/core/cpu/registers/Cop1.hpp>
#include <n64/core/cpu/Registers.hpp>

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

}

void Cop1::absl(Registers& regs, u32 instr) {

}

void Cop1::adds(Registers& regs, u32 instr) {

}

void Cop1::addd(Registers& regs, u32 instr) {

}

void Cop1::subs(Registers& regs, u32 instr) {

}

void Cop1::subd(Registers& regs, u32 instr) {

}

void Cop1::subw(Registers& regs, u32 instr) {

}

void Cop1::subl(Registers& regs, u32 instr) {

}

void Cop1::ceills(Registers& regs, u32 instr) {

}

void Cop1::ceilws(Registers& regs, u32 instr) {

}

void Cop1::ceilld(Registers& regs, u32 instr) {

}

void Cop1::ceilwd(Registers& regs, u32 instr) {

}

void Cop1::cfc1(Registers& regs, u32 instr) {

}

void Cop1::ctc1(Registers& regs, u32 instr) {

}

void Cop1::roundls(Registers& regs, u32 instr) {

}

void Cop1::roundld(Registers& regs, u32 instr) {

}

void Cop1::roundws(Registers& regs, u32 instr) {

}

void Cop1::roundwd(Registers& regs, u32 instr) {

}

void Cop1::floorls(Registers& regs, u32 instr) {

}

void Cop1::floorld(Registers& regs, u32 instr) {

}

void Cop1::floorws(Registers& regs, u32 instr) {

}

void Cop1::floorwd(Registers& regs, u32 instr) {

}

void Cop1::cvtls(Registers& regs, u32 instr) {

}

void Cop1::cvtws(Registers& regs, u32 instr) {

}

void Cop1::cvtds(Registers& regs, u32 instr) {

}

void Cop1::cvtsw(Registers& regs, u32 instr) {

}

void Cop1::cvtdw(Registers& regs, u32 instr) {

}

void Cop1::cvtsd(Registers& regs, u32 instr) {

}

void Cop1::cvtwd(Registers& regs, u32 instr) {

}

void Cop1::cvtld(Registers& regs, u32 instr) {

}

void Cop1::cvtdl(Registers& regs, u32 instr) {

}

void Cop1::cvtsl(Registers& regs, u32 instr) {

}

void Cop1::ccondd(Registers& regs, u32 instr, CompConds cond) {

}

void Cop1::cconds(Registers& regs, u32 instr, CompConds cond) {

}

void Cop1::divs(Registers& regs, u32 instr) {

}

void Cop1::divd(Registers& regs, u32 instr) {

}

void Cop1::muls(Registers& regs, u32 instr) {

}

void Cop1::muld(Registers& regs, u32 instr) {

}

void Cop1::mulw(Registers& regs, u32 instr) {

}

void Cop1::mull(Registers& regs, u32 instr) {

}

void Cop1::movs(Registers& regs, u32 instr) {

}

void Cop1::movd(Registers& regs, u32 instr) {

}

void Cop1::movw(Registers& regs, u32 instr) {

}

void Cop1::movl(Registers& regs, u32 instr) {

}

void Cop1::negs(Registers& regs, u32 instr) {

}

void Cop1::negd(Registers& regs, u32 instr) {

}

void Cop1::sqrts(Registers& regs, u32 instr) {

}

void Cop1::sqrtd(Registers& regs, u32 instr) {

}

void Cop1::lwc1(Registers& regs, Mem& mem, u32 instr) {

}

void Cop1::swc1(Registers& regs, Mem& mem, u32 instr) {

}

void Cop1::ldc1(Registers& regs, Mem& mem, u32 instr) {

}

void Cop1::mfc1(Registers& regs, u32 instr) {

}

void Cop1::dmfc1(Registers& regs, u32 instr) {

}

void Cop1::mtc1(Registers& regs, u32 instr) {

}

void Cop1::dmtc1(Registers& regs, u32 instr) {

}

void Cop1::sdc1(Registers& regs, Mem&, u32 instr) {

}

void Cop1::truncws(Registers& regs, u32 instr) {

}

void Cop1::truncwd(Registers& regs, u32 instr) {

}

void Cop1::truncls(Registers& regs, u32 instr) {

}

void Cop1::truncld(Registers& regs, u32 instr) {

}

}