#include <JIT.hpp>

#define check_signed_overflow(op1, op2, res) (((~((op1) ^ (op2)) & ((op1) ^ (res))) >> ((sizeof(res) * 8) - 1)) & 1)
#define check_signed_underflow(op1, op2, res) (((((op1) ^ (op2)) & ((op1) ^ (res))) >> ((sizeof(res) * 8) - 1)) & 1)

namespace n64 {
void JIT::lui(u32 instr) {
  u64 val = s64((s16)instr);
  val <<= 16;
  if (RT(instr) != 0) [[likely]] {
    regs.gpr[RT(instr)] = val;
    regs.gprIsConstant[RT(instr)] = true;
  }
}

void JIT::add(u32 instr) {
  if(regs.IsRegConstant(RS(instr), RT(instr))) {
    u32 rs = (s32)regs.gpr[RS(instr)];
    u32 rt = (s32)regs.gpr[RT(instr)];
    u32 result = rs + rt;
    if(check_signed_overflow(rs, rt, result)) {
      //regs.cop0.FireException(ExceptionCode::Overflow, 0, regs.oldPC);
      Util::panic("[JIT]: Unhandled Overflow exception in ADD!");
    }

    if (RD(instr) != 0) [[likely]] {
      regs.gpr[RD(instr)] = s32(result);
      regs.gprIsConstant[RD(instr)] = true;
    }
  } else {
    Util::panic("[JIT]: Implement non constant ADD");
  }
}

void JIT::addu(u32 instr) {
  if(regs.IsRegConstant(RS(instr), RT(instr))) {
    s32 rs = (s32) regs.gpr[RS(instr)];
    s32 rt = (s32) regs.gpr[RT(instr)];
    s32 result = rs + rt;

    if (RD(instr) != 0) [[likely]] {
      regs.gpr[RD(instr)] = result;
      regs.gprIsConstant[RD(instr)] = true;
    }
  } else {
    Util::panic("[JIT]: Implement non constant ADDI");
  }
}

void JIT::addi(u32 instr) {
  if(regs.IsRegConstant(RS(instr))) {
    u32 rs = regs.gpr[RS(instr)];
    u32 imm = s32(s16(instr));
    u32 result = rs + imm;
    if(check_signed_overflow(rs, imm, result)) {
      Util::panic("[JIT]: Unhandled Overflow exception in ADDI!");
    } else {
      if (RT(instr) != 0) [[likely]] {
        regs.gpr[RT(instr)] = s32(result);
        regs.gprIsConstant[RT(instr)] = true;
      }
    }
  } else {
    Util::panic("[JIT]: Implement non constant ADDI!");
  }
}

void JIT::addiu(u32 instr) {
  if(regs.IsRegConstant(RS(instr))) {
    u32 rs = regs.gpr[RS(instr)];
    u32 imm = s32(s16(instr));
    u32 result = rs + imm;

    if (RT(instr) != 0) [[likely]] {
      regs.gpr[RT(instr)] = s32(result);
      regs.gprIsConstant[RT(instr)] = true;
    }
  } else {
    Util::panic("[JIT]: Implement non constant ADDIU!");
  }
}

void JIT::andi(u32 instr) {
  s64 imm = (u16)instr;
  if(regs.IsRegConstant(RS(instr))) {
    if (RT(instr) != 0) [[likely]] {
      regs.gpr[RT(instr)] = regs.gpr[RS(instr)] & imm;
      regs.gprIsConstant[RT(instr)] = true;
    }
  } else {
    Util::panic("[JIT]: Implement non constant ANDI!");
  }
}

void JIT::and_(u32 instr) {
  if(regs.IsRegConstant(RS(instr), RT(instr))) {
    if (RD(instr) != 0) [[likely]] {
      regs.gpr[RD(instr)] = regs.gpr[RS(instr)] & regs.gpr[RT(instr)];
      regs.gprIsConstant[RD(instr)] = true;
    }
  } else {
    Util::panic("[JIT]: Implement non constant AND!");
  }
}

void JIT::dadd(u32 instr) {
  if(regs.IsRegConstant(RS(instr), RT(instr))) {
    u64 rs = regs.gpr[RS(instr)];
    u64 rt = regs.gpr[RT(instr)];
    u64 result = rt + rs;
    if (check_signed_overflow(rs, rt, result)) {
      //regs.cop0.FireException(ExceptionCode::Overflow, 0, regs.oldPC);
      Util::panic("[JIT]: Unhandled Overflow exception in DADD!");
    }

    if (RD(instr) != 0) [[likely]] {
      regs.gpr[RD(instr)] = result;
      regs.gprIsConstant[RD(instr)] = true;
    }
  } else {
    Util::panic("[JIT]: Implement non constant DADD!");
  }
}

void JIT::daddu(u32 instr) {
  if(regs.IsRegConstant(RS(instr), RT(instr))) {
    if (RD(instr) != 0) [[likely]] {
      s64 rs = regs.gpr[RS(instr)];
      s64 rt = regs.gpr[RT(instr)];
      regs.gpr[RD(instr)] = rt + rs;
      regs.gprIsConstant[RD(instr)] = true;
    }
  } else {
    Util::panic("[JIT]: Implement non constant DADD!");
  }
}

void JIT::daddi(u32 instr) {
  if(regs.IsRegConstant(RS(instr))) {
    u64 imm = s64(s16(instr));
    u64 rs = regs.gpr[RS(instr)];
    u64 result = imm + rs;
    if (check_signed_overflow(rs, imm, result)) {
      //regs.cop0.FireException(ExceptionCode::Overflow, 0, regs.oldPC);
      Util::panic("[JIT]: Unhandled Overflow exception in DADDI!");
    }

    if (RT(instr) != 0) [[likely]] {
      regs.gpr[RT(instr)] = result;
      regs.gprIsConstant[RT(instr)] = true;
    }
  } else {
    Util::panic("[JIT]: Implement non constant DADDI!");
  }
}

void JIT::daddiu(u32 instr) {
  if(regs.IsRegConstant(RS(instr))) {
    if (RT(instr) != 0) [[likely]] {
      s16 imm = s16(instr);
      s64 rs = regs.gpr[RS(instr)];
      regs.gpr[RT(instr)] = imm + rs;
      regs.gprIsConstant[RT(instr)] = true;
    }
  } else {
    Util::panic("[JIT]: Implement non constant DADDI!");
  }
}

void JIT::ddiv(u32 instr) {
  if(regs.IsRegConstant(RS(instr), RT(instr))) {
    s64 dividend = regs.gpr[RS(instr)];
    s64 divisor = regs.gpr[RT(instr)];
    if (dividend == 0x8000000000000000 && divisor == 0xFFFFFFFFFFFFFFFF) {
      regs.lo = dividend;
      regs.hi = 0;
    } else if(divisor == 0) {
      regs.hi = dividend;
      if(dividend >= 0) {
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
  if(regs.IsRegConstant(RS(instr), RT(instr))) {
    u64 dividend = regs.gpr[RS(instr)];
    u64 divisor = regs.gpr[RT(instr)];
    if (divisor == 0) {
      regs.lo = -1;
      regs.hi = (s64) dividend;
    } else {
      u64 quotient = dividend / divisor;
      u64 remainder = dividend % divisor;
      regs.lo = (s64) quotient;
      regs.hi = (s64) remainder;
    }

    regs.loIsConstant = true;
    regs.hiIsConstant = true;
  } else {
    Util::panic("[JIT]: Implement non constant DDIVU!");
  }
}

void JIT::div(u32 instr) {
  if(regs.IsRegConstant(RS(instr), RT(instr))) {
    s64 dividend = (s32) regs.gpr[RS(instr)];
    s64 divisor = (s32) regs.gpr[RT(instr)];

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
  if(regs.IsRegConstant(RS(instr), RT(instr))) {
    u32 dividend = regs.gpr[RS(instr)];
    u32 divisor = regs.gpr[RT(instr)];
    if (divisor == 0) {
      regs.lo = -1;
      regs.hi = (s32) dividend;
    } else {
      s32 quotient = (s32) (dividend / divisor);
      s32 remainder = (s32) (dividend % divisor);
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
  if(regs.IsRegConstant(RT(instr), RS(instr))) {
    s64 rt = regs.gpr[RT(instr)];
    s64 rs = regs.gpr[RS(instr)];
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
  if(regs.IsRegConstant(RT(instr), RS(instr))) {
    u64 rt = regs.gpr[RT(instr)];
    u64 rs = regs.gpr[RS(instr)];
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
  if(regs.IsRegConstant(RT(instr))) {
    if (RD(instr) != 0) [[likely]] {
      u8 sa = ((instr >> 6) & 0x1f);
      s64 result = regs.gpr[RT(instr)] << sa;
      regs.gpr[RD(instr)] = result;
      regs.gprIsConstant[RD(instr)] = true;
    }
  } else {
    Util::panic("[JIT]: Implement non constant DSLL!");
  }
}

void JIT::dsllv(u32 instr) {
  if(regs.IsRegConstant(RT(instr), RS(instr))) {
    if (RD(instr) != 0) [[likely]] {
      s64 sa = regs.gpr[RS(instr)] & 63;
      s64 result = regs.gpr[RT(instr)] << sa;
      regs.gpr[RD(instr)] = result;
      regs.gprIsConstant[RD(instr)] = true;
    }
  } else {
    Util::panic("[JIT]: Implement non constant DSLLV!");
  }
}

void JIT::dsll32(u32 instr) {
  if(regs.IsRegConstant(RT(instr))) {
    if (RD(instr) != 0) [[likely]] {
      u8 sa = ((instr >> 6) & 0x1f);
      s64 result = regs.gpr[RT(instr)] << (sa + 32);
      regs.gpr[RD(instr)] = result;
      regs.gprIsConstant[RD(instr)] = true;
    }
  } else {
    Util::panic("[JIT]: Implement non constant DSLL32!");
  }
}

void JIT::dsra(u32 instr) {
  if(regs.IsRegConstant(RT(instr))) {
    if (RD(instr) != 0) [[likely]] {
      s64 rt = regs.gpr[RT(instr)];
      u8 sa = ((instr >> 6) & 0x1f);
      s64 result = rt >> sa;
      regs.gpr[RD(instr)] = result;
      regs.gprIsConstant[RD(instr)] = true;
    }
  } else {
    Util::panic("[JIT]: Implement non constant DSRA!");
  }
}

void JIT::dsrav(u32 instr) {
  if(regs.IsRegConstant(RT(instr), RS(instr))) {
    if (RD(instr) != 0) [[likely]] {
      s64 rt = regs.gpr[RT(instr)];
      s64 rs = regs.gpr[RS(instr)];
      s64 sa = rs & 63;
      s64 result = rt >> sa;
      regs.gpr[RD(instr)] = result;
      regs.gprIsConstant[RD(instr)] = true;
    }
  } else {
    Util::panic("[JIT]: Implement non constant DSRAV!");
  }
}

void JIT::dsra32(u32 instr) {
  if(regs.IsRegConstant(RT(instr))) {
    if (RD(instr) != 0) [[likely]] {
      s64 rt = regs.gpr[RT(instr)];
      u8 sa = ((instr >> 6) & 0x1f);
      s64 result = rt >> (sa + 32);
      regs.gpr[RD(instr)] = result;
      regs.gprIsConstant[RD(instr)] = true;
    }
  } else {
    Util::panic("[JIT]: Implement non constant DSRA32!");
  }
}

void JIT::dsrl(u32 instr) {
  if(regs.IsRegConstant(RT(instr))) {
    if (RD(instr) != 0) [[likely]] {
      u64 rt = regs.gpr[RT(instr)];
      u8 sa = ((instr >> 6) & 0x1f);
      u64 result = rt >> sa;
      regs.gpr[RD(instr)] = s64(result);
      regs.gprIsConstant[RD(instr)] = true;
    }
  } else {
    Util::panic("[JIT]: Implement non constant DSRL!");
  }
}

void JIT::dsrlv(u32 instr) {
  if(regs.IsRegConstant(RT(instr), RS(instr))) {
    if (RD(instr) != 0) [[likely]] {
      u8 amount = (regs.gpr[RS(instr)] & 63);
      u64 rt = regs.gpr[RT(instr)];
      u64 result = rt >> amount;
      regs.gpr[RD(instr)] = s64(result);
      regs.gprIsConstant[RD(instr)] = true;
    }
  } else {
    Util::panic("[JIT]: Implement non constant DSRLV!");
  }
}

void JIT::dsrl32(u32 instr) {
  if(regs.IsRegConstant(RT(instr))) {
    if (RD(instr) != 0) [[likely]] {
      u64 rt = regs.gpr[RT(instr)];
      u8 sa = ((instr >> 6) & 0x1f);
      u64 result = rt >> (sa + 32);
      regs.gpr[RD(instr)] = s64(result);
      regs.gprIsConstant[RD(instr)] = true;
    }
  } else {
    Util::panic("[JIT]: Implement non constant DSRL32!");
  }
}

void JIT::dsub(u32 instr) {
  if(regs.IsRegConstant(RT(instr), RS(instr))) {
    s64 rt = regs.gpr[RT(instr)];
    s64 rs = regs.gpr[RS(instr)];
    s64 result = rs - rt;
    if (check_signed_underflow(rs, rt, result)) {
      // regs.cop0.FireException(ExceptionCode::Overflow, 0, regs.oldPC);
      Util::panic("[JIT]: Unhandled Overflow exception in DSUB!");
    } else {
      if (RD(instr) != 0) [[likely]] {
        regs.gpr[RD(instr)] = result;
        regs.gprIsConstant[RD(instr)] = true;
      }
    }
  } else {
    Util::panic("[JIT]: Implement non constant DSUB!");
  }
}

void JIT::dsubu(u32 instr) {
  if(regs.IsRegConstant(RT(instr), RS(instr))) {
    s64 rt = regs.gpr[RT(instr)];
    s64 rs = regs.gpr[RS(instr)];
    s64 result = rs - rt;

    if (RD(instr) != 0) [[likely]] {
      regs.gpr[RD(instr)] = result;
      regs.gprIsConstant[RD(instr)] = true;
    }
  } else {
    Util::panic("[JIT]: Implement non constant DSUBU!");
  }
}
}