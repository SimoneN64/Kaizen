#include <core/JIT.hpp>

#define check_address_error(mask, vaddr) (((!regs.cop0.is_64bit_addressing) && (s32)(vaddr) != (vaddr)) || (((vaddr) & (mask)) != 0))
#define check_signed_overflow(op1, op2, res) (((~((op1) ^ (op2)) & ((op1) ^ (res))) >> ((sizeof(res) * 8) - 1)) & 1)
#define check_signed_underflow(op1, op2, res) (((((op1) ^ (op2)) & ((op1) ^ (res))) >> ((sizeof(res) * 8) - 1)) & 1)

namespace n64 {
void JIT::add(u32 instr) {
  u32 rs = (s32)regs.gpr[RS(instr)];
  u32 rt = (s32)regs.gpr[RT(instr)];
  u32 result = rs + rt;
  if(check_signed_overflow(rs, rt, result)) {
    FireException(regs, ExceptionCode::Overflow, 0, true);
  } else {
    if(likely(RD(instr) != 0)) {
      regs.gpr[RD(instr)] = s32(result);
    }
  }
}

void JIT::addu(u32 instr) {
  if(likely(RD(instr) != 0)) {
    s32 rs = (s32)regs.gpr[RS(instr)];
    s32 rt = (s32)regs.gpr[RT(instr)];
    s32 result = rs + rt;
    regs.gpr[RD(instr)] = result;
  }
}

void JIT::addi(u32 instr) {
  u32 rs = regs.gpr[RS(instr)];
  u32 imm = s32(s16(instr));
  u32 result = rs + imm;
  if(check_signed_overflow(rs, imm, result)) {
    FireException(regs, ExceptionCode::Overflow, 0, true);
  } else {
    regs.gpr[RT(instr)] = s32(result);
  }
}

void JIT::addiu(u32 instr) {
  s32 rs = (s32)regs.gpr[RS(instr)];
  s16 imm = (s16)(instr);
  s32 result = rs + imm;
  regs.gpr[RT(instr)] = result;
}

void JIT::dadd(u32 instr) {
  u64 rs = regs.gpr[RS(instr)];
  u64 rt = regs.gpr[RT(instr)];
  u64 result = rt + rs;
  if(check_signed_overflow(rs, rt, result)) {
    FireException(regs, ExceptionCode::Overflow, 0, true);
  } else {
    if(likely(RD(instr) != 0)) {
      regs.gpr[RD(instr)] = result;
    }
  }
}

void JIT::daddu(u32 instr) {
  if(likely(RD(instr) != 0)) {
    s64 rs = regs.gpr[RS(instr)];
    s64 rt = regs.gpr[RT(instr)];
    regs.gpr[RD(instr)] = rs + rt;
  }
}

void JIT::daddi(u32 instr) {
  u64 imm = s64(s16(instr));
  u64 rs = regs.gpr[RS(instr)];
  u64 result = imm + rs;
  if(check_signed_overflow(rs, imm, result)) {
    FireException(regs, ExceptionCode::Overflow, 0, true);
  } else {
    regs.gpr[RT(instr)] = result;
  }
}

void JIT::daddiu(u32 instr) {
  s16 imm = (s16)(instr);
  s64 rs = regs.gpr[RS(instr)];
  regs.gpr[RT(instr)] = rs + imm;
}

void JIT::div(u32 instr) {
  s64 dividend = (s32)regs.gpr[RS(instr)];
  s64 divisor = (s32)regs.gpr[RT(instr)];

  if(divisor == 0) {
    regs.hi = dividend;
    if(dividend >= 0) {
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
}

void JIT::divu(u32 instr) {
  u32 dividend = regs.gpr[RS(instr)];
  u32 divisor = regs.gpr[RT(instr)];
  if(divisor == 0) {
    regs.lo = -1;
    regs.hi = (s32)dividend;
  } else {
    s32 quotient = (s32)(dividend / divisor);
    s32 remainder = (s32)(dividend % divisor);
    regs.lo = quotient;
    regs.hi = remainder;
  }
}

void JIT::ddiv(u32 instr) {
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
}

void JIT::ddivu(u32 instr) {
  u64 dividend = regs.gpr[RS(instr)];
  u64 divisor = regs.gpr[RT(instr)];
  if(divisor == 0) {
    regs.lo = -1;
    regs.hi = (s64)dividend;
  } else {
    u64 quotient = dividend / divisor;
    u64 remainder = dividend % divisor;
    regs.lo = (s64)quotient;
    regs.hi = (s64)remainder;
  }
}

void JIT::branch(bool cond, s64 address) {
  regs.delaySlot = true;
  if (cond) {
    regs.nextPC = address;
  }
}

void JIT::branch_likely(bool cond, s64 address) {
  if (cond) {
    regs.delaySlot = true;
    regs.nextPC = address;
  } else {
    regs.SetPC64(regs.nextPC);
  }
}

void JIT::b(u32 instr, bool cond) {
  s16 imm = instr;
  s64 offset = u64((s64)imm) << 2;
  s64 address = regs.pc + offset;
  branch(cond, address);
}

void JIT::blink(u32 instr, bool cond) {
  regs.gpr[31] = regs.nextPC;
  s16 imm = instr;
  s64 offset = u64((s64)imm) << 2;
  s64 address = regs.pc + offset;
  branch(cond, address);
}

void JIT::bl(u32 instr, bool cond) {
  s16 imm = instr;
  s64 offset = u64((s64)imm) << 2;
  s64 address = regs.pc + offset;
  branch_likely(cond, address);
}

void JIT::bllink(u32 instr, bool cond) {
  regs.gpr[31] = regs.nextPC;
  s16 imm = instr;
  s64 offset = u64((s64)imm) << 2;
  s64 address = regs.pc + offset;
  branch_likely(cond, address);
}

void JIT::lui(u32 instr) {
  u64 val = s64((s16)instr);
  val <<= 16;
  regs.gpr[RT(instr)] = val;
}

void JIT::lb(u32 instr) {
  u64 address = regs.gpr[RS(instr)] + (s16)instr;
  u32 paddr = 0;
  if(!MapVAddr(regs, LOAD, address, paddr)) {
    HandleTLBException(regs, address);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, LOAD), 0, true);
  } else {
    regs.gpr[RT(instr)] = (s8)mem.Read8(regs, paddr);
  }
}

void JIT::lh(u32 instr) {
  u64 address = regs.gpr[RS(instr)] + (s16)instr;
  if ((address & 0b1) > 0) {
    HandleTLBException(regs, address);
    FireException(regs, ExceptionCode::AddressErrorLoad, 0, true);
    return;
  }

  u32 paddr = 0;
  if(!MapVAddr(regs, LOAD, address, paddr)) {
    HandleTLBException(regs, address);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, LOAD), 0, true);
  } else {
    regs.gpr[RT(instr)] = (s16)mem.Read16(regs, paddr);
  }
}

void JIT::lw(u32 instr) {
  s16 offset = instr;
  u64 address = regs.gpr[RS(instr)] + offset;
  if (check_address_error(0b11, address)) {
    HandleTLBException(regs, address);
    FireException(regs, ExceptionCode::AddressErrorLoad, 0, true);
    return;
  }

  u32 physical = 0;
  if (!MapVAddr(regs, LOAD, address, physical)) {
    HandleTLBException(regs, address);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, LOAD), 0, true);
  } else {
    regs.gpr[RT(instr)] = (s32)mem.Read32(regs, physical);
  }
}

void JIT::ll(u32 instr) {
  u64 address = regs.gpr[RS(instr)] + (s16)instr;
  u32 physical;
  if (!MapVAddr(regs, LOAD, address, physical)) {
    HandleTLBException(regs, address);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, LOAD), 0, true);
  } else {
    if ((address & 0b11) > 0) {
      FireException(regs, ExceptionCode::AddressErrorLoad, 0, true);
      return;
    } else {
      regs.gpr[RT(instr)] = (s32)mem.Read32(regs, physical);
    }
  }

  regs.cop0.llbit = true;
  regs.cop0.LLAddr = physical >> 4;
}

void JIT::lwl(u32 instr) {
  u64 address = regs.gpr[RS(instr)] + (s16)instr;
  u32 paddr = 0;
  if(!MapVAddr(regs, LOAD, address, paddr)) {
    HandleTLBException(regs, address);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, LOAD), 0, true);
  } else {
    u32 shift = 8 * ((address ^ 0) & 3);
    u32 mask = 0xFFFFFFFF << shift;
    u32 data = mem.Read32(regs, paddr & ~3);
    s32 result = s32((regs.gpr[RT(instr)] & ~mask) | (data << shift));
    regs.gpr[RT(instr)] = result;
  }
}

