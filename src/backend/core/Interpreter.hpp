#pragma once
#include <Mem.hpp>
#include <vector>
#include <BaseCPU.hpp>

namespace n64 {
struct Core;
struct Interpreter final : BaseCPU {
  explicit Interpreter(ParallelRDP&);
  ~Interpreter() override = default;
  int Step() override;
  void Reset() override {
    regs.Reset();
    mem.Reset();
    cop2Latch = {};
  }

  Mem& GetMem() override {
    return mem;
  }

  Registers& GetRegs() override {
    return regs;
  }
private:
  Registers regs;
  Mem mem;
  u64 cop2Latch{};
  friend struct Cop1;
#define check_address_error(mask, vaddr) (((!regs.cop0.is64BitAddressing) && (s32)(vaddr) != (vaddr)) || (((vaddr) & (mask)) != 0))
  bool ShouldServiceInterrupt() override;
  void CheckCompareInterrupt() override;
  std::vector<u8> Serialize() override;
  void Deserialize(const std::vector<u8>&) override;

  void cop2Decode(u32);
  void special(u32);
  void regimm(u32);
  void Exec(u32);
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
  void lbu(u32);
  void lb(u32);
  void ld(u32);
  void ldl(u32);
  void ldr(u32);
  void lh(u32);
  void lhu(u32);
  void ll(u32);
  void lld(u32);
  void lw(u32);
  void lwl(u32);
  void lwu(u32);
  void lwr(u32);
  void mfhi(u32);
  void mflo(u32);
  void mult(u32);
  void multu(u32);
  void mthi(u32);
  void mtlo(u32);
  void nor(u32);
  void sb(u32);
  void sc(u32);
  void scd(u32);
  void sd(u32);
  void sdl(u32);
  void sdr(u32);
  void sh(u32);
  void sw(u32);
  void swl(u32);
  void swr(u32);
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
