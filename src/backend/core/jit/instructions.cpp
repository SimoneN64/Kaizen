#include <JIT.hpp>
#include <jit/helpers.hpp>

#define check_signed_overflow(op1, op2, res) (((~((op1) ^ (op2)) & ((op1) ^ (res))) >> ((sizeof(res) * 8) - 1)) & 1)
#define check_signed_underflow(op1, op2, res) (((((op1) ^ (op2)) & ((op1) ^ (res))) >> ((sizeof(res) * 8) - 1)) & 1)

namespace n64 {
using namespace Xbyak::util;

void JIT::lui(const u32 instr) {
  u64 val = static_cast<s64>(static_cast<s16>(instr));
  val <<= 16;
  regs.Write<s64>(RT(instr), val);
}

void JIT::add(const u32 instr) {
  if (regs.IsRegConstant(RS(instr), RT(instr))) {
    const u32 rs = regs.Read<s32>(RS(instr));
    const u32 rt = regs.Read<s32>(RT(instr));
    const u32 result = rs + rt;
    if (check_signed_overflow(rs, rt, result)) {
      // regs.cop0.FireException(ExceptionCode::Overflow, 0, regs.oldPC);
      Util::panic("[JIT]: Unhandled Overflow exception in ADD!");
    }

    regs.Write<s32>(RD(instr), result);
    return;
  }

  if (regs.IsRegConstant(RS(instr))) {
    const u32 rs = regs.Read<s32>(RS(instr));
    regs.Read<u32>(RT(instr), code.eax);
    code.add(code.eax, rs);
    regs.Write<s32>(RD(instr), code.eax);

    return;
  }

  if (regs.IsRegConstant(RT(instr))) {
    const u32 rt = regs.Read<s32>(RT(instr));
    regs.Read<u32>(RS(instr), code.eax);
    code.add(code.eax, rt);
    regs.Write<s32>(RD(instr), code.eax);

    return;
  }

  regs.Read<u32>(RT(instr), code.eax);
  regs.Read<u32>(RS(instr), code.edi);
  code.add(code.eax, code.edi);
  regs.Write<s32>(RD(instr), code.eax);
}

void JIT::addu(u32 instr) {
  if (regs.IsRegConstant(RS(instr), RT(instr))) {
    const s32 rs = regs.Read<s32>(RS(instr));
    const s32 rt = regs.Read<s32>(RT(instr));
    const s32 result = rs + rt;

    regs.Write<s32>(RD(instr), result);
    return;
  }

  if (regs.IsRegConstant(RS(instr))) {
    const s32 rs = regs.Read<s32>(RS(instr));
    regs.Read<s32>(RT(instr), code.eax);
    code.add(code.eax, rs);
    regs.Write<s32>(RD(instr), code.eax);
    return;
  }

  if (regs.IsRegConstant(RT(instr))) {
    const s32 rs = regs.Read<s32>(RT(instr));
    regs.Read<s32>(RS(instr), code.eax);
    code.add(code.eax, rs);
    regs.Write<s32>(RD(instr), code.eax);
    return;
  }

  regs.Read<s32>(RS(instr), code.eax);
  regs.Read<s32>(RT(instr), code.edi);
  code.add(code.eax, code.edi);
  regs.Write<s32>(RD(instr), code.eax);
}

void JIT::addi(u32 instr) {
  u32 imm = s32(s16(instr));
  if (regs.IsRegConstant(RS(instr))) {
    auto rs = regs.Read<u32>(RS(instr));
    u32 result = rs + imm;
    if (check_signed_overflow(rs, imm, result)) {
      Util::panic("[JIT]: Unhandled Overflow exception in ADDI!");
    }

    regs.Write<s32>(RT(instr), static_cast<s32>(result));
    return;
  }

  regs.Read<u32>(RS(instr), code.eax);
  code.add(code.eax, imm);
  regs.Write<s32>(RT(instr), code.eax);
}

void JIT::addiu(u32 instr) {
  u32 imm = s32(s16(instr));
  if (regs.IsRegConstant(RS(instr))) {
    auto rs = regs.Read<u32>(RS(instr));
    u32 result = rs + imm;
    regs.Write(RT(instr), s32(result));

    return;
  }

  regs.Read<u32>(RS(instr), code.eax);
  code.add(code.eax, imm);
  regs.Write<s32>(RT(instr), code.eax);
}

void JIT::andi(u32 instr) {
  const s64 imm = static_cast<u16>(instr);
  if (regs.IsRegConstant(RS(instr))) {
    regs.Write(RT(instr), regs.Read<s64>(RS(instr)) & imm);
    return;
  }

  regs.Read<s64>(RS(instr), code.rax);
  code.and_(code.rax, imm);
  regs.Write<s64>(RT(instr), code.rax);
}

void JIT::and_(u32 instr) {
  if (regs.IsRegConstant(RS(instr), RT(instr))) {
    regs.Write(RD(instr), regs.Read<s64>(RS(instr)) & regs.Read<s64>(RT(instr)));
    return;
  }

  if (regs.IsRegConstant(RS(instr))) {
    const auto rs = regs.Read<s64>(RS(instr));
    regs.Read<s64>(RT(instr), code.rax);
    code.and_(code.rax, rs);
    regs.Write<s64>(RD(instr), code.rax);
    return;
  }

  if (regs.IsRegConstant(RT(instr))) {
    const auto rt = regs.Read<s64>(RT(instr));
    regs.Read<s64>(RS(instr), code.rax);
    code.and_(code.rax, rt);
    regs.Write<s64>(RD(instr), code.rax);
    return;
  }

  regs.Read<s64>(RS(instr), code.rax);
  regs.Read<s64>(RT(instr), code.rdi);
  code.and_(code.rdi, code.rax);
  regs.Write<s64>(RD(instr), code.rdi);
}

void JIT::SkipSlot() { code.jmp("not_taken"); }

void JIT::SkipSlotConstant() { blockPC += 4; }

void JIT::BranchTaken(const s64 offs) {
  code.mov(code.rax, blockPC + offs);
  code.mov(REG(qword, pc), code.rax);
}

void JIT::BranchTaken(const Xbyak::Reg64 &offs) {
  code.add(offs, blockPC);
  code.mov(REG(qword, pc), offs);
}

void JIT::BranchAbsTaken(const s64 addr) {
  code.mov(code.rax, addr);
  code.mov(REG(qword, pc), code.rax);
}

void JIT::BranchAbsTaken(const Xbyak::Reg64 &addr) { code.mov(REG(qword, pc), addr); }

#define branch(offs, cond)                                                                                             \
  do {                                                                                                                 \
    Xbyak::Label taken, not_taken;                                                                                     \
    code.j##cond(taken);                                                                                               \
    code.mov(code.rax, blockPC);                                                                                       \
    code.mov(REG(qword, pc), code.rax);                                                                                \
    code.jmp(not_taken);                                                                                               \
    code.L(taken);                                                                                                     \
    BranchTaken(offs);                                                                                                 \
    code.L(not_taken);                                                                                                 \
  }                                                                                                                    \
  while (0)

#define branch_abs(addr, cond)                                                                                         \
  do {                                                                                                                 \
    Xbyak::Label taken, not_taken;                                                                                     \
    code.j##cond(taken);                                                                                               \
    code.mov(code.rax, blockPC);                                                                                       \
    code.mov(REG(qword, pc), code.rax);                                                                                \
    code.jmp(not_taken);                                                                                               \
    code.L(taken);                                                                                                     \
    BranchAbsTaken(addr);                                                                                              \
    code.L(not_taken);                                                                                                 \
  }                                                                                                                    \
  while (0)

#define branch_likely(offs, cond)                                                                                      \
  do {                                                                                                                 \
    Xbyak::Label taken;                                                                                                \
    code.j##cond(taken);                                                                                               \
    SkipSlot();                                                                                                        \
    code.jmp(branch_likely_not_taken);                                                                                 \
    code.L(taken);                                                                                                     \
    BranchTaken(offs);                                                                                                 \
  }                                                                                                                    \
  while (0)

void JIT::branch_constant(const bool cond, const s64 offset) {
  if (cond) {
    BranchTaken(offset);
  }
}

void JIT::branch_likely_constant(const bool cond, const s64 offset) {
  if (cond) {
    BranchTaken(offset);
  } else {
    SkipSlotConstant();
  }
}

void JIT::branch_abs_constant(const bool cond, const s64 address) {
  if (cond) {
    BranchAbsTaken(address);
  }
}

void JIT::bfc0(u32 instr) {
  const s16 imm = instr;
  const s64 offset = u64((s64)imm) << 2;
  const s64 address = regs.pc + offset;
  // branch(regs, EvaluateCondition(regs, cond, regs.cop1.fcr31.compare, 1), address);
}

void JIT::blfc0(u32 instr) {
  const s16 imm = instr;
  const s64 offset = u64((s64)imm) << 2;
  const s64 address = regs.pc + offset;
  // branch_likely(regs, EvaluateCondition(regs, cond, regs.cop1.fcr31.compare, 1), address);
}

void JIT::bfc1(u32 instr) {
  const s16 imm = instr;
  const s64 offset = u64((s64)imm) << 2;
  const s64 address = regs.pc + offset;
  // branch(regs, EvaluateCondition(regs, cond, regs.cop1.fcr31.compare, 1), address);
}

void JIT::blfc1(u32 instr) {
  const s16 imm = instr;
  const s64 offset = u64((s64)imm) << 2;
  const s64 address = regs.pc + offset;
  // branch_likely(regs, EvaluateCondition(regs, cond, regs.cop1.fcr31.compare, 1), address);
}

void JIT::bltz(const u32 instr) {
  const s16 imm = instr;
  const s64 offset = u64((s64)imm) << 2;
  if (regs.IsRegConstant(RS(instr))) {
    branch_constant(regs.Read<s64>(RS(instr)) < 0, offset);
    return;
  }

  regs.Read<s64>(RS(instr), code.rax);
  code.cmp(code.rax, 0);
  branch(offset, l);
}

void JIT::bgez(const u32 instr) {
  const s16 imm = instr;
  const s64 offset = u64((s64)imm) << 2;
  if (regs.IsRegConstant(RS(instr))) {
    branch_constant(regs.Read<s64>(RS(instr)) >= 0, offset);
    return;
  }

  regs.Read<s64>(RS(instr), code.rax);
  code.cmp(code.rax, 0);
  branch(offset, ge);
}

void JIT::bltzl(const u32 instr) {
  const s16 imm = instr;
  const s64 offset = u64((s64)imm) << 2;
  if (regs.IsRegConstant(RS(instr))) {
    branch_likely_constant(regs.Read<s64>(RS(instr)) < 0, offset);
    return;
  }

  regs.Read<s64>(RS(instr), code.rax);
  code.cmp(code.rax, 0);
  branch_likely(offset, l);
}

void JIT::bgezl(const u32 instr) {
  const s16 imm = instr;
  const s64 offset = u64((s64)imm) << 2;
  if (regs.IsRegConstant(RS(instr))) {
    branch_likely_constant(regs.Read<s64>(RS(instr)) >= 0, offset);
    return;
  }

  regs.Read<s64>(RS(instr), code.rax);
  code.cmp(code.rax, 0);
  branch_likely(offset, ge);
}

void JIT::bltzal(const u32 instr) {
  const s16 imm = instr;
  const s64 offset = u64((s64)imm) << 2;
  regs.Write<s64>(31, blockPC + 4);
  if (regs.IsRegConstant(RS(instr))) {
    branch_constant(regs.Read<s64>(RS(instr)) < 0, offset);
    return;
  }

  regs.Read<s64>(RS(instr), code.rax);
  code.cmp(code.rax, 0);
  branch(offset, l);
}

void JIT::bgezal(const u32 instr) {
  const s16 imm = instr;
  const s64 offset = u64((s64)imm) << 2;
  regs.Write<s64>(31, blockPC + 4);
  if (regs.IsRegConstant(RS(instr))) {
    branch_constant(regs.Read<s64>(RS(instr)) >= 0, offset);
    return;
  }

  regs.Read<s64>(RS(instr), code.rax);
  code.cmp(code.rax, 0);
  branch(offset, ge);
}

void JIT::bltzall(const u32 instr) {
  const s16 imm = instr;
  const s64 offset = u64((s64)imm) << 2;
  regs.Write<s64>(31, blockPC + 4);
  if (regs.IsRegConstant(RS(instr))) {
    branch_likely_constant(regs.Read<s64>(RS(instr)) < 0, offset);
    return;
  }

  regs.Read<s64>(RS(instr), code.rax);
  code.cmp(code.rax, 0);
  branch_likely(offset, l);
}

void JIT::bgezall(const u32 instr) {
  const s16 imm = instr;
  const s64 offset = u64((s64)imm) << 2;
  regs.Write<s64>(31, blockPC + 4);
  if (regs.IsRegConstant(RS(instr))) {
    branch_likely_constant(regs.Read<s64>(RS(instr)) >= 0, offset);
    return;
  }

  regs.Read<s64>(RS(instr), code.rax);
  code.cmp(code.rax, 0);
  branch_likely(offset, ge);
}

void JIT::beq(const u32 instr) {
  const s16 imm = instr;
  const s64 offset = u64((s64)imm) << 2;
  if (regs.IsRegConstant(RS(instr)) && regs.IsRegConstant(RT(instr))) {
    branch_constant(regs.Read<s64>(RS(instr)) == regs.Read<s64>(RT(instr)), offset);
    return;
  }

  if (regs.IsRegConstant(RS(instr))) {
    regs.Read<s64>(RT(instr), code.rax);
    code.cmp(code.rax, regs.Read<s64>(RS(instr)));
    branch(offset, e);
    return;
  }

  if (regs.IsRegConstant(RT(instr))) {
    regs.Read<s64>(RS(instr), code.rax);
    code.cmp(code.rax, regs.Read<s64>(RT(instr)));
    branch(offset, e);
    return;
  }

  regs.Read<s64>(RS(instr), code.rax);
  regs.Read<s64>(RT(instr), code.rdi);
  code.cmp(code.rax, code.rdi);
  branch(offset, e);
}

void JIT::beql(const u32 instr) {
  const s16 imm = instr;
  const s64 offset = u64((s64)imm) << 2;
  if (regs.IsRegConstant(RS(instr)) && regs.IsRegConstant(RT(instr))) {
    branch_likely_constant(regs.Read<s64>(RS(instr)) == regs.Read<s64>(RT(instr)), offset);
    return;
  }

  if (regs.IsRegConstant(RS(instr))) {
    regs.Read<s64>(RT(instr), code.rax);
    code.cmp(code.rax, regs.Read<s64>(RS(instr)));
    branch_likely(offset, e);
    return;
  }

  if (regs.IsRegConstant(RT(instr))) {
    regs.Read<s64>(RS(instr), code.rax);
    code.cmp(code.rax, regs.Read<s64>(RT(instr)));
    branch_likely(offset, e);
    return;
  }

  regs.Read<s64>(RS(instr), code.rax);
  regs.Read<s64>(RT(instr), code.rdi);
  code.cmp(code.rax, code.rdi);
  branch_likely(offset, e);
}

void JIT::bne(const u32 instr) {
  const s16 imm = instr;
  const s64 offset = u64((s64)imm) << 2;
  if (regs.IsRegConstant(RS(instr)) && regs.IsRegConstant(RT(instr))) {
    branch_constant(regs.Read<s64>(RS(instr)) != regs.Read<s64>(RT(instr)), offset);
    return;
  }

  if (regs.IsRegConstant(RS(instr))) {
    regs.Read<s64>(RT(instr), code.rax);
    code.cmp(code.rax, regs.Read<s64>(RS(instr)));
    branch(offset, ne);
    return;
  }

  if (regs.IsRegConstant(RT(instr))) {
    regs.Read<s64>(RS(instr), code.rax);
    code.cmp(code.rax, regs.Read<s64>(RT(instr)));
    branch(offset, ne);
    return;
  }

  regs.Read<s64>(RS(instr), code.rax);
  regs.Read<s64>(RT(instr), code.rdi);
  code.cmp(code.rax, code.rdi);
  branch(offset, ne);
}

void JIT::bnel(const u32 instr) {
  const s16 imm = instr;
  const s64 offset = u64((s64)imm) << 2;
  if (regs.IsRegConstant(RS(instr)) && regs.IsRegConstant(RT(instr))) {
    branch_likely_constant(regs.Read<s64>(RS(instr)) != regs.Read<s64>(RT(instr)), offset);
    return;
  }

  if (regs.IsRegConstant(RS(instr))) {
    regs.Read<s64>(RT(instr), code.rax);
    code.cmp(code.rax, regs.Read<s64>(RS(instr)));
    branch_likely(offset, ne);
    return;
  }

  if (regs.IsRegConstant(RT(instr))) {
    regs.Read<s64>(RS(instr), code.rax);
    code.cmp(code.rax, regs.Read<s64>(RT(instr)));
    branch_likely(offset, ne);
    return;
  }

  regs.Read<s64>(RS(instr), code.rax);
  regs.Read<s64>(RT(instr), code.rdi);
  code.cmp(code.rax, code.rdi);
  branch_likely(offset, ne);
}

void JIT::blez(const u32 instr) {
  const s16 imm = instr;
  const s64 offset = u64((s64)imm) << 2;
  if (regs.IsRegConstant(RS(instr))) {
    branch_constant(regs.Read<s64>(RS(instr)) <= 0, offset);
    return;
  }

  regs.Read<s64>(RS(instr), code.rax);
  code.cmp(code.rax, 0);
  branch(offset, le);
}

void JIT::blezl(const u32 instr) {
  const s16 imm = instr;
  const s64 offset = u64((s64)imm) << 2;
  if (regs.IsRegConstant(RS(instr))) {
    branch_likely_constant(regs.Read<s64>(RS(instr)) <= 0, offset);
    return;
  }

  regs.Read<s64>(RS(instr), code.rax);
  code.cmp(code.rax, 0);
  branch_likely(offset, le);
}

void JIT::bgtz(const u32 instr) {
  const s16 imm = instr;
  const s64 offset = u64((s64)imm) << 2;
  if (regs.IsRegConstant(RS(instr))) {
    branch_constant(regs.Read<s64>(RS(instr)) > 0, offset);
    return;
  }

  regs.Read<s64>(RS(instr), code.rax);
  code.cmp(code.rax, 0);
  branch(offset, g);
}

void JIT::bgtzl(const u32 instr) {
  const s16 imm = instr;
  const s64 offset = u64((s64)imm) << 2;
  if (regs.IsRegConstant(RS(instr))) {
    branch_likely_constant(regs.Read<s64>(RS(instr)) > 0, offset);
    return;
  }

  regs.Read<s64>(RS(instr), code.rax);
  code.cmp(code.rax, 0);
  branch_likely(offset, g);
}

void JIT::dadd(u32 instr) {
  if (regs.IsRegConstant(RS(instr), RT(instr))) {
    auto rs = regs.Read<u64>(RS(instr));
    auto rt = regs.Read<u64>(RT(instr));
    u64 result = rt + rs;
    if (check_signed_overflow(rs, rt, result)) {
      // regs.cop0.FireException(ExceptionCode::Overflow, 0, regs.oldPC);
      Util::panic("[JIT]: Unhandled Overflow exception in DADD!");
    }
    regs.Write(RD(instr), result);
  } else {
    Util::panic("[JIT]: Implement non constant DADD!");
  }
}

void JIT::daddu(u32 instr) {
  if (regs.IsRegConstant(RS(instr), RT(instr))) {
    auto rs = regs.Read<s64>(RS(instr));
    auto rt = regs.Read<s64>(RT(instr));
    regs.Write(RD(instr), rt + rs);
  } else {
    Util::panic("[JIT]: Implement non constant DADD!");
  }
}

void JIT::daddi(u32 instr) {
  if (regs.IsRegConstant(RS(instr))) {
    u64 imm = s64(s16(instr));
    auto rs = regs.Read<u64>(RS(instr));
    u64 result = imm + rs;
    if (check_signed_overflow(rs, imm, result)) {
      // regs.cop0.FireException(ExceptionCode::Overflow, 0, regs.oldPC);
      Util::panic("[JIT]: Unhandled Overflow exception in DADDI!");
    }
    regs.Write(RT(instr), result);
  } else {
    Util::panic("[JIT]: Implement non constant DADDI!");
  }
}

void JIT::daddiu(u32 instr) {
  if (regs.IsRegConstant(RS(instr))) {
    s16 imm = s16(instr);
    auto rs = regs.Read<s64>(RS(instr));
    regs.Write(RT(instr), imm + rs);
  } else {
    Util::panic("[JIT]: Implement non constant DADDI!");
  }
}

void JIT::ddiv(u32 instr) {
  if (regs.IsRegConstant(RS(instr), RT(instr))) {
    auto dividend = regs.Read<s64>(RS(instr));
    auto divisor = regs.Read<s64>(RT(instr));
    if (dividend == 0x8000000000000000 && divisor == 0xFFFFFFFFFFFFFFFF) {
      regs.lo = dividend;
      regs.hi = 0;
    } else if (divisor == 0) {
      regs.hi = dividend;
      if (dividend >= 0) {
        regs.lo = -1;
      } else {
        regs.lo = 1;
      }
    } else {
      s64 quotient = dividend / divisor;
      s64 remainder = dividend % divisor;
      regs.lo = quotient;
      regs.hi = remainder;
    }

    regs.loIsConstant = true;
    regs.hiIsConstant = true;
  } else {
    Util::panic("[JIT]: Implement non constant DDIV!");
  }
}

void JIT::ddivu(u32 instr) {
  if (regs.IsRegConstant(RS(instr), RT(instr))) {
    auto dividend = regs.Read<u64>(RS(instr));
    auto divisor = regs.Read<u64>(RT(instr));
    if (divisor == 0) {
      regs.lo = -1;
      regs.hi = (s64)dividend;
    } else {
      u64 quotient = dividend / divisor;
      u64 remainder = dividend % divisor;
      regs.lo = (s64)quotient;
      regs.hi = (s64)remainder;
    }

    regs.loIsConstant = true;
    regs.hiIsConstant = true;
  } else {
    Util::panic("[JIT]: Implement non constant DDIVU!");
  }
}

void JIT::div(u32 instr) {
  if (regs.IsRegConstant(RS(instr), RT(instr))) {
    s64 dividend = regs.Read<s32>(RS(instr));
    s64 divisor = regs.Read<s32>(RT(instr));

    if (divisor == 0) {
      regs.hi = dividend;
      if (dividend >= 0) {
        regs.lo = s64(-1);
      } else {
        regs.lo = s64(1);
      }
    } else {
      s32 quotient = dividend / divisor;
      s32 remainder = dividend % divisor;
      regs.lo = quotient;
      regs.hi = remainder;
    }

    regs.loIsConstant = true;
    regs.hiIsConstant = true;
  } else {
    Util::panic("[JIT]: Implement non constant DIV!");
  }
}

void JIT::divu(u32 instr) {
  if (regs.IsRegConstant(RS(instr), RT(instr))) {
    auto dividend = regs.Read<u32>(RS(instr));
    auto divisor = regs.Read<u32>(RT(instr));
    if (divisor == 0) {
      regs.lo = -1;
      regs.hi = (s32)dividend;
    } else {
      s32 quotient = (s32)(dividend / divisor);
      s32 remainder = (s32)(dividend % divisor);
      regs.lo = quotient;
      regs.hi = remainder;
    }

    regs.loIsConstant = true;
    regs.hiIsConstant = true;
  } else {
    Util::panic("[JIT]: Implement non constant DIVU!");
  }
}

void JIT::dmult(u32 instr) {
  if (regs.IsRegConstant(RT(instr), RS(instr))) {
    auto rt = regs.Read<s64>(RT(instr));
    auto rs = regs.Read<s64>(RS(instr));
    s128 result = (s128)rt * (s128)rs;
    regs.lo = result & 0xFFFFFFFFFFFFFFFF;
    regs.hi = result >> 64;
    regs.hiIsConstant = true;
    regs.loIsConstant = true;
  } else {
    Util::panic("[JIT]: Implement non constant DMULT!");
  }
}

void JIT::dmultu(u32 instr) {
  if (regs.IsRegConstant(RT(instr), RS(instr))) {
    auto rt = regs.Read<u64>(RT(instr));
    auto rs = regs.Read<u64>(RS(instr));
    u128 result = (u128)rt * (u128)rs;
    regs.lo = s64(result & 0xFFFFFFFFFFFFFFFF);
    regs.hi = s64(result >> 64);
    regs.hiIsConstant = true;
    regs.loIsConstant = true;
  } else {
    Util::panic("[JIT]: Implement non constant DMULT!");
  }
}

void JIT::dsll(u32 instr) {
  if (regs.IsRegConstant(RT(instr))) {
    u8 sa = ((instr >> 6) & 0x1f);
    auto result = regs.Read<s64>(RT(instr)) << sa;
    regs.Write(RD(instr), result);
  } else {
    Util::panic("[JIT]: Implement non constant DSLL!");
  }
}

void JIT::dsllv(u32 instr) {
  if (regs.IsRegConstant(RT(instr), RS(instr))) {
    auto sa = regs.Read<s64>(RS(instr)) & 63;
    auto result = regs.Read<s64>(RT(instr)) << sa;
    regs.Write(RD(instr), result);
  } else {
    Util::panic("[JIT]: Implement non constant DSLLV!");
  }
}

void JIT::dsll32(u32 instr) {
  if (regs.IsRegConstant(RT(instr))) {
    u8 sa = ((instr >> 6) & 0x1f);
    auto result = regs.Read<s64>(RT(instr)) << (sa + 32);
    regs.Write(RD(instr), result);
  } else {
    Util::panic("[JIT]: Implement non constant DSLL32!");
  }
}

void JIT::dsra(u32 instr) {
  if (regs.IsRegConstant(RT(instr))) {
    auto rt = regs.Read<s64>(RT(instr));
    u8 sa = ((instr >> 6) & 0x1f);
    s64 result = rt >> sa;
    regs.Write(RD(instr), result);
  } else {
    Util::panic("[JIT]: Implement non constant DSRA!");
  }
}

void JIT::dsrav(u32 instr) {
  if (regs.IsRegConstant(RT(instr), RS(instr))) {
    auto rt = regs.Read<s64>(RT(instr));
    auto rs = regs.Read<s64>(RS(instr));
    s64 sa = rs & 63;
    s64 result = rt >> sa;
    regs.Write(RD(instr), result);
  } else {
    Util::panic("[JIT]: Implement non constant DSRAV!");
  }
}

void JIT::dsra32(u32 instr) {
  if (regs.IsRegConstant(RT(instr))) {
    auto rt = regs.Read<s64>(RT(instr));
    u8 sa = ((instr >> 6) & 0x1f);
    s64 result = rt >> (sa + 32);
    regs.Write(RD(instr), result);
  } else {
    Util::panic("[JIT]: Implement non constant DSRA32!");
  }
}

void JIT::dsrl(u32 instr) {
  if (regs.IsRegConstant(RT(instr))) {
    auto rt = regs.Read<u64>(RT(instr));
    u8 sa = ((instr >> 6) & 0x1f);
    u64 result = rt >> sa;
    regs.Write(RD(instr), s64(result));
  } else {
    Util::panic("[JIT]: Implement non constant DSRL!");
  }
}

void JIT::dsrlv(u32 instr) {
  if (regs.IsRegConstant(RT(instr), RS(instr))) {
    u8 amount = regs.Read<u8>(RS(instr)) & 63;
    auto rt = regs.Read<u64>(RT(instr));
    u64 result = rt >> amount;
    regs.Write(RD(instr), s64(result));
  } else {
    Util::panic("[JIT]: Implement non constant DSRLV!");
  }
}

void JIT::dsrl32(u32 instr) {
  if (regs.IsRegConstant(RT(instr))) {
    auto rt = regs.Read<u64>(RT(instr));
    u8 sa = ((instr >> 6) & 0x1f);
    u64 result = rt >> (sa + 32);
    regs.Write(RD(instr), s64(result));
  } else {
    Util::panic("[JIT]: Implement non constant DSRL32!");
  }
}

void JIT::dsub(u32 instr) {
  if (regs.IsRegConstant(RT(instr), RS(instr))) {
    auto rt = regs.Read<s64>(RT(instr));
    auto rs = regs.Read<s64>(RS(instr));
    s64 result = rs - rt;
    if (check_signed_underflow(rs, rt, result)) {
      // regs.cop0.FireException(ExceptionCode::Overflow, 0, regs.oldPC);
      Util::panic("[JIT]: Unhandled Overflow exception in DSUB!");
    } else {
      regs.Write(RD(instr), result);
    }
  } else {
    Util::panic("[JIT]: Implement non constant DSUB!");
  }
}

void JIT::dsubu(u32 instr) {
  if (regs.IsRegConstant(RT(instr), RS(instr))) {
    auto rt = regs.Read<s64>(RT(instr));
    auto rs = regs.Read<s64>(RS(instr));
    s64 result = rs - rt;
    regs.Write(RD(instr), result);
  } else {
    Util::panic("[JIT]: Implement non constant DSUBU!");
  }
}

void JIT::j(const u32 instr) {
  const s32 target = (instr & 0x3ffffff) << 2;
  const s64 oldPC = branchPC - 8;
  const s64 address = (oldPC & ~0xfffffff) | target;
  branch_abs_constant(true, address);
}

void JIT::jr(const u32 instr) {
  if (regs.IsRegConstant(RS(instr))) {
    const u64 address = regs.Read<s64>(RS(instr));
    branch_abs_constant(true, address);
    return;
  }

  regs.Read<s64>(RS(instr), code.rax);
  branch_abs(code.rax, mp);
}

void JIT::jal(const u32 instr) {
  regs.Write<s64>(31, blockPC + 4);
  j(instr);
}

void JIT::jalr(const u32 instr) {
  regs.Write<s64>(RD(instr), blockPC + 4);
  jr(instr);
}

void JIT::lbu(u32 instr) {
  if (regs.IsRegConstant(RS(instr))) {
    const u64 address = regs.Read<s64>(RS(instr)) + (s16)instr;
    u32 paddr;
    if (!regs.cop0.MapVAddr(Cop0::LOAD, address, paddr)) {
      // regs.cop0.HandleTLBException(address);
      // regs.cop0.FireException(Cop0::GetTLBExceptionCode(regs.cop0.tlbError, Cop0::LOAD), 0, regs.oldPC);
      Util::panic("[JIT]: Unhandled TLBL exception in LBU!");
    } else {
      const u8 value = mem.Read<u8>(regs, paddr);
      regs.Write(RT(instr), value);
    }
  } else {
    Util::panic("[JIT]: Implement non constant LBU!");
  }
}

void JIT::lb(u32 instr) {
  if (regs.IsRegConstant(RS(instr))) {
    const u64 address = regs.Read<s64>(RS(instr)) + (s16)instr;
    if (u32 paddr = 0; !regs.cop0.MapVAddr(Cop0::LOAD, address, paddr)) {
      // regs.cop0.HandleTLBException(address);
      // regs.cop0.FireException(Cop0::GetTLBExceptionCode(regs.cop0.tlbError, Cop0::LOAD), 0, regs.oldPC);
      Util::panic("[JIT]: Unhandled TLBL exception in LB!");
    } else {
      regs.Write(RT(instr), (s8)mem.Read<u8>(regs, paddr));
    }
  } else {
    Util::panic("[JIT]: Implement non constant LB!");
  }
}

void JIT::ld(u32 instr) {
  if (regs.IsRegConstant(RS(instr))) {
    const s64 address = regs.Read<s64>(RS(instr)) + (s16)instr;
    if (check_address_error(0b111, address)) {
      // regs.cop0.HandleTLBException(address);
      // regs.cop0.FireException(ExceptionCode::AddressErrorLoad, 0, regs.oldPC);
      // return;
      Util::panic("[JIT]: Unhandled ADEL exception in LD!");
    }

    u32 paddr = 0;
    if (!regs.cop0.MapVAddr(Cop0::LOAD, address, paddr)) {
      // regs.cop0.HandleTLBException(address);
      // regs.cop0.FireException(Cop0::GetTLBExceptionCode(regs.cop0.tlbError, Cop0::LOAD), 0, regs.oldPC);
      Util::panic("[JIT]: Unhandled TLBL exception in LD!");
    } else {
      const s64 value = mem.Read<u64>(regs, paddr);
      regs.Write(RT(instr), value);
    }
  } else {
    Util::panic("[JIT]: Implement non constant LD!");
  }
}

void JIT::ldc1(u32 instr) {
  if (regs.IsRegConstant(BASE(instr))) {
    const u64 addr = static_cast<s64>(static_cast<s16>(instr)) + regs.Read<s64>(BASE(instr));

    if (u32 physical; !regs.cop0.MapVAddr(Cop0::LOAD, addr, physical)) {
      // regs.cop0.HandleTLBException(addr);
      // regs.cop0.FireException(Cop0::GetTLBExceptionCode(regs.cop0.tlbError, Cop0::LOAD), 0, regs.oldPC);
      Util::panic("[JIT]: Unhandled TLBL exception in LD1!");
    } else {
      const u64 data = mem.Read<u64>(regs, physical);
      regs.cop1.FGR_T<u64>(regs.cop0.status, FT(instr)) = data;
      regs.cop1.fgrIsConstant[FT(instr)] = true;
    }
  } else {
    Util::panic("[JIT]: Implement non constant LD1!");
  }
}

void JIT::ldl(u32 instr) {
  if (regs.IsRegConstant(RS(instr), RT(instr))) {
    const u64 address = regs.Read<s64>(RS(instr)) + (s16)instr;
    u32 paddr = 0;
    if (!regs.cop0.MapVAddr(Cop0::LOAD, address, paddr)) {
      // regs.cop0.HandleTLBException(address);
      // regs.cop0.FireException(Cop0::GetTLBExceptionCode(regs.cop0.tlbError, Cop0::LOAD), 0, regs.oldPC);
      Util::panic("[JIT]: Unhandled TLBL exception in LDL!");
    } else {
      const s32 shift = 8 * ((address ^ 0) & 7);
      const u64 mask = 0xFFFFFFFFFFFFFFFF << shift;
      const u64 data = mem.Read<u64>(regs, paddr & ~7);
      const s64 result = (s64)((regs.Read<s64>(RT(instr)) & ~mask) | (data << shift));
      regs.Write(RT(instr), result);
    }
  } else {
    Util::panic("[JIT]: Implement non constant LDL!");
  }
}

void JIT::ldr(u32 instr) {
  if (regs.IsRegConstant(RS(instr), RT(instr))) {
    const u64 address = regs.Read<s64>(RS(instr)) + (s16)instr;
    u32 paddr;
    if (!regs.cop0.MapVAddr(Cop0::LOAD, address, paddr)) {
      // regs.cop0.HandleTLBException(address);
      // regs.cop0.FireException(Cop0::GetTLBExceptionCode(regs.cop0.tlbError, Cop0::LOAD), 0, regs.oldPC);
      Util::panic("[JIT]: Unhandled TLBL exception in LDR!");
    } else {
      const s32 shift = 8 * ((address ^ 7) & 7);
      const u64 mask = 0xFFFFFFFFFFFFFFFF >> shift;
      const u64 data = mem.Read<u64>(regs, paddr & ~7);
      const s64 result = (s64)((regs.Read<s64>(RT(instr)) & ~mask) | (data >> shift));
      regs.Write(RT(instr), result);
    }
  } else {
    Util::panic("[JIT]: Implement non constant LDR!");
  }
}

void JIT::lh(u32 instr) {
  if (regs.IsRegConstant(RS(instr))) {
    const u64 address = regs.Read<s64>(RS(instr)) + (s16)instr;
    if (check_address_error(0b1, address)) {
      // regs.cop0.HandleTLBException(address);
      // regs.cop0.FireException(ExceptionCode::AddressErrorLoad, 0, regs.oldPC);
      // return;
      Util::panic("[JIT]: Unhandled ADEL exception in LH!");
      return;
    }

    u32 paddr = 0;
    if (!regs.cop0.MapVAddr(Cop0::LOAD, address, paddr)) {
      // regs.cop0.HandleTLBException(address);
      // regs.cop0.FireException(Cop0::GetTLBExceptionCode(regs.cop0.tlbError, Cop0::LOAD), 0, regs.oldPC);
      Util::panic("[JIT]: Unhandled TLBL exception in LH!");
      return;
    }

    code.mov(code.rsi, code.ptr[code.rbp + (reinterpret_cast<uintptr_t>(&regs) - reinterpret_cast<uintptr_t>(this))]);
    code.mov(code.edx, paddr);
    emitMemberFunctionCall(&Mem::Read<u16>, &mem);
    regs.Write<s16>(RT(instr), code.rax);
    return;
  }

  Util::panic("[JIT]: Implement non constant LH!");
}

void JIT::lhu(u32) {}

void JIT::ll(u32) {}

void JIT::lld(u32) {}

void JIT::lw(u32 instr) {
  if (regs.IsRegConstant(RS(instr))) {
    const s16 offset = instr;
    const u64 address = regs.Read<s64>(RS(instr)) + offset;
    if (check_address_error(0b11, address)) {
      // regs.cop0.HandleTLBException(address);
      // regs.cop0.FireException(ExceptionCode::AddressErrorLoad, 0, regs.oldPC);
      // return;
      Util::panic("[JIT]: Unhandled ADEL exception in LW!");
      return;
    }

    u32 paddr = 0;
    if (!regs.cop0.MapVAddr(Cop0::LOAD, address, paddr)) {
      // regs.cop0.HandleTLBException(address);
      // regs.cop0.FireException(Cop0::GetTLBExceptionCode(regs.cop0.tlbError, Cop0::LOAD), 0, regs.oldPC);
      Util::panic("[JIT]: Unhandled TLBL exception in LW!");
      return;
    }

    code.mov(code.rsi, code.ptr[code.rbp + (reinterpret_cast<uintptr_t>(&regs) - reinterpret_cast<uintptr_t>(this))]);
    code.mov(code.edx, paddr);
    emitMemberFunctionCall(&Mem::Read<u32>, &mem);
    regs.Write<s32>(RT(instr), code.rax);
    return;
  }

  Util::panic("[JIT]: Implement non constant LW!");
}

void JIT::lwc1(u32) {}

void JIT::lwl(u32) {}

void JIT::lwu(u32) {}

void JIT::lwr(u32) {}

void JIT::mfhi(u32 instr) {
  if (regs.hiIsConstant) {
    regs.Write(RD(instr), regs.hi);
  } else {
    Util::panic("[JIT]: Implement non constant MFHI!");
  }
}

void JIT::mflo(u32 instr) {
  if (regs.loIsConstant) {
    regs.Write(RD(instr), regs.lo);
  } else {
    Util::panic("[JIT]: Implement non constant MFLO!");
  }
}

void JIT::mult(u32 instr) {
  if (regs.IsRegConstant(RT(instr), RS(instr))) {
    auto rt = regs.Read<s32>(RT(instr));
    auto rs = regs.Read<s32>(RS(instr));
    s64 result = (s64)rt * (s64)rs;
    regs.lo = (s64)((s32)result);
    regs.loIsConstant = true;
    regs.hi = (s64)((s32)(result >> 32));
    regs.hiIsConstant = true;
  } else {
    Util::panic("[JIT]: Implement non constant MULT!");
  }
}

void JIT::multu(u32 instr) {
  if (regs.IsRegConstant(RT(instr), RS(instr))) {
    auto rt = regs.Read<u32>(RT(instr));
    auto rs = regs.Read<u32>(RS(instr));
    u64 result = (u64)rt * (u64)rs;
    regs.lo = (s64)((s32)result);
    regs.loIsConstant = true;
    regs.hi = (s64)((s32)(result >> 32));
    regs.hiIsConstant = true;
  } else {
    Util::panic("[JIT]: Implement non constant MULTU!");
  }
}

void JIT::mthi(u32 instr) {
  if (regs.IsRegConstant(RS(instr))) {
    regs.hi = regs.Read<s64>(RS(instr));
    regs.hiIsConstant = true;
  } else {
    Util::panic("[JIT]: Implement non constant MTHI!");
  }
}

void JIT::mtlo(u32 instr) {
  if (regs.IsRegConstant(RS(instr))) {
    regs.lo = regs.Read<s64>(RS(instr));
    regs.loIsConstant = true;
  } else {
    Util::panic("[JIT]: Implement non constant MTLO!");
  }
}

void JIT::nor(u32 instr) {
  if (regs.IsRegConstant(RS(instr), RT(instr))) {
    regs.Write(RD(instr), ~(regs.Read<s64>(RS(instr)) | regs.Read<s64>(RT(instr))));
  } else {
    Util::panic("[JIT]: Implement non constant NOR!");
  }
}

void JIT::slti(u32 instr) {
  if (regs.IsRegConstant(RS(instr))) {
    s16 imm = instr;
    regs.Write(RT(instr), regs.Read<s64>(RS(instr)) < imm);
  } else {
    Util::panic("[JIT]: Implement non constant SLTI!");
  }
}

void JIT::sltiu(u32 instr) {
  if (regs.IsRegConstant(RS(instr))) {
    s16 imm = instr;
    regs.Write(RT(instr), regs.Read<u64>(RS(instr)) < imm);
  } else {
    Util::panic("[JIT]: Implement non constant SLTIU!");
  }
}

void JIT::slt(u32 instr) {
  if (regs.IsRegConstant(RS(instr), RT(instr))) {
    regs.Write(RD(instr), regs.Read<s64>(RS(instr)) < regs.Read<s64>(RT(instr)));
  } else {
    Util::panic("[JIT]: Implement non constant SLT!");
  }
}

void JIT::sltu(u32 instr) {
  if (regs.IsRegConstant(RS(instr), RT(instr))) {
    regs.Write(RD(instr), regs.Read<u64>(RS(instr)) < regs.Read<u64>(RT(instr)));
  } else {
    Util::panic("[JIT]: Implement non constant SLT!");
  }
}

void JIT::sll(u32 instr) {
  if (regs.IsRegConstant(RT(instr))) {
    u8 sa = ((instr >> 6) & 0x1f);
    s32 result = regs.Read<s64>(RT(instr)) << sa;
    regs.Write(RD(instr), (s64)result);
  } else {
    Util::panic("[JIT]: Implement non constant SLL!");
  }
}

void JIT::sllv(u32 instr) {
  if (regs.IsRegConstant(RS(instr), RT(instr))) {
    u8 sa = (regs.Read<s64>(RS(instr))) & 0x1F;
    u32 rt = regs.Read<s64>(RT(instr));
    s32 result = rt << sa;
    regs.Write(RD(instr), (s64)result);
  } else {
    Util::panic("[JIT]: Implement non constant SLLV!");
  }
}

void JIT::sub(u32 instr) {
  if (regs.IsRegConstant(RS(instr), RT(instr))) {
    s32 rt = regs.Read<s64>(RT(instr));
    s32 rs = regs.Read<s64>(RS(instr));
    s32 result = rs - rt;
    if (check_signed_underflow(rs, rt, result)) {
      regs.cop0.FireException(ExceptionCode::Overflow, 0, regs.oldPC);
    } else {
      regs.Write(RD(instr), result);
    }
  } else {
    Util::panic("[JIT]: Implement non constant SUB!");
  }
}

void JIT::subu(u32 instr) {
  if (regs.IsRegConstant(RS(instr), RT(instr))) {
    u32 rt = regs.Read<s64>(RT(instr));
    u32 rs = regs.Read<s64>(RS(instr));
    u32 result = rs - rt;
    regs.Write(RD(instr), (s64)((s32)result));
  } else {
    Util::panic("[JIT]: Implement non constant SUBU!");
  }
}

void JIT::sra(u32 instr) {
  if (regs.IsRegConstant(RT(instr))) {
    s64 rt = regs.Read<s64>(RT(instr));
    u8 sa = ((instr >> 6) & 0x1f);
    s32 result = rt >> sa;
    regs.Write(RD(instr), result);
  } else {
    Util::panic("[JIT]: Implement non constant SRA!");
  }
}

void JIT::srav(u32 instr) {
  if (regs.IsRegConstant(RS(instr), RT(instr))) {
    s64 rs = regs.Read<s64>(RS(instr));
    s64 rt = regs.Read<s64>(RT(instr));
    u8 sa = rs & 0x1f;
    s32 result = rt >> sa;
    regs.Write(RD(instr), result);
  } else {
    Util::panic("[JIT]: Implement non constant SRAV!");
  }
}

void JIT::srl(u32 instr) {
  if (regs.IsRegConstant(RT(instr))) {
    u32 rt = regs.Read<s64>(RT(instr));
    u8 sa = ((instr >> 6) & 0x1f);
    u32 result = rt >> sa;
    regs.Write(RD(instr), (s32)result);
  } else {
    Util::panic("[JIT]: Implement non constant SRL!");
  }
}

void JIT::sw(const u32 instr) {
  if (regs.IsRegConstant(RS(instr), RT(instr))) {
    const s16 offset = instr;
    const u64 address = regs.Read<s64>(RS(instr)) + offset;
    if (check_address_error(0b11, address)) {
      // regs.cop0.HandleTLBException(address);
      // regs.cop0.FireException(ExceptionCode::AddressErrorStore, 0, regs.oldPC);
      Util::panic("[JIT]: Unhandled ADES exception in SW!");
      return;
    }

    u32 physical;
    if (!regs.cop0.MapVAddr(Cop0::STORE, address, physical)) {
      // regs.cop0.HandleTLBException(address);
      // regs.cop0.FireException(Cop0::GetTLBExceptionCode(regs.cop0.tlbError, Cop0::STORE), 0, regs.oldPC);
      Util::panic("[JIT]: Unhandled TLBS exception in SW!");
    } else {
      code.lea(code.rsi, code.ptr[code.rbp + (reinterpret_cast<uintptr_t>(&regs) - reinterpret_cast<uintptr_t>(this))]);
      code.mov(code.edx, physical);
      code.mov(code.rcx, regs.Read<s64>(RT(instr)));
      emitMemberFunctionCall(&Mem::WriteJIT<u32>, &mem);
    }

    return;
  }

  if (regs.IsRegConstant(RS(instr))) {
    const s16 offset = instr;
    const u64 address = regs.Read<s64>(RS(instr)) + offset;
    if (check_address_error(0b11, address)) {
      // regs.cop0.HandleTLBException(address);
      // regs.cop0.FireException(ExceptionCode::AddressErrorStore, 0, regs.oldPC);
      Util::panic("[JIT]: Unhandled ADES exception in SW!");
      return;
    }

    u32 physical;
    if (!regs.cop0.MapVAddr(Cop0::STORE, address, physical)) {
      // regs.cop0.HandleTLBException(address);
      // regs.cop0.FireException(Cop0::GetTLBExceptionCode(regs.cop0.tlbError, Cop0::STORE), 0, regs.oldPC);
      Util::panic("[JIT]: Unhandled TLBS exception in SW!");
    } else {
      code.mov(code.rsi, code.ptr[code.rbp + (reinterpret_cast<uintptr_t>(&regs) - reinterpret_cast<uintptr_t>(this))]);
      code.mov(code.edx, physical);
      regs.Read<s64>(RT(instr), code.rcx);
      emitMemberFunctionCall(&Mem::WriteJIT<u32>, &mem);
    }

    return;
  }

  if (regs.IsRegConstant(RT(instr))) {
    const s16 offset = instr;
    regs.Read<s64>(RS(instr), code.rdx);
    code.add(code.rdx, offset);

    code.mov(code.esi, Cop0::STORE);

    u32 physical;
    code.mov(code.rcx, reinterpret_cast<uintptr_t>(&physical));
    emitMemberFunctionCall(&Cop0::MapVAddr, &regs.cop0);

    code.mov(code.rsi, code.ptr[code.rbp + (reinterpret_cast<uintptr_t>(&regs) - reinterpret_cast<uintptr_t>(this))]);
    code.mov(code.edx, physical);
    code.mov(code.rcx, regs.Read<s64>(RT(instr)));
    emitMemberFunctionCall(&Mem::WriteJIT<u32>, &mem);

    return;
  }

  const s16 offset = instr;
  regs.Read<s64>(RS(instr), code.rdx);
  code.add(code.rdx, offset);

  code.mov(code.esi, Cop0::STORE);

  u32 physical;
  code.mov(code.rcx, reinterpret_cast<uintptr_t>(&physical));
  emitMemberFunctionCall(&Cop0::MapVAddr, &regs.cop0);

  code.mov(code.rsi, code.ptr[code.rbp + (reinterpret_cast<uintptr_t>(&regs) - reinterpret_cast<uintptr_t>(this))]);
  code.mov(code.edx, physical);
  regs.Read<s64>(RT(instr), code.rcx);
  emitMemberFunctionCall(&Mem::WriteJIT<u32>, &mem);
}

void JIT::srlv(u32 instr) {
  if (regs.IsRegConstant(RS(instr), RT(instr))) {
    u8 sa = (regs.Read<s64>(RS(instr)) & 0x1F);
    u32 rt = regs.Read<s64>(RT(instr));
    s32 result = rt >> sa;
    regs.Write(RD(instr), (s64)result);
  } else {
    Util::panic("[JIT]: Implement non constant SRLV!");
  }
}

void JIT::or_(u32 instr) {
  if (regs.IsRegConstant(RS(instr), RT(instr))) {
    regs.Write(RD(instr), regs.Read<s64>(RS(instr)) | regs.Read<s64>(RT(instr)));
  } else {
    Util::panic("[JIT]: Implement non constant OR!");
  }
}

void JIT::ori(u32 instr) {
  if (RT(instr) == 0)
    return;

  s64 imm = (u16)instr;
  if (regs.IsRegConstant(RS(instr))) {
    s64 result = imm | regs.Read<s64>(RS(instr));
    regs.Write(RT(instr), result);
    return;
  }

  regs.Read<s64>(RS(instr), code.rax);
  code.or_(code.rax, imm);
  regs.Write<s64>(RT(instr), code.rax);
}

void JIT::xori(u32 instr) {
  if (regs.IsRegConstant(RS(instr))) {
    s64 imm = (u16)instr;
    regs.Write(RT(instr), regs.Read<s64>(RS(instr)) ^ imm);
  } else {
    Util::panic("[JIT]: Implement non constant XORI!");
  }
}

void JIT::xor_(u32 instr) {
  if (regs.IsRegConstant(RS(instr), RT(instr))) {
    regs.Write(RD(instr), regs.Read<s64>(RT(instr)) ^ regs.Read<s64>(RS(instr)));
  } else {
    Util::panic("[JIT]: Implement non constant XOR!");
  }
}
} // namespace n64