void JIT::lwr(u32 instr) {
  u64 address = regs.gpr[RS(instr)] + (s16)instr;
  u32 paddr = 0;
  if(!MapVAddr(regs, LOAD, address, paddr)) {
    HandleTLBException(regs, address);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, LOAD), 0, true);
  } else {
    u32 shift = 8 * ((address ^ 3) & 3);
    u32 mask = 0xFFFFFFFF >> shift;
    u32 data = mem.Read32(regs, paddr & ~3);
    s32 result = s32((regs.gpr[RT(instr)] & ~mask) | (data >> shift));
    regs.gpr[RT(instr)] = result;
  }
}

void JIT::ld(u32 instr) {
  s64 address = regs.gpr[RS(instr)] + (s16)instr;
  if (check_address_error(0b111, address)) {
    HandleTLBException(regs, address);
    FireException(regs, ExceptionCode::AddressErrorLoad, 0, true);
    return;
  }

  u32 paddr = 0;
  if(!MapVAddr(regs, LOAD, address, paddr)) {
    HandleTLBException(regs, address);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, LOAD), 0, true);
  } else {
    s64 value = mem.Read64(regs, paddr);
    regs.gpr[RT(instr)] = value;
  }
}

void JIT::lld(u32 instr) {
  if (!regs.cop0.is_64bit_addressing && !regs.cop0.kernel_mode) {
    FireException(regs, ExceptionCode::ReservedInstruction, 0, true);
    return;
  }

  u64 address = regs.gpr[RS(instr)] + (s16)instr;
  u32 paddr;
  if (!MapVAddr(regs, LOAD, address, paddr)) {
    HandleTLBException(regs, address);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, LOAD), 0, true);
  } else {
    if ((address & 0b111) > 0) {
      FireException(regs, ExceptionCode::AddressErrorLoad, 0, true);
    } else {
      regs.gpr[RT(instr)] = mem.Read64(regs, paddr);
      regs.cop0.llbit = true;
      regs.cop0.LLAddr = paddr >> 4;
    }
  }
}

void JIT::ldl(u32 instr) {
  u64 address = regs.gpr[RS(instr)] + (s16)instr;
  u32 paddr = 0;
  if (!MapVAddr(regs, LOAD, address, paddr)) {
    HandleTLBException(regs, address);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, LOAD), 0, true);
  } else {
    s32 shift = 8 * ((address ^ 0) & 7);
    u64 mask = 0xFFFFFFFFFFFFFFFF << shift;
    u64 data = mem.Read64(regs, paddr & ~7);
    s64 result = (s64) ((regs.gpr[RT(instr)] & ~mask) | (data << shift));
    regs.gpr[RT(instr)] = result;
  }
}

void JIT::ldr(u32 instr) {
  u64 address = regs.gpr[RS(instr)] + (s16)instr;
  u32 paddr;
  if (!MapVAddr(regs, LOAD, address, paddr)) {
    HandleTLBException(regs, address);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, LOAD), 0, true);
  } else {
    s32 shift = 8 * ((address ^ 7) & 7);
    u64 mask = 0xFFFFFFFFFFFFFFFF >> shift;
    u64 data = mem.Read64(regs, paddr & ~7);
    s64 result = (s64) ((regs.gpr[RT(instr)] & ~mask) | (data >> shift));
    regs.gpr[RT(instr)] = result;
  }
}

void JIT::lbu(u32 instr) {
  u64 address = regs.gpr[RS(instr)] + (s16)instr;
  u32 paddr;
  if (!MapVAddr(regs, LOAD, address, paddr)) {
    HandleTLBException(regs, address);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, LOAD), 0, true);
  } else {
    u8 value = mem.Read8(regs, paddr);
    regs.gpr[RT(instr)] = value;
  }
}

void JIT::lhu(u32 instr) {
  s64 address = regs.gpr[RS(instr)] + (s16)instr;
  if ((address & 0b1) > 0) {
    HandleTLBException(regs, address);
    FireException(regs, ExceptionCode::AddressErrorLoad, 0, true);
    return;
  }
  u32 paddr;
  if (!MapVAddr(regs, LOAD, address, paddr)) {
    HandleTLBException(regs, address);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, LOAD), 0, true);
  } else {
    u16 value = mem.Read16(regs, paddr);
    regs.gpr[RT(instr)] = value;
  }
}

