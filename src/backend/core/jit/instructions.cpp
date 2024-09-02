#include <JIT.hpp>

#define check_signed_overflow(op1, op2, res) (((~((op1) ^ (op2)) & ((op1) ^ (res))) >> ((sizeof(res) * 8) - 1)) & 1)
#define check_signed_underflow(op1, op2, res) (((((op1) ^ (op2)) & ((op1) ^ (res))) >> ((sizeof(res) * 8) - 1)) & 1)

namespace n64 {
void JIT::lui(u32 instr) {
  u64 val = s64((s16)instr);
  val <<= 16;
  regs.Write(RT(instr), val);
}

void JIT::add(u32 instr) {
  if (regs.IsRegConstant(RS(instr), RT(instr))) {
    u32 rs = regs.Read<s32>(RS(instr));
    u32 rt = regs.Read<s32>(RT(instr));
    u32 result = rs + rt;
    if (check_signed_overflow(rs, rt, result)) {
      // regs.cop0.FireException(ExceptionCode::Overflow, 0, regs.oldPC);
      Util::panic("[JIT]: Unhandled Overflow exception in ADD!");
    }
    regs.Write(RD(instr), s32(result));
  } else {
    Util::panic("[JIT]: Implement non constant ADD");
  }
}

void JIT::addu(u32 instr) {
  if (regs.IsRegConstant(RS(instr), RT(instr))) {
    s32 rs = regs.Read<s32>(RS(instr));
    s32 rt = regs.Read<s32>(RT(instr));
    s32 result = rs + rt;
    regs.Write(RD(instr), result);
  } else {
    Util::panic("[JIT]: Implement non constant ADDI");
  }
}

void JIT::addi(u32 instr) {
  if (regs.IsRegConstant(RS(instr))) {
    auto rs = regs.Read<u32>(RS(instr));
    u32 imm = s32(s16(instr));
    u32 result = rs + imm;
    if (check_signed_overflow(rs, imm, result)) {
      Util::panic("[JIT]: Unhandled Overflow exception in ADDI!");
    } else {
      regs.Write(RT(instr), s32(result));
    }
  } else {
    Util::panic("[JIT]: Implement non constant ADDI!");
  }
}

void JIT::addiu(u32 instr) {
  if (regs.IsRegConstant(RS(instr))) {
    auto rs = regs.Read<u32>(RS(instr));
    u32 imm = s32(s16(instr));
    u32 result = rs + imm;
    regs.Write(RT(instr), s32(result));
  } else {
    Util::panic("[JIT]: Implement non constant ADDIU!");
  }
}

void JIT::andi(u32 instr) {
  s64 imm = (u16)instr;
  if (regs.IsRegConstant(RS(instr))) {
    regs.Write(RT(instr), regs.Read<s64>(RS(instr)) & imm);
  } else {
    Util::panic("[JIT]: Implement non constant ANDI!");
  }
}

void JIT::and_(u32 instr) {
  if (regs.IsRegConstant(RS(instr), RT(instr))) {
    regs.Write(RD(instr), regs.Read<s64>(RS(instr)) & regs.Read<s64>(RT(instr)));
  } else {
    Util::panic("[JIT]: Implement non constant AND!");
  }
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
  if (regs.IsRegConstant(RS(instr))) {
    s64 imm = (u16)instr;
    s64 result = imm | regs.Read<s64>(RS(instr));
    regs.Write(RT(instr), result);
  } else {
    Util::panic("[JIT]: Implement non constant ORI!");
  }
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
