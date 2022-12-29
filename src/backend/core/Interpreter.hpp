#pragma once
#include <core/registers/Registers.hpp>
#include <Mem.hpp>
#include <vector>

namespace n64 {
struct Interpreter {
  Interpreter() = default;
  ~Interpreter() = default;
  void Step(Mem&, Registers&);
private:
  u64 cop2Latch{};
  friend struct Cop1;

  void cop2Decode(Registers&, u32);
  void special(Registers&, Mem&, u32);
  void regimm(Registers&, u32);
  void Exec(Registers&, Mem&, u32);
  void add(Registers&, u32);
  void addu(Registers&, u32);
  void addi(Registers&, u32);
  void addiu(Registers&, u32);
  void andi(Registers&, u32);
  void and_(Registers&, u32);
  void branch(Registers&, bool, s64);
  void branch_likely(Registers&, bool, s64);
  void b(Registers&, u32, bool);
  void blink(Registers&, u32, bool);
  void bl(Registers&, u32, bool);
  void bllink(Registers&, u32, bool);
  void dadd(Registers&, u32);
  void daddu(Registers&, u32);
  void daddi(Registers&, u32);
  void daddiu(Registers&, u32);
  void ddiv(Registers&, u32);
  void ddivu(Registers&, u32);
  void div(Registers&, u32);
  void divu(Registers&, u32);
  void dmult(Registers&, u32);
  void dmultu(Registers&, u32);
  void dsll(Registers&, u32);
  void dsllv(Registers&, u32);
  void dsll32(Registers&, u32);
  void dsra(Registers&, u32);
  void dsrav(Registers&, u32);
  void dsra32(Registers&, u32);
  void dsrl(Registers&, u32);
  void dsrlv(Registers&, u32);
  void dsrl32(Registers&, u32);
  void dsub(Registers&, u32);
  void dsubu(Registers&, u32);
  void j(Registers&, u32);
  void jr(Registers&, u32);
  void jal(Registers&, u32);
  void jalr(Registers&, u32);
  void lui(Registers&, u32);
  void lbu(Registers&, Mem&, u32);
  void lb(Registers&, Mem&, u32);
  void ld(Registers&, Mem&, u32);
  void ldl(Registers&, Mem&, u32);
  void ldr(Registers&, Mem&, u32);
  void lh(Registers&, Mem&, u32);
  void lhu(Registers&, Mem&, u32);
  void ll(Registers&, Mem&, u32);
  void lld(Registers&, Mem&, u32);
  void lw(Registers&, Mem&, u32);
  void lwl(Registers&, Mem&, u32);
  void lwu(Registers&, Mem&, u32);
  void lwr(Registers&, Mem&, u32);
  void mfhi(Registers&, u32);
  void mflo(Registers&, u32);
  void mult(Registers&, u32);
  void multu(Registers&, u32);
  void mthi(Registers&, u32);
  void mtlo(Registers&, u32);
  void nor(Registers&, u32);
  void sb(Registers&, Mem&, u32);
  void sc(Registers&, Mem&, u32);
  void scd(Registers&, Mem&, u32);
  void sd(Registers&, Mem&, u32);
  void sdl(Registers&, Mem&, u32);
  void sdr(Registers&, Mem&, u32);
  void sh(Registers&, Mem&, u32);
  void sw(Registers&, Mem&, u32);
  void swl(Registers&, Mem&, u32);
  void swr(Registers&, Mem&, u32);
  void slti(Registers&, u32);
  void sltiu(Registers&, u32);
  void slt(Registers&, u32);
  void sltu(Registers&, u32);
  void sll(Registers&, u32);
  void sllv(Registers&, u32);
  void sub(Registers&, u32);
  void subu(Registers&, u32);
  void sra(Registers&, u32);
  void srav(Registers&, u32);
  void srl(Registers&, u32);
  void srlv(Registers&, u32);
  void trap(Registers&, bool);
  void or_(Registers&, u32);
  void ori(Registers&, u32);
  void xor_(Registers&, u32);
  void xori(Registers&, u32);

  void mtc2(Registers&, u32);
  void mfc2(Registers&, u32);
  void dmtc2(Registers&, u32);
  void dmfc2(Registers&, u32);
  void ctc2(Registers&, u32);
  void cfc2(Registers&, u32);
};
}