void JIT::lwu(u32 instr) {
  s64 address = regs.gpr[RS(instr)] + (s16)instr;
  if ((address & 0b11) > 0) {
    HandleTLBException(regs, address);
    FireException(regs, ExceptionCode::AddressErrorLoad, 0, true);
    return;
  }

  u32 paddr;
  if (!MapVAddr(regs, LOAD, address, paddr)) {
    HandleTLBException(regs, address);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, LOAD), 0, true);
  } else {
    u32 value = mem.Read32(regs, paddr);
    regs.gpr[RT(instr)] = value;
  }
}

void JIT::sb(u32 instr) {
  u64 address = regs.gpr[RS(instr)] + (s16)instr;
  u32 paddr;
  if (!MapVAddr(regs, STORE, address, paddr)) {
    HandleTLBException(regs, address);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, STORE), 0, true);
  } else {
    mem.Write8(regs, paddr, regs.gpr[RT(instr)]);
  }
}

void JIT::sc(u32 instr) {
  u64 address = regs.gpr[RS(instr)] + (s16)instr;

  if ((address & 0b11) > 0) {
    FireException(regs, ExceptionCode::AddressErrorStore, 0, true);
    return;
  }

  if(regs.cop0.llbit) {
    regs.cop0.llbit = false;
    u32 paddr = 0;
    if(!MapVAddr(regs, STORE, address, paddr)) {
      HandleTLBException(regs, address);
      FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, STORE), 0, true);
    } else {
      mem.Write32(regs, paddr, regs.gpr[RT(instr)]);
      regs.gpr[RT(instr)] = 1;
    }
  } else {
    regs.gpr[RT(instr)] = 0;
  }
}

void JIT::scd(u32 instr) {
  if (!regs.cop0.is_64bit_addressing && !regs.cop0.kernel_mode) {
    FireException(regs, ExceptionCode::ReservedInstruction, 0, true);
    return;
  }

  s64 address = regs.gpr[RS(instr)] + (s16)instr;
  if ((address & 0b111) > 0) {
    HandleTLBException(regs, address);
    FireException(regs, ExceptionCode::AddressErrorStore, 0, true);
    return;
  }

  if(regs.cop0.llbit) {
    regs.cop0.llbit = false;
    u32 paddr = 0;
    if(!MapVAddr(regs, STORE, address, paddr)) {
      HandleTLBException(regs, address);
      FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, STORE), 0, true);
    } else {
      mem.Write32(regs, paddr, regs.gpr[RT(instr)]);
      regs.gpr[RT(instr)] = 1;
    }
  } else {
    regs.gpr[RT(instr)] = 0;
  }
}

void JIT::sh(u32 instr) {
  s64 address = regs.gpr[RS(instr)] + (s16)instr;

  u32 physical;
  if(!MapVAddr(regs, STORE, address, physical)) {
    HandleTLBException(regs, address);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, STORE), 0, true);
  } else {
    mem.Write16(regs, physical, regs.gpr[RT(instr)]);
  }
}

void JIT::sw(u32 instr) {
  s16 offset = instr;
  u64 address = regs.gpr[RS(instr)] + offset;
  if (check_address_error(0b11, address)) {
    HandleTLBException(regs, address);
    FireException(regs, ExceptionCode::AddressErrorStore, 0, true);
    return;
  }

  u32 physical;
  if(!MapVAddr(regs, STORE, address, physical)) {
    HandleTLBException(regs, address);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, STORE), 0, true);
  } else {
    mem.Write32(regs, physical, regs.gpr[RT(instr)]);
  }
}

void JIT::sd(u32 instr) {
  s64 address = regs.gpr[RS(instr)] + (s16)instr;
  if (check_address_error(0b111, address)) {
    HandleTLBException(regs, address);
    FireException(regs, ExceptionCode::AddressErrorStore, 0, true);
    return;
  }

  u32 physical;
  if(!MapVAddr(regs, STORE, address, physical)) {
    HandleTLBException(regs, address);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, STORE), 0, true);
  } else {
    mem.Write64(regs, physical, regs.gpr[RT(instr)]);
  }
}

void JIT::sdl(u32 instr) {
  u64 address = regs.gpr[RS(instr)] + (s16)instr;
  u32 paddr;
  if (!MapVAddr(regs, STORE, address, paddr)) {
    HandleTLBException(regs, address);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, STORE), 0, true);
  } else {
    s32 shift = 8 * ((address ^ 0) & 7);
    u64 mask = 0xFFFFFFFFFFFFFFFF >> shift;
    u64 data = mem.Read64(regs, paddr & ~7);
    u64 rt = regs.gpr[RT(instr)];
    mem.Write64(regs, paddr & ~7, (data & ~mask) | (rt >> shift));
  }
}

void JIT::sdr(u32 instr) {
  u64 address = regs.gpr[RS(instr)] + (s16)instr;
  u32 paddr;
  if (!MapVAddr(regs, STORE, address, paddr)) {
    HandleTLBException(regs, address);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, STORE), 0, true);
  } else {
    s32 shift = 8 * ((address ^ 7) & 7);
    u64 mask = 0xFFFFFFFFFFFFFFFF << shift;
    u64 data = mem.Read64(regs, paddr & ~7);
    u64 rt = regs.gpr[RT(instr)];
    mem.Write64(regs, paddr & ~7, (data & ~mask) | (rt << shift));
  }
}

void JIT::swl(u32 instr) {
  u64 address = regs.gpr[RS(instr)] + (s16)instr;
  u32 paddr;
  if (!MapVAddr(regs, STORE, address, paddr)) {
    HandleTLBException(regs, address);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, STORE), 0, true);
  } else {
    u32 shift = 8 * ((address ^ 0) & 3);
    u32 mask = 0xFFFFFFFF >> shift;
    u32 data = mem.Read32(regs, paddr & ~3);
    u32 rt = regs.gpr[RT(instr)];
    mem.Write32(regs, paddr & ~3, (data & ~mask) | (rt >> shift));
  }
}

void JIT::swr(u32 instr) {
  u64 address = regs.gpr[RS(instr)] + (s16)instr;
  u32 paddr;
  if (!MapVAddr(regs, STORE, address, paddr)) {
    HandleTLBException(regs, address);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, STORE), 0, true);
  } else {
    u32 shift = 8 * ((address ^ 3) & 3);
    u32 mask = 0xFFFFFFFF << shift;
    u32 data = mem.Read32(regs, paddr & ~3);
    u32 rt = regs.gpr[RT(instr)];
    mem.Write32(regs, paddr & ~3, (data & ~mask) | (rt << shift));
  }
}

void JIT::ori(u32 instr) {
  s64 imm = (u16)instr;
  s64 result = imm | regs.gpr[RS(instr)];
  regs.gpr[RT(instr)] = result;
}

void JIT::or_(u32 instr) {
  if(likely(RD(instr) != 0)) {
    regs.gpr[RD(instr)] = regs.gpr[RS(instr)] | regs.gpr[RT(instr)];
  }
}

void JIT::nor(u32 instr) {
  if(likely(RD(instr) != 0)) {
    regs.gpr[RD(instr)] = ~(regs.gpr[RS(instr)] | regs.gpr[RT(instr)]);
  }
}

void JIT::j(u32 instr) {
  s32 target = (instr & 0x3ffffff) << 2;
  s64 address = (regs.oldPC & ~0xfffffff) | target;

  branch(true, address);
}

void JIT::jal(u32 instr) {
  regs.gpr[31] = regs.nextPC;
  j(instr);
}

void JIT::jalr(u32 instr) {
  branch(true, regs.gpr[RS(instr)]);
  if(likely(RD(instr) != 0)) {
    regs.gpr[RD(instr)] = regs.pc + 4;
  }
}

void JIT::slti(u32 instr) {
  s16 imm = instr;
  regs.gpr[RT(instr)] = regs.gpr[RS(instr)] < imm;
}

void JIT::sltiu(u32 instr) {
  s16 imm = instr;
  regs.gpr[RT(instr)] = (u64)regs.gpr[RS(instr)] < imm;
}

void JIT::slt(u32 instr) {
  if(likely(RD(instr) != 0)) {
    regs.gpr[RD(instr)] = regs.gpr[RS(instr)] < regs.gpr[RT(instr)];
  }
}

void JIT::sltu(u32 instr) {
  if(likely(RD(instr) != 0)) {
    regs.gpr[RD(instr)] = (u64) regs.gpr[RS(instr)] < (u64) regs.gpr[RT(instr)];
  }
}

void JIT::xori(u32 instr) {
  s64 imm = (u16)instr;
  regs.gpr[RT(instr)] = regs.gpr[RS(instr)] ^ imm;
}

void JIT::xor_(u32 instr) {
  if(likely(RD(instr) != 0)) {
    regs.gpr[RD(instr)] = regs.gpr[RT(instr)] ^ regs.gpr[RS(instr)];
  }
}

void JIT::andi(u32 instr) {
  s64 imm = (u16)instr;
  regs.gpr[RT(instr)] = regs.gpr[RS(instr)] & imm;
}

void JIT::and_(u32 instr) {
  if(likely(RD(instr) != 0)) {
    regs.gpr[RD(instr)] = regs.gpr[RS(instr)] & regs.gpr[RT(instr)];
  }
}

void JIT::sll(u32 instr) {
  if(likely(RD(instr) != 0)) {
    u8 sa = ((instr >> 6) & 0x1f);
    s32 result = regs.gpr[RT(instr)] << sa;
    regs.gpr[RD(instr)] = (s64) result;
  }
}

void JIT::sllv(u32 instr) {
  if(likely(RD(instr) != 0)) {
    u8 sa = (regs.gpr[RS(instr)]) & 0x1F;
    u32 rt = regs.gpr[RT(instr)];
    s32 result = rt << sa;
    regs.gpr[RD(instr)] = (s64) result;
  }
}

void JIT::dsll32(u32 instr) {
  if(likely(RD(instr) != 0)) {
    u8 sa = ((instr >> 6) & 0x1f);
    s64 result = regs.gpr[RT(instr)] << (sa + 32);
    regs.gpr[RD(instr)] = result;
  }
}

void JIT::dsll(u32 instr) {
  if(likely(RD(instr) != 0)) {
    u8 sa = ((instr >> 6) & 0x1f);
    s64 result = regs.gpr[RT(instr)] << sa;
    regs.gpr[RD(instr)] = result;
  }
}

void JIT::dsllv(u32 instr) {
  if(likely(RD(instr) != 0)) {
    s64 sa = regs.gpr[RS(instr)] & 63;
    s64 result = regs.gpr[RT(instr)] << sa;
    regs.gpr[RD(instr)] = result;
  }
}

void JIT::srl(u32 instr) {
  if(likely(RD(instr) != 0)) {
    u32 rt = regs.gpr[RT(instr)];
    u8 sa = ((instr >> 6) & 0x1f);
    u32 result = rt >> sa;
    regs.gpr[RD(instr)] = (s32) result;
  }
}

void JIT::srlv(u32 instr) {
  if(likely(RD(instr) != 0)) {
    u8 sa = (regs.gpr[RS(instr)] & 0x1F);
    u32 rt = regs.gpr[RT(instr)];
    s32 result = rt >> sa;
    regs.gpr[RD(instr)] = (s64) result;
  }
}

void JIT::dsrl(u32 instr) {
  if(likely(RD(instr) != 0)) {
    u64 rt = regs.gpr[RT(instr)];
    u8 sa = ((instr >> 6) & 0x1f);
    u64 result = rt >> sa;
    regs.gpr[RD(instr)] = s64(result);
  }
}

void JIT::dsrlv(u32 instr) {
  if(likely(RD(instr) != 0)) {
    u8 amount = (regs.gpr[RS(instr)] & 63);
    u64 rt = regs.gpr[RT(instr)];
    u64 result = rt >> amount;
    regs.gpr[RD(instr)] = s64(result);
  }
}

void JIT::dsrl32(u32 instr) {
  if(likely(RD(instr) != 0)) {
    u64 rt = regs.gpr[RT(instr)];
    u8 sa = ((instr >> 6) & 0x1f);
    u64 result = rt >> (sa + 32);
    regs.gpr[RD(instr)] = s64(result);
  }
}

void JIT::sra(u32 instr) {
  if(likely(RD(instr) != 0)) {
    s64 rt = regs.gpr[RT(instr)];
    u8 sa = ((instr >> 6) & 0x1f);
    s32 result = rt >> sa;
    regs.gpr[RD(instr)] = result;
  }
}

void JIT::srav(u32 instr) {
  s64 rt = regs.gpr[RT(instr)];
  if(likely(RD(instr) != 0)) {
    s64 rs = regs.gpr[RS(instr)];
    u8 sa = rs & 0x1f;
    s32 result = rt >> sa;
    regs.gpr[RD(instr)] = result;
  }
}

void JIT::dsra(u32 instr) {
  if(likely(RD(instr) != 0)) {
    s64 rt = regs.gpr[RT(instr)];
    u8 sa = ((instr >> 6) & 0x1f);
    s64 result = rt >> sa;
    regs.gpr[RD(instr)] = result;
  }
}

void JIT::dsrav(u32 instr) {
  if(likely(RD(instr) != 0)) {
    s64 rt = regs.gpr[RT(instr)];
    s64 rs = regs.gpr[RS(instr)];
    s64 sa = rs & 63;
    s64 result = rt >> sa;
    regs.gpr[RD(instr)] = result;
  }
}

void JIT::dsra32(u32 instr) {
  if(likely(RD(instr) != 0)) {
    s64 rt = regs.gpr[RT(instr)];
    u8 sa = ((instr >> 6) & 0x1f);
    s64 result = rt >> (sa + 32);
    regs.gpr[RD(instr)] = result;
  }
}

void JIT::jr(u32 instr) {
  s64 address = regs.gpr[RS(instr)];
  branch(true, address);
}

void JIT::dsub(u32 instr) {
  s64 rt = regs.gpr[RT(instr)];
  s64 rs = regs.gpr[RS(instr)];
  s64 result = rs - rt;
  if(check_signed_underflow(rs, rt, result)) {
    FireException(regs, ExceptionCode::Overflow, 0, true);
  } else {
    if(likely(RD(instr) != 0)) {
      regs.gpr[RD(instr)] = result;
    }
  }
}

void JIT::dsubu(u32 instr) {
  if(likely(RD(instr) != 0)) {
    u64 rt = regs.gpr[RT(instr)];
    u64 rs = regs.gpr[RS(instr)];
    u64 result = rs - rt;
    regs.gpr[RD(instr)] = s64(result);
  }
}

void JIT::sub(u32 instr) {
  s32 rt = regs.gpr[RT(instr)];
  s32 rs = regs.gpr[RS(instr)];
  s32 result = rs - rt;
  if(check_signed_underflow(rs, rt, result)) {
    FireException(regs, ExceptionCode::Overflow, 0, true);
  } else {
    if(likely(RD(instr) != 0)) {
      regs.gpr[RD(instr)] = result;
    }
  }
}

void JIT::subu(u32 instr) {
  if(likely(RD(instr) != 0)) {
    u32 rt = regs.gpr[RT(instr)];
    u32 rs = regs.gpr[RS(instr)];
    u32 result = rs - rt;
    regs.gpr[RD(instr)] = (s64) ((s32) result);
  }
}

void JIT::dmultu(u32 instr) {
  u64 rt = regs.gpr[RT(instr)];
  u64 rs = regs.gpr[RS(instr)];
  u128 result = (u128)rt * (u128)rs;
  regs.lo = (s64)(result & 0xFFFFFFFFFFFFFFFF);
  regs.hi = (s64)(result >> 64);
}

void JIT::dmult(u32 instr) {
  s64 rt = regs.gpr[RT(instr)];
  s64 rs = regs.gpr[RS(instr)];
  s128 result = (s128)rt * (s128)rs;
  regs.lo = result & 0xFFFFFFFFFFFFFFFF;
  regs.hi = result >> 64;
}

void JIT::multu(u32 instr) {
  u32 rt = regs.gpr[RT(instr)];
  u32 rs = regs.gpr[RS(instr)];
  u64 result = (u64)rt * (u64)rs;
  regs.lo = (s64)((s32)result);
  regs.hi = (s64)((s32)(result >> 32));
}

void JIT::mult(u32 instr) {
  s32 rt = regs.gpr[RT(instr)];
  s32 rs = regs.gpr[RS(instr)];
  s64 result = (s64)rt * (s64)rs;
  regs.lo = (s64)((s32)result);
  regs.hi = (s64)((s32)(result >> 32));
}

void JIT::mflo(u32 instr) {
  if(likely(RD(instr) != 0)) {
    regs.gpr[RD(instr)] = regs.lo;
  }
}

void JIT::mfhi(u32 instr) {
  if(likely(RD(instr) != 0)) {
    regs.gpr[RD(instr)] = regs.hi;
  }
}

void JIT::mtlo(u32 instr) {
  regs.lo = regs.gpr[RS(instr)];
}

void JIT::mthi(u32 instr) {
  regs.hi = regs.gpr[RS(instr)];
}

void JIT::trap(bool cond) {
  if(cond) {
    FireException(regs, ExceptionCode::Trap, 0, true);
  }
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