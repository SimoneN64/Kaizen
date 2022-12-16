#pragma once
#include <core/registers/Registers.hpp>
#include <Mem.hpp>
#include <capstone/capstone.h>
#include <vector>

namespace n64 {
struct Interpreter {
  Interpreter() {
    if(cs_open(CS_ARCH_MIPS, CS_MODE_MIPS64, &handle) != CS_ERR_OK) {
      util::panic("Could not initialize capstone!\n");
    }
    Reset();
  }

  ~Interpreter() {
    cs_close(&handle);
  }
  void Reset();
  void Step(Mem&);
  Registers regs;
private:
  u64 cop2Latch{};
  csh handle;

  void disassembly(u32 instr);
  friend struct Cop1;

  void cop2Decode(u32);
  void special(Mem&, u32);
  void regimm(u32);
  void Exec(Mem&, u32);
  void add(u32);
  void addu(u32);
  void addi(u32);
  void addiu(u32);
  void andi(u32);
  void and_(u32);
  void branch(bool, s64);
  void branch_likely(bool, s64);
  void b(u32, bool);
  void blink(u32, bool);
  void bl(u32, bool);
  void bllink(u32, bool);
  void dadd(u32);
  void daddu(u32);
  void daddi(u32);
  void daddiu(u32);
  void ddiv(u32);
  void ddivu(u32);
  void div(u32);
  void divu(u32);
  void dmult(u32);
  void dmultu(u32);
  void dsll(u32);
  void dsllv(u32);
  void dsll32(u32);
  void dsra(u32);
  void dsrav(u32);
  void dsra32(u32);
  void dsrl(u32);
  void dsrlv(u32);
  void dsrl32(u32);
  void dsub(u32);
  void dsubu(u32);
  void j(u32);
  void jr(u32);
  void jal(u32);
  void jalr(u32);
  void lui(u32);
  void lbu(Mem&, u32);
  void lb(Mem&, u32);
  void ld(Mem&, u32);
  void ldl(Mem&, u32);
  void ldr(Mem&, u32);
  void lh(Mem&, u32);
  void lhu(Mem&, u32);
  void ll(Mem&, u32);
  void lld(Mem&, u32);
  void lw(Mem&, u32);
  void lwl(Mem&, u32);
  void lwu(Mem&, u32);
  void lwr(Mem&, u32);
  void mfhi(u32);
  void mflo(u32);
  void mult(u32);
  void multu(u32);
  void mthi(u32);
  void mtlo(u32);
  void nor(u32);
  void sb(Mem&, u32);
  void sc(Mem&, u32);
  void scd(Mem&, u32);
  void sd(Mem&, u32);
  void sdl(Mem&, u32);
  void sdr(Mem&, u32);
  void sh(Mem&, u32);
  void sw(Mem&, u32);
  void swl(Mem&, u32);
  void swr(Mem&, u32);
  void slti(u32);
  void sltiu(u32);
  void slt(u32);
  void sltu(u32);
  void sll(u32);
  void sllv(u32);
  void sub(u32);
  void subu(u32);
  void sra(u32);
  void srav(u32);
  void srl(u32);
  void srlv(u32);
  void trap(bool);
  void or_(u32);
  void ori(u32);
  void xor_(u32);
  void xori(u32);

  void mtc2(u32);
  void mfc2(u32);
  void dmtc2(u32);
  void dmfc2(u32);
  void ctc2(u32);
  void cfc2(u32);
};
}
