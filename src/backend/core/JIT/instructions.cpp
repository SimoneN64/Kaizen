#include <core/JIT.hpp>

#define check_address_error(mask, vaddr) (((!regs.cop0.is_64bit_addressing) && (s32)(vaddr) != (vaddr)) || (((vaddr) & (mask)) != 0))
#define check_signed_overflow(op1, op2, res) (((~((op1) ^ (op2)) & ((op1) ^ (res))) >> ((sizeof(res) * 8) - 1)) & 1)
#define check_signed_underflow(op1, op2, res) (((((op1) ^ (op2)) & ((op1) ^ (res))) >> ((sizeof(res) * 8) - 1)) & 1)

namespace n64 {
void JIT::add(u32 instr) {
  auto dst = Entry::Operand{ Entry::Operand::REG_S64, u8(RD(instr)) };
  auto op1 = Entry::Operand{ Entry::Operand::REG_U32, u8(RS(instr)) };
  auto op2 = Entry::Operand{ Entry::Operand::REG_U32, u8(RT(instr)) };
  Entry e(Entry::ADD, dst, op1, op2);
  ir.push(e);
}

void JIT::addu(u32 instr) {
  add(instr);
}

void JIT::addi(u32 instr) {
  auto dst = Entry::Operand{ Entry::Operand::REG_S64, u8(RT(instr)) };
  auto op1 = Entry::Operand{ Entry::Operand::REG_U32, u8(RS(instr)) };
  auto op2 = Entry::Operand{ Entry::Operand::IMM_S16, s16(instr) };
  Entry e(Entry::ADD, dst, op1, op2);
  ir.push(e);
}

void JIT::addiu(u32 instr) {
  addi(instr);
}

void JIT::dadd(u32 instr) {
  auto dst = Entry::Operand{ Entry::Operand::REG_S64, u8(RD(instr)) };
  auto op1 = Entry::Operand{ Entry::Operand::REG_U64, u8(RS(instr)) };
  auto op2 = Entry::Operand{ Entry::Operand::REG_U64, u8(RT(instr)) };
  Entry e(Entry::ADD, dst, op1, op2);
  ir.push(e);
}

Entry JIT::branch(u32 instr) {
  auto dst = Entry::Operand{ Entry::Operand::IMM_S64, u64(s64(s16(instr))) << 2 };
  auto pc = Entry::Operand{Entry::Operand::PC64};
  Entry add_(Entry::ADD, dst, dst, pc);
  ir.push(add_);
  return add_;
}

void JIT::bltz(u32 instr) {
  auto dst = branch(instr);
  auto op1 = Entry::Operand{ Entry::Operand::REG_S64, u8(RS(instr)) };
  auto op2 = Entry::Operand{ Entry::Operand::IMM_U64, 0 };
  Entry e(Entry::BRANCH, dst.GetDst(), op1, Entry::BranchCond::LT, op2);
  ir.push(e);
}

void JIT::bgez(u32 instr) {
  auto dst = branch(instr);
  auto op1 = Entry::Operand{ Entry::Operand::REG_S64, u8(RS(instr)) };
  auto op2 = Entry::Operand{ Entry::Operand::IMM_U64, 0 };
  Entry e(Entry::BRANCH, dst.GetDst(), op1, Entry::BranchCond::GE, op2);
  ir.push(e);
}

void JIT::bltzl(u32 instr) {
  auto dst = branch(instr);
  auto op1 = Entry::Operand{ Entry::Operand::REG_S64, u8(RS(instr)) };
  auto op2 = Entry::Operand{ Entry::Operand::IMM_U64, 0 };
  auto opc = Entry::Opcode(u16(Entry::BRANCH) | Entry::LIKELY);
  Entry e(opc, dst.GetDst(), op1, Entry::BranchCond::LT, op2);
  ir.push(e);
}

void JIT::bgezl(u32 instr) {
  auto dst = branch(instr);
  auto op1 = Entry::Operand{ Entry::Operand::REG_S64, u8(RS(instr)) };
  auto op2 = Entry::Operand{ Entry::Operand::IMM_U64, 0 };
  auto opc = Entry::Opcode(u16(Entry::BRANCH) | Entry::LIKELY);
  Entry e(opc, dst.GetDst(), op1, Entry::BranchCond::GE, op2);
  ir.push(e);
}

void JIT::bltzal(u32 instr) {
  auto dst = branch(instr);
  auto op1 = Entry::Operand{ Entry::Operand::REG_S64, u8(RS(instr)) };
  auto op2 = Entry::Operand{ Entry::Operand::IMM_U64, 0 };
  auto opc = Entry::Opcode(u16(Entry::BRANCH) | Entry::LINK);
  Entry e(opc, dst.GetDst(), op1, Entry::BranchCond::LT, op2);
  ir.push(e);
}

void JIT::bgezal(u32 instr) {
  auto dst = branch(instr);
  auto op1 = Entry::Operand{ Entry::Operand::REG_S64, u8(RS(instr)) };
  auto op2 = Entry::Operand{ Entry::Operand::IMM_U64, 0 };
  auto opc = Entry::Opcode(u16(Entry::BRANCH) | Entry::LINK);
  Entry e(opc, dst.GetDst(), op1, Entry::BranchCond::GE, op2);
  ir.push(e);
}

void JIT::bltzall(u32 instr) {
  auto dst = branch(instr);
  auto op1 = Entry::Operand{ Entry::Operand::REG_S64, u8(RS(instr)) };
  auto op2 = Entry::Operand{ Entry::Operand::IMM_U64, 0 };
  auto opc = Entry::Opcode(u16(Entry::BRANCH) | Entry::LINK | Entry::LIKELY);
  Entry e(opc, dst.GetDst(), op1, Entry::BranchCond::LT, op2);
  ir.push(e);
}

void JIT::bgezall(u32 instr) {
  auto dst = branch(instr);
  auto op1 = Entry::Operand{ Entry::Operand::REG_S64, u8(RS(instr)) };
  auto op2 = Entry::Operand{ Entry::Operand::IMM_U64, 0 };
  auto opc = Entry::Opcode(u16(Entry::BRANCH) | Entry::LINK | Entry::LIKELY);
  Entry e(opc, dst.GetDst(), op1, Entry::BranchCond::GE, op2);
  ir.push(e);
}

void JIT::beq(u32 instr) {
  auto dst = branch(instr);
  auto op1 = Entry::Operand{ Entry::Operand::REG_S64, u8(RS(instr)) };
  auto op2 = Entry::Operand{ Entry::Operand::REG_S64, u8(RT(instr)) };
  Entry e(Entry::BRANCH, dst.GetDst(), op1, Entry::BranchCond::EQ, op2);
  ir.push(e);
}

void JIT::bne(u32 instr) {
  auto dst = branch(instr);
  auto op1 = Entry::Operand{ Entry::Operand::REG_S64, u8(RS(instr)) };
  auto op2 = Entry::Operand{ Entry::Operand::REG_S64, u8(RT(instr)) };
  Entry e(Entry::BRANCH, dst.GetDst(), op1, Entry::BranchCond::NE, op2);
  ir.push(e);
}

void JIT::blez(u32 instr) {
  auto dst = branch(instr);
  auto op1 = Entry::Operand{ Entry::Operand::REG_S64, u8(RS(instr)) };
  auto op2 = Entry::Operand{ Entry::Operand::IMM_S64, 0 };
  Entry e(Entry::BRANCH, dst.GetDst(), op1, Entry::BranchCond::LE, op2);
  ir.push(e);
}

void JIT::bgtz(u32 instr) {
  auto dst = branch(instr);
  auto op1 = Entry::Operand{ Entry::Operand::REG_S64, u8(RS(instr)) };
  auto op2 = Entry::Operand{ Entry::Operand::IMM_S64, 0 };
  Entry e(Entry::BRANCH, dst.GetDst(), op1, Entry::BranchCond::GT, op2);
  ir.push(e);
}

void JIT::beql(u32 instr) {
  auto dst = branch(instr);
  auto op1 = Entry::Operand{ Entry::Operand::REG_S64, u8(RS(instr)) };
  auto op2 = Entry::Operand{ Entry::Operand::REG_S64, u8(RT(instr)) };
  auto opc = Entry::Opcode(u16(Entry::BRANCH) | Entry::LIKELY);
  Entry e(opc, dst.GetDst(), op1, Entry::BranchCond::EQ, op2);
  ir.push(e);
}

void JIT::bnel(u32 instr) {
  auto dst = branch(instr);
  auto op1 = Entry::Operand{ Entry::Operand::REG_S64, u8(RS(instr)) };
  auto op2 = Entry::Operand{ Entry::Operand::REG_S64, u8(RT(instr)) };
  auto opc = Entry::Opcode(u16(Entry::BRANCH) | Entry::LIKELY);
  Entry e(opc, dst.GetDst(), op1, Entry::BranchCond::NE, op2);
  ir.push(e);
}

void JIT::blezl(u32 instr) {
  auto dst = branch(instr);
  auto op1 = Entry::Operand{ Entry::Operand::REG_S64, u8(RS(instr)) };
  auto op2 = Entry::Operand{ Entry::Operand::IMM_S64, 0 };
  auto opc = Entry::Opcode(u16(Entry::BRANCH) | Entry::LIKELY);
  Entry e(opc, dst.GetDst(), op1, Entry::BranchCond::LE, op2);
  ir.push(e);
}

void JIT::bgtzl(u32 instr) {
  auto dst = branch(instr);
  auto op1 = Entry::Operand{ Entry::Operand::REG_S64, u8(RS(instr)) };
  auto op2 = Entry::Operand{ Entry::Operand::IMM_S64, 0 };
  auto opc = Entry::Opcode(u16(Entry::BRANCH) | Entry::LIKELY);
  Entry e(opc, dst.GetDst(), op1, Entry::BranchCond::GT, op2);
  ir.push(e);
}

void JIT::daddu(u32 instr) {
  dadd(instr);
}

void JIT::daddi(u32 instr) {
  auto dst = Entry::Operand{ Entry::Operand::REG_S64, u8(RT(instr)) };
  auto op1 = Entry::Operand{ Entry::Operand::REG_S64, u8(RS(instr)) };
  auto op2 = Entry::Operand{ Entry::Operand::IMM_S16, s16(instr) };
  Entry e(Entry::ADD, dst, op1, op2);
  ir.push(e);
}

void JIT::daddiu(u32 instr) {
  daddi(instr);
}

void JIT::div(u32 instr) {
  auto op1 = Entry::Operand{ Entry::Operand::REG_S32, u8(RS(instr)) };
  auto op2 = Entry::Operand{ Entry::Operand::REG_S32, u8(RT(instr)) };
  Entry e(Entry::DIV, op1, op2);
  ir.push(e);
}

void JIT::divu(u32 instr) {
  auto op1 = Entry::Operand{ Entry::Operand::REG_U32, u8(RS(instr)) };
  auto op2 = Entry::Operand{ Entry::Operand::REG_U32, u8(RT(instr)) };
  Entry e(Entry::DIV, op1, op2);
  ir.push(e);
}

void JIT::ddiv(u32 instr) {
  auto op1 = Entry::Operand{ Entry::Operand::REG_S64, u8(RS(instr)) };
  auto op2 = Entry::Operand{ Entry::Operand::REG_S64, u8(RT(instr)) };
  Entry e(Entry::DIV, op1, op2);
  ir.push(e);
}

void JIT::ddivu(u32 instr) {
  auto op1 = Entry::Operand{ Entry::Operand::REG_U64, u8(RS(instr)) };
  auto op2 = Entry::Operand{ Entry::Operand::REG_U64, u8(RT(instr)) };
  Entry e(Entry::DIV, op1, op2);
  ir.push(e);
}

void JIT::lui(u32 instr) {
  auto dst = Entry::Operand{ Entry::Operand::REG_S64, u8(RT(instr)) };
  auto op1 = Entry::Operand{ Entry::Operand::IMM_S16, u64(s16(instr)) << 16 };
  Entry e(Entry::LOADS64, dst, op1);
  ir.push(e);
}

void JIT::lb(u32 instr) {
  auto dst = Entry::Operand{ Entry::Operand::REG_S64, u8(RT(instr)) };
  auto op1 = Entry::Operand{ Entry::Operand::MEM_U8, s16(instr) };
  auto op2 = Entry::Operand{ Entry::Operand::REG_S64, u8(RS(instr))};
  Entry e(Entry::LOADS8, dst, op1, op2);
  ir.push(e);
}

void JIT::lh(u32 instr) {
  auto dst = Entry::Operand{ Entry::Operand::REG_S64, u8(RT(instr)) };
  auto op1 = Entry::Operand{ Entry::Operand::MEM_U16, s16(instr) };
  auto op2 = Entry::Operand{ Entry::Operand::REG_S64, u8(RS(instr)) };
  Entry e(Entry::LOADS16, dst, op1, op2);
  ir.push(e);
}

void JIT::lw(u32 instr) {
  auto dst = Entry::Operand{ Entry::Operand::REG_S64, u8(RT(instr)) };
  auto op1 = Entry::Operand{ Entry::Operand::MEM_U32, s16(instr) };
  auto op2 = Entry::Operand{ Entry::Operand::REG_S64, u8(RS(instr)) };
  Entry e(Entry::LOADS32, dst, op1, op2);
  ir.push(e);
}

void JIT::ll(u32 instr) {
  auto dst = Entry::Operand{ Entry::Operand::REG_S64, u8(RT(instr)) };
  auto op1 = Entry::Operand{ Entry::Operand::MEM_U32, s16(instr) };
  auto op2 = Entry::Operand{ Entry::Operand::REG_S64, u8(RS(instr)) };
  auto opc = Entry::Opcode(u16(Entry::LOADS64) | Entry::SET_LLBIT);
  Entry e(opc, dst, op1, op2);
  ir.push(e);
}

void JIT::lwl(u32 instr) {
  auto dst = Entry::Operand{ Entry::Operand::REG_S64, u8(RT(instr)) };
  auto op1 = Entry::Operand{ Entry::Operand::MEM_U32, s16(instr) };
  auto op2 = Entry::Operand{ Entry::Operand::REG_S64, u8(RS(instr)) };
  Entry e(Entry::LOADS32_SHIFT, dst, op1, op2, Entry::LEFT);
  ir.push(e);
}

void JIT::lwr(u32 instr) {
  auto dst = Entry::Operand{ Entry::Operand::REG_S64, u8(RT(instr)) };
  auto op1 = Entry::Operand{ Entry::Operand::MEM_U32, s16(instr) };
  auto op2 = Entry::Operand{ Entry::Operand::REG_S64, u8(RS(instr)) };
  Entry e(Entry::LOADS32_SHIFT, dst, op1, op2, Entry::RIGHT);
  ir.push(e);
}

void JIT::ld(u32 instr) {
  auto dst = Entry::Operand{ Entry::Operand::REG_S64, u8(RT(instr)) };
  auto op1 = Entry::Operand{ Entry::Operand::MEM_U64, s16(instr) };
  auto op2 = Entry::Operand{ Entry::Operand::REG_S64, u8(RS(instr)) };
  Entry e(Entry::LOADS64, dst, op1, op2);
  ir.push(e);
}

void JIT::lld(u32 instr) {
  auto dst = Entry::Operand{ Entry::Operand::REG_S64, u8(RT(instr)) };
  auto op1 = Entry::Operand{ Entry::Operand::MEM_U32, s16(instr) };
  auto op2 = Entry::Operand{ Entry::Operand::REG_S64, u8(RS(instr)) };
  auto opc = Entry::Opcode(u16(Entry::Opcode::LOADS64) | Entry::SET_LLBIT);
  Entry e(opc, dst, op1, op2);
  ir.push(e);
}

void JIT::ldl(u32 instr) {
  auto dst = Entry::Operand{ Entry::Operand::REG_S64, u8(RT(instr)) };
  auto op1 = Entry::Operand{ Entry::Operand::MEM_U32, s16(instr) };
  auto op2 = Entry::Operand{ Entry::Operand::REG_S64, u8(RS(instr)) };
  Entry e(Entry::LOADS64_SHIFT, dst, op1, op2, Entry::LEFT);
  ir.push(e);
}

void JIT::ldr(u32 instr) {
  auto dst = Entry::Operand{ Entry::Operand::REG_S64, u8(RT(instr)) };
  auto op1 = Entry::Operand{ Entry::Operand::MEM_U32, s16(instr) };
  auto op2 = Entry::Operand{ Entry::Operand::REG_S64, u8(RS(instr)) };
  Entry e(Entry::LOADS64_SHIFT, dst, op1, op2, Entry::RIGHT);
  ir.push(e);
}

void JIT::lbu(u32 instr) {
  auto dst = Entry::Operand{ Entry::Operand::REG_S64, u8(RT(instr)) };
  auto op1 = Entry::Operand{ Entry::Operand::MEM_U8, s16(instr) };
  auto op2 = Entry::Operand{ Entry::Operand::REG_S64, u8(RS(instr)) };
  Entry e(Entry::LOADU8, dst, op1, op2);
  ir.push(e);
}

void JIT::lhu(u32 instr) {
  auto dst = Entry::Operand{ Entry::Operand::REG_S64, u8(RT(instr)) };
  auto op1 = Entry::Operand{ Entry::Operand::MEM_U16, s16(instr) };
  auto op2 = Entry::Operand{ Entry::Operand::REG_S64, u8(RS(instr)) };
  Entry e(Entry::LOADU16, dst, op1, op2);
  ir.push(e);
}

void JIT::lwu(u32 instr) {
  auto dst = Entry::Operand{ Entry::Operand::REG_S64, u8(RT(instr)) };
  auto op1 = Entry::Operand{ Entry::Operand::MEM_U32, s16(instr) };
  auto op2 = Entry::Operand{ Entry::Operand::REG_S64, u8(RS(instr)) };
  Entry e(Entry::LOADU16, dst, op1, op2);
  ir.push(e);
}

void JIT::sb(u32 instr) {
  auto dst = Entry::Operand{ Entry::Operand::REG_S64, u8(RT(instr)) };
  auto op1 = Entry::Operand{ Entry::Operand::MEM_U8, s16(instr) };
  auto op2 = Entry::Operand{ Entry::Operand::REG_S64, u8(RS(instr)) };
  Entry e(Entry::STORE8, dst, op1, op2);
  ir.push(e);
}

void JIT::sc(u32 instr) {
  auto dst = Entry::Operand{ Entry::Operand::REG_S64, u8(RT(instr)) };
  auto op1 = Entry::Operand{ Entry::Operand::MEM_U32, s16(instr) };
  auto op2 = Entry::Operand{ Entry::Operand::REG_S64, u8(RS(instr)) };
  auto opc = Entry::Opcode(u16(Entry::STORE32) | Entry::SET_LLBIT);
  Entry e(opc, dst, op1, op2);
  ir.push(e);
}

void JIT::scd(u32 instr) {
  auto dst = Entry::Operand{ Entry::Operand::REG_S64, u8(RT(instr)) };
  auto op1 = Entry::Operand{ Entry::Operand::MEM_U32, s16(instr) };
  auto op2 = Entry::Operand{ Entry::Operand::REG_S64, u8(RS(instr)) };
  auto opc = Entry::Opcode(u16(Entry::STORE64) | Entry::SET_LLBIT);
  Entry e(opc, dst, op1, op2);
  ir.push(e);
}

void JIT::sh(u32 instr) {
  auto dst = Entry::Operand{ Entry::Operand::REG_S64, u8(RT(instr)) };
  auto op1 = Entry::Operand{ Entry::Operand::MEM_U16, s16(instr) };
  auto op2 = Entry::Operand{ Entry::Operand::REG_S64, u8(RS(instr)) };
  Entry e(Entry::STORE16, dst, op1, op2);
  ir.push(e);
}

void JIT::sw(u32 instr) {
  auto dst = Entry::Operand{ Entry::Operand::REG_S64, u8(RT(instr)) };
  auto op1 = Entry::Operand{ Entry::Operand::MEM_U32, s16(instr) };
  auto op2 = Entry::Operand{ Entry::Operand::REG_S64, u8(RS(instr)) };
  Entry e(Entry::STORE32, dst, op1, op2);
  ir.push(e);
}

void JIT::sd(u32 instr) {
  auto dst = Entry::Operand{ Entry::Operand::REG_S64, u8(RT(instr)) };
  auto op1 = Entry::Operand{ Entry::Operand::MEM_U64, s16(instr) };
  auto op2 = Entry::Operand{ Entry::Operand::REG_S64, u8(RS(instr)) };
  Entry e(Entry::STORE64, dst, op1, op2);
  ir.push(e);
}

void JIT::sdl(u32 instr) {
  auto dst = Entry::Operand{ Entry::Operand::REG_S64, u8(RT(instr)) };
  auto op1 = Entry::Operand{ Entry::Operand::MEM_U64, s16(instr) };
  auto op2 = Entry::Operand{ Entry::Operand::REG_S64, u8(RS(instr)) };
  Entry e(Entry::STORE64, dst, op1, op2, Entry::LEFT);
  ir.push(e);
}

void JIT::sdr(u32 instr) {
  auto dst = Entry::Operand{ Entry::Operand::REG_S64, u8(RT(instr)) };
  auto op1 = Entry::Operand{ Entry::Operand::MEM_U64, s16(instr) };
  auto op2 = Entry::Operand{ Entry::Operand::REG_S64, u8(RS(instr)) };
  Entry e(Entry::STORE64, dst, op1, op2, Entry::RIGHT);
  ir.push(e);
}

void JIT::swl(u32 instr) {
  auto dst = Entry::Operand{ Entry::Operand::REG_S64, u8(RT(instr)) };
  auto op1 = Entry::Operand{ Entry::Operand::MEM_U32, s16(instr) };
  auto op2 = Entry::Operand{ Entry::Operand::REG_S64, u8(RS(instr)) };
  Entry e(Entry::STORE32, dst, op1, op2, Entry::LEFT);
  ir.push(e);
}

void JIT::swr(u32 instr) {
  auto dst = Entry::Operand{ Entry::Operand::REG_S64, u8(RT(instr)) };
  auto op1 = Entry::Operand{ Entry::Operand::MEM_U32, s16(instr) };
  auto op2 = Entry::Operand{ Entry::Operand::REG_S64, u8(RS(instr)) };
  Entry e(Entry::STORE32, dst, op1, op2, Entry::RIGHT);
  ir.push(e);
}

void JIT::ori(u32 instr) {
  auto dst = Entry::Operand{ Entry::Operand::REG_S64, u8(RT(instr)) };
  auto op1 = Entry::Operand{ Entry::Operand::IMM_U16, u16(instr) };
  auto op2 = Entry::Operand{ Entry::Operand::REG_S64, u8(RS(instr)) };
  Entry e(Entry::OR, dst, op1, op2);
  ir.push(e);
}

void JIT::or_(u32 instr) {
  auto dst = Entry::Operand{ Entry::Operand::REG_S64, u8(RD(instr)) };
  auto op1 = Entry::Operand{ Entry::Operand::REG_S64, u8(RT(instr)) };
  auto op2 = Entry::Operand{ Entry::Operand::REG_S64, u8(RS(instr)) };
  Entry e(Entry::OR, dst, op1, op2);
  ir.push(e);
}

void JIT::nor(u32 instr) {
  auto dst = Entry::Operand{ Entry::Operand::REG_S64, u8(RD(instr)) };
  auto op1 = Entry::Operand{ Entry::Operand::REG_S64, u8(RT(instr)) };
  auto op2 = Entry::Operand{ Entry::Operand::REG_S64, u8(RS(instr)) };
  Entry e(Entry::NOR, dst, op1, op2);
  ir.push(e);
}

void JIT::j(u32 instr) {
  auto dst = Entry::Operand{Entry::Operand::IMM_S64};
  auto op1 = Entry::Operand{Entry::Operand::PC64};
  auto op2 = Entry::Operand{Entry::Operand::IMM_S64, ~0xfffffff};
  Entry and_(Entry::AND, dst, op1, op2);
  ir.push(and_);
  op2 = Entry::Operand{Entry::Operand::IMM_S64, (instr & 0x3ffffff) << 2};
  Entry or_(Entry::OR, dst, dst, op2);
  ir.push(or_);
  Entry e(Entry::BRANCH, or_.GetDst());
}

void JIT::jal(u32 instr) {
  Entry link(Entry::MOV,
             Entry::Operand{Entry::Operand::REG_S64, 31},
             Entry::Operand{Entry::Operand::NEXTPC64});
  ir.push(link);
  j(instr);
}

void JIT::jalr(u32 instr) {
  auto addr = Entry::Operand{Entry::Operand::REG_U64, RS(instr)};
  Entry e(Entry::BRANCH, addr);
  Entry link(Entry::MOV,
             Entry::Operand{Entry::Operand::REG_S64, RD(instr)},
             Entry::Operand{Entry::Operand::PC64});
  ir.push(link);
}

void JIT::slti(u32 instr) {
  Entry e(Entry::SLT,
    { Entry::Operand::REG_U5, RT(instr) },
    { Entry::Operand::REG_S64, RS(instr) },
    { Entry::Operand::IMM_S64, s64(s16(instr)) });
  ir.push(e);
}

void JIT::sltiu(u32 instr) {
  Entry e(Entry::SLT,
    { Entry::Operand::REG_U5, RT(instr) },
    { Entry::Operand::REG_U64, RS(instr) },
    { Entry::Operand::IMM_U64, u64(s64(s16(instr))) });
  ir.push(e);
}

void JIT::slt(u32 instr) {
  Entry e(Entry::SLT,
    { Entry::Operand::REG_U5, RD(instr) },
    { Entry::Operand::REG_S64, RS(instr) },
    { Entry::Operand::REG_S64, RT(instr) });
  ir.push(e);
}

void JIT::sltu(u32 instr) {
  Entry e(Entry::SLT,
    { Entry::Operand::REG_U5, RD(instr) },
    { Entry::Operand::REG_U64, RS(instr) },
    { Entry::Operand::REG_U64, RT(instr) });
  ir.push(e);
}

void JIT::xori(u32 instr) {
  auto dst = Entry::Operand{ Entry::Operand::REG_S64, u8(RT(instr)) };
  auto op1 = Entry::Operand{ Entry::Operand::IMM_U16, u16(instr)};
  auto op2 = Entry::Operand{ Entry::Operand::REG_S64, u8(RS(instr)) };
  Entry e(Entry::XOR, dst, op1, op2);
  ir.push(e);
}

void JIT::xor_(u32 instr) {
  auto dst = Entry::Operand{ Entry::Operand::REG_S64, u8(RD(instr)) };
  auto op1 = Entry::Operand{ Entry::Operand::REG_S64, u8(RT(instr)) };
  auto op2 = Entry::Operand{ Entry::Operand::REG_S64, u8(RS(instr)) };
  Entry e(Entry::XOR, dst, op1, op2);
  ir.push(e);
}

void JIT::andi(u32 instr) {
  auto dst = Entry::Operand{ Entry::Operand::REG_S64, u8(RT(instr)) };
  auto op1 = Entry::Operand{ Entry::Operand::IMM_U16, u16(instr) };
  auto op2 = Entry::Operand{ Entry::Operand::REG_S64, u8(RS(instr)) };
  Entry e(Entry::AND, dst, op1, op2);
  ir.push(e);
}

void JIT::and_(u32 instr) {
  auto dst = Entry::Operand{ Entry::Operand::REG_S64, u8(RD(instr)) };
  auto op1 = Entry::Operand{ Entry::Operand::REG_S64, u8(RT(instr)) };
  auto op2 = Entry::Operand{ Entry::Operand::REG_S64, u8(RS(instr)) };
  Entry e(Entry::AND, dst, op1, op2);
  ir.push(e);
}

void JIT::sll(u32 instr) {
  auto dst = Entry::Operand{ Entry::Operand::REG_S64, u8(RD(instr)) };
  auto op1 = Entry::Operand{ Entry::Operand::REG_S64, u8(RT(instr)) };
  auto op2 = Entry::Operand{ Entry::Operand::IMM_U5, std::nullopt };
  Entry e(Entry::SLL, dst, op1, op2);
  ir.push(e);
}

void JIT::sllv(u32 instr) {
  if (RD(instr) != 0) [[likely]] {
    mov(rcx, GPR(qword, RS(instr)));
    CodeGenerator::and_(cl, 0x1F);
    mov(rax, GPR(qword, RT(instr)));
    sal(rax, cl);
    movsxd(rcx, eax);
    mov(GPR(qword, RD(instr)), rcx);
  }
}

void JIT::dsll32(u32 instr) {
  if (RD(instr) != 0) [[likely]] {
    u8 sa = ((instr >> 6) & 0x1f) + 32;
    mov(rax, GPR(qword, RT(instr)));
    sal(rax, sa);
    mov(GPR(qword, RT(instr)), rax);
  }
}

void JIT::dsll(u32 instr) {
  if (RD(instr) != 0) [[likely]] {
    u8 sa = ((instr >> 6) & 0x1f);
    mov(rax, GPR(qword, RT(instr)));
    sal(rax, sa);
    mov(GPR(qword, RT(instr)), rax);
  }
}

void JIT::dsllv(u32 instr) {
  if (RD(instr) != 0) [[likely]] {
    mov(rcx, GPR(qword, RS(instr)));
    CodeGenerator::and_(cl, 63);
    mov(rax, GPR(qword, RT(instr)));
    sal(rax, cl);
    mov(GPR(qword, RD(instr)), rax);
  }
}

void JIT::srl(u32 instr) {
  if (RD(instr) != 0) [[likely]] {
    u8 sa = ((instr >> 6) & 0x1f);
    mov(rax, GPR(qword, RT(instr)));
    CodeGenerator::shr(rax, sa);
    movsxd(rcx, eax);
    mov(GPR(qword, RD(instr)), rcx);
  }
}

void JIT::srlv(u32 instr) {
  if (RD(instr) != 0) [[likely]] {
    mov(rcx, GPR(qword, RS(instr)));
    CodeGenerator::and_(cl, 0x1F);
    mov(rax, GPR(qword, RT(instr)));
    shr(rax, cl);
    movsxd(rcx, eax);
    mov(GPR(qword, RD(instr)), rcx);
  }
}

void JIT::tgei(u32) {

}

void JIT::tgeiu(u32) {

}

void JIT::tlti(u32) {

}

void JIT::tltiu(u32) {

}

void JIT::teqi(u32) {

}

void JIT::tnei(u32) {

}

void JIT::tge(u32) {

}

void JIT::tgeu(u32) {

}

void JIT::tlt(u32) {

}

void JIT::tltu(u32) {

}

void JIT::teq(u32) {

}

void JIT::tne(u32) {

}

void JIT::dsrl(u32 instr) {
  if (RD(instr) != 0) [[likely]] {
    u8 sa = ((instr >> 6) & 0x1f);
    mov(rax, GPR(qword, RT(instr)));
    CodeGenerator::shr(rax, sa);
    mov(GPR(qword, RD(instr)), rax);
  }
}

void JIT::dsrlv(u32 instr) {
  if (RD(instr) != 0) [[likely]] {
    mov(rcx, GPR(qword, RS(instr)));
    CodeGenerator::and_(cl, 63);
    mov(rax, GPR(qword, RT(instr)));
    shr(rax, cl);
    mov(GPR(qword, RD(instr)), rax);
  }
}

void JIT::dsrl32(u32 instr) {
  if (RD(instr) != 0) [[likely]] {
    u8 sa = ((instr >> 6) & 0x1f) + 32;
    mov(rax, GPR(qword, RT(instr)));
    CodeGenerator::shr(rax, sa);
    mov(GPR(qword, RD(instr)), rax);
  }
}

void JIT::sra(u32 instr) {
  if (RD(instr) != 0) [[likely]] {
    u8 sa = ((instr >> 6) & 0x1f);
    mov(rax, GPR(qword, RT(instr)));
    sar(rax, sa);
    movsxd(rcx, eax);
    mov(GPR(qword, RD(instr)), rcx);
  }
}

void JIT::srav(u32 instr) {
  if (RD(instr) != 0) [[likely]] {
    mov(rcx, GPR(qword, RS(instr)));
    CodeGenerator::and_(cl, 0x1F);
    mov(rax, GPR(qword, RT(instr)));
    sar(rax, cl);
    movsxd(rcx, eax);
    mov(GPR(qword, RD(instr)), rcx);
  }
}

void JIT::dsra(u32 instr) {
  if (RD(instr) != 0) [[likely]] {
    u8 sa = ((instr >> 6) & 0x1f);
    mov(rax, GPR(qword, RT(instr)));
    sar(rax, sa);
    mov(GPR(qword, RD(instr)), rax);
  }
}

void JIT::dsrav(u32 instr) {
  if (RD(instr) != 0) [[likely]] {
    mov(rcx, GPR(qword, RS(instr)));
    CodeGenerator::and_(cl, 63);
    mov(rax, GPR(qword, RT(instr)));
    sar(rax, cl);
    mov(GPR(qword, RD(instr)), rax);
  }
}

void JIT::dsra32(u32 instr) {
  if (RD(instr) != 0) [[likely]] {
    u8 sa = ((instr >> 6) & 0x1f) + 32;
    mov(rax, GPR(qword, RT(instr)));
    sar(rax, sa);
    mov(GPR(qword, RD(instr)), rax);
  }
}

void JIT::jr(u32 instr) {
  auto addr = Entry::Operand{Entry::Operand::REG_U64, RS(instr)};
  Entry e(Entry::BRANCH, addr);
}

void JIT::dsub(u32 instr) {
  auto dst = Entry::Operand{ Entry::Operand::REG_S64, u8(RD(instr)) };
  auto op1 = Entry::Operand{ Entry::Operand::REG_S64, u8(RS(instr)) };
  auto op2 = Entry::Operand{ Entry::Operand::REG_S64, u8(RT(instr)) };
  Entry e(Entry::SUB, dst, op1, op2);
  ir.push(e);
}

void JIT::dsubu(u32 instr) {
  dsub(instr);
}

void JIT::sub(u32 instr) {
  auto dst = Entry::Operand{ Entry::Operand::REG_S64, u8(RD(instr)) };
  auto op1 = Entry::Operand{ Entry::Operand::REG_S32, u8(RS(instr)) };
  auto op2 = Entry::Operand{ Entry::Operand::REG_S32, u8(RT(instr)) };
  Entry e(Entry::SUB, dst, op1, op2);
  ir.push(e);
}

void JIT::subu(u32 instr) {
  sub(instr);
}

void JIT::dmultu(u32 instr) {
  auto op1 = Entry::Operand{ Entry::Operand::REG_U64, u8(RS(instr)) };
  auto op2 = Entry::Operand{ Entry::Operand::REG_U64, u8(RT(instr)) };
  Entry e(Entry::UMUL, op1, op2);
  ir.push(e);
}

void JIT::dmult(u32 instr) {
  auto op1 = Entry::Operand{ Entry::Operand::REG_S64, u8(RS(instr)) };
  auto op2 = Entry::Operand{ Entry::Operand::REG_S64, u8(RT(instr)) };
  Entry e(Entry::SMUL, op1, op2);
  ir.push(e);
}

void JIT::multu(u32 instr) {
  auto op1 = Entry::Operand{ Entry::Operand::REG_S32, u8(RS(instr)) };
  auto op2 = Entry::Operand{ Entry::Operand::REG_S32, u8(RT(instr)) };
  Entry e(Entry::UMUL, op1, op2);
  ir.push(e);
}

void JIT::mult(u32 instr) {
  auto op1 = Entry::Operand{ Entry::Operand::REG_S32, u8(RS(instr)) };
  auto op2 = Entry::Operand{ Entry::Operand::REG_S32, u8(RT(instr)) };
  Entry e(Entry::SMUL, op1, op2);
  ir.push(e);
}

void JIT::mflo(u32 instr) {
  auto dst = Entry::Operand{Entry::Operand::REG_S64, RD(instr)};
  auto src = Entry::Operand{Entry::Operand::LO};
  ir.push({Entry::MOV, dst, src});
}

void JIT::mfhi(u32 instr) {
  auto dst = Entry::Operand{Entry::Operand::REG_S64, RD(instr)};
  auto src = Entry::Operand{Entry::Operand::HI};
  ir.push({Entry::MOV, dst, src});
}

void JIT::mtlo(u32 instr) {
  auto dst = Entry::Operand{Entry::Operand::LO};
  auto src = Entry::Operand{Entry::Operand::REG_S64, RS(instr)};
  ir.push({Entry::MOV, dst, src});
}

void JIT::mthi(u32 instr) {
  auto dst = Entry::Operand{Entry::Operand::HI};
  auto src = Entry::Operand{Entry::Operand::REG_S64, RS(instr)};
  ir.push({Entry::MOV, dst, src});
}

void JIT::mtc0(u32 instr) {
  ir.push({Entry::MTC0,
           {Entry::Operand::IMM_U5, RD(instr)},
           {Entry::Operand::REG_S32, RT(instr)}});
}

void JIT::dmtc0(u32 instr) {
  ir.push({Entry::MTC0,
           {Entry::Operand::IMM_U5, RD(instr)},
           {Entry::Operand::REG_S64, RT(instr)}});
}

void JIT::mfc0(u32 instr) {
  ir.push({Entry::MFC0,
           {Entry::Operand::REG_S32, RT(instr)},
           {Entry::Operand::IMM_U5, RD(instr)}});
}

void JIT::dmfc0(u32 instr) {
  ir.push({Entry::MFC0,
           {Entry::Operand::REG_S64, RT(instr)},
           {Entry::Operand::IMM_U5, RD(instr)}});
}

void JIT::eret() {
  /*if(status.erl) {
    regs.SetPC64(ErrorEPC);
    status.erl = false;
  } else {
    regs.SetPC64(EPC);
    status.exl = false;
  }
  regs.cop0.Update();
  llbit = false;*/
}


void JIT::tlbr() {
  /*if (index.i >= 32) {
    Util::panic("TLBR with TLB index {}", index.i);
  }

  TLBEntry entry = tlb[index.i];

  entryHi.raw = entry.entryHi.raw;
  entryLo0.raw = entry.entryLo0.raw & 0x3FFFFFFF;
  entryLo1.raw = entry.entryLo1.raw & 0x3FFFFFFF;

  entryLo0.g = entry.global;
  entryLo1.g = entry.global;
  pageMask.raw = entry.pageMask.raw;*/
}

void JIT::tlbw(int index_) {
  /*PageMask page_mask{};
  page_mask = pageMask;
  u32 top = page_mask.mask & 0xAAA;
  page_mask.mask = top | (top >> 1);

  if(index_ >= 32) {
    Util::panic("TLBWI with TLB index {}", index_);
  }

  tlb[index_].entryHi.raw  = entryHi.raw;
  tlb[index_].entryHi.vpn2 &= ~page_mask.mask;

  tlb[index_].entryLo0.raw = entryLo0.raw & 0x03FFFFFE;
  tlb[index_].entryLo1.raw = entryLo1.raw & 0x03FFFFFE;
  tlb[index_].pageMask.raw = page_mask.raw;

  tlb[index_].global = entryLo0.g && entryLo1.g;
  tlb[index_].initialized = true;*/
}

void JIT::tlbp() {
  /*int match = -1;
  TLBEntry* entry = TLBTryMatch(regs, entryHi.raw, &match);
  if(entry && match >= 0) {
    index.raw = match;
  } else {
    index.raw = 0;
    index.p = 1;
  }*/
}

void JIT::mtc2(u32 instr) {

}

void JIT::mfc2(u32 instr) {

}

void JIT::dmtc2(u32 instr) {

}

void JIT::dmfc2(u32 instr) {

}

void JIT::ctc2(u32) {

}

void JIT::cfc2(u32) {

}

}