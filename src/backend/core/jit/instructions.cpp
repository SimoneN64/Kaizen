#include <core/JIT.hpp>

#define se_imm(x) ((s16)((x) & 0xFFFF))
#define check_address_error(mask, addr) (((!regs.cop0.is_64bit_addressing) && (s32)(addr) != (addr)) || (((addr) & (mask)) != 0))
#define check_signed_overflow(op1, op2, res) (((~((op1) ^ (op2)) & ((op1) ^ (res))) >> ((sizeof(res) * 8) - 1)) & 1)
#define check_signed_underflow(op1, op2, res) (((((op1) ^ (op2)) & ((op1) ^ (res))) >> ((sizeof(res) * 8) - 1)) & 1)

namespace n64 {
void add(JIT& dyn, u32 instr) {
  Registers& regs = dyn.regs;
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

void addu(JIT& dyn, u32 instr) {
  Registers& regs = dyn.regs;
  if(likely(RD(instr) != 0)) {
    s32 rs = (s32)regs.gpr[RS(instr)];
    s32 rt = (s32)regs.gpr[RT(instr)];
    s32 result = rs + rt;
    regs.gpr[RD(instr)] = result;
  }
}

void addi(JIT& dyn, u32 instr) {
  Registers& regs = dyn.regs;
  u32 rs = regs.gpr[RS(instr)];
  u32 imm = s32(s16(instr));
  u32 result = rs + imm;
  if(check_signed_overflow(rs, imm, result)) {
    FireException(regs, ExceptionCode::Overflow, 0, true);
  } else {
    regs.gpr[RT(instr)] = s32(result);
  }
}

void addiu(JIT& dyn, u32 instr) {
  Registers& regs = dyn.regs;
  s32 rs = (s32)regs.gpr[RS(instr)];
  s16 imm = (s16)(instr);
  s32 result = rs + imm;
  regs.gpr[RT(instr)] = result;
}

void dadd(JIT& dyn, u32 instr) {
  Registers& regs = dyn.regs;
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

void daddu(JIT& dyn, u32 instr) {
  Registers& regs = dyn.regs;
  if(likely(RD(instr) != 0)) {
    s64 rs = regs.gpr[RS(instr)];
    s64 rt = regs.gpr[RT(instr)];
    regs.gpr[RD(instr)] = rs + rt;
  }
}

void daddi(JIT& dyn, u32 instr) {
  Registers& regs = dyn.regs;
  u64 imm = s64(s16(instr));
  u64 rs = regs.gpr[RS(instr)];
  u64 result = imm + rs;
  if(check_signed_overflow(rs, imm, result)) {
    FireException(regs, ExceptionCode::Overflow, 0, true);
  } else {
    regs.gpr[RT(instr)] = result;
  }
}

void daddiu(JIT& dyn, u32 instr) {
  Registers& regs = dyn.regs;
  s16 imm = (s16)(instr);
  s64 rs = regs.gpr[RS(instr)];
  regs.gpr[RT(instr)] = rs + imm;
}

void div(JIT& dyn, u32 instr) {
  Registers& regs = dyn.regs;
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

void divu(JIT& dyn, u32 instr) {
  Registers& regs = dyn.regs;
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

void ddiv(JIT& dyn, u32 instr) {
  Registers& regs = dyn.regs;
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

void ddivu(JIT& dyn, u32 instr) {
  Registers& regs = dyn.regs;
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

void branch(JIT& dyn, bool cond, s64 address) {
  Registers& regs = dyn.regs;
  regs.delaySlot = true;
  if (cond) {
    regs.nextPC = address;
  }
}

void branch_likely(JIT& dyn, bool cond, s64 address) {
  Registers& regs = dyn.regs;
  regs.delaySlot = true;
  if (cond) {
    regs.nextPC = address;
  } else {
    regs.SetPC64(regs.nextPC);
  }
}

void b(JIT& dyn, u32 instr, bool cond) {
  Registers& regs = dyn.regs;
  s64 offset = (s64)se_imm(instr) << 2;
  s64 address = regs.pc + offset;
  branch(dyn, cond, address);
}

void blink(JIT& dyn, u32 instr, bool cond) {
  Registers& regs = dyn.regs;
  regs.gpr[31] = regs.nextPC;
  s64 offset = (s64)se_imm(instr) << 2;
  s64 address = regs.pc + offset;
  branch(dyn, cond, address);
}

void bl(JIT& dyn, u32 instr, bool cond) {
  Registers& regs = dyn.regs;
  s64 offset = (s64)se_imm(instr) << 2;
  s64 address = regs.pc + offset;
  branch_likely(dyn, cond, address);
}

void bllink(JIT& dyn, u32 instr, bool cond) {
  Registers& regs = dyn.regs;
  regs.gpr[31] = regs.nextPC;
  s64 offset = (s64)se_imm(instr) << 2;
  s64 address = regs.pc + offset;
  branch_likely(dyn, cond, address);
}

void lui(JIT& dyn, u32 instr) {
  Registers& regs = dyn.regs;
  s64 val = (s16)instr;
  val <<= 16;
  regs.gpr[RT(instr)] = val;
}

void lb(JIT& dyn, u32 instr) {
  Registers& regs = dyn.regs;
  Mem& mem = dyn.mem;
  u64 address = regs.gpr[RS(instr)] + (s16)instr;
  u32 paddr = 0;
  if(!MapVAddr(regs, LOAD, address, paddr)) {
    HandleTLBException(regs, address);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, LOAD), 0, true);
  } else {
    regs.gpr[RT(instr)] = (s8)mem.Read8(regs, paddr);
  }
}

void lh(JIT& dyn, u32 instr) {
  Registers& regs = dyn.regs;
  Mem& mem = dyn.mem;
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
    regs.gpr[RT(instr)] = (s16)mem.Read16(regs, address);
  }
}

void lw(JIT& dyn, u32 instr) {
  Registers& regs = dyn.regs;
  Mem& mem = dyn.mem;
  s16 offset = instr;
  u64 address = regs.gpr[RS(instr)] + offset;
  if (check_address_error(address, 0b11)) {
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

void ll(JIT& dyn, u32 instr) {
  Registers& regs = dyn.regs;
  Mem& mem = dyn.mem;
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

void lwl(JIT& dyn, u32 instr) {
  Registers& regs = dyn.regs;
  Mem& mem = dyn.mem;
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

void lwr(JIT& dyn, u32 instr) {
  Registers& regs = dyn.regs;
  Mem& mem = dyn.mem;
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

void ld(JIT& dyn, u32 instr) {
  Registers& regs = dyn.regs;
  Mem& mem = dyn.mem;
  s64 address = regs.gpr[RS(instr)] + (s16)instr;
  if (check_address_error(address, 0b111)) {
    HandleTLBException(regs, address);
    FireException(regs, ExceptionCode::AddressErrorLoad, 0, true);
    return;
  }

  s64 value = mem.Read64(regs, address);
  regs.gpr[RT(instr)] = value;
}

void lld(JIT& dyn, u32 instr) {
  Registers& regs = dyn.regs;
  Mem& mem = dyn.mem;
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

void ldl(JIT& dyn, u32 instr) {
  Registers& regs = dyn.regs;
  Mem& mem = dyn.mem;
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

void ldr(JIT& dyn, u32 instr) {
  Registers& regs = dyn.regs;
  Mem& mem = dyn.mem;
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

void lbu(JIT& dyn, u32 instr) {
  Registers& regs = dyn.regs;
  Mem& mem = dyn.mem;
  u64 address = regs.gpr[RS(instr)] + (s16)instr;
  u8 value = mem.Read8(regs, address);
  regs.gpr[RT(instr)] = value;
}

void lhu(JIT& dyn, u32 instr) {
  Registers& regs = dyn.regs;
  Mem& mem = dyn.mem;
  s64 address = regs.gpr[RS(instr)] + (s16)instr;
  if (check_address_error(address, 0b1)) {
    HandleTLBException(regs, address);
    FireException(regs, ExceptionCode::AddressErrorLoad, 0, true);
    return;
  }

  u16 value = mem.Read16(regs, address);
  regs.gpr[RT(instr)] = value;
}

void lwu(JIT& dyn, u32 instr) {
  Registers& regs = dyn.regs;
  Mem& mem = dyn.mem;
  s64 address = regs.gpr[RS(instr)] + (s16)instr;
  if (check_address_error(address, 0b11)) {
    HandleTLBException(regs, address);
    FireException(regs, ExceptionCode::AddressErrorLoad, 0, true);
    return;
  }

  u32 value = mem.Read32(regs, address);
  regs.gpr[RT(instr)] = value;
}

void sb(JIT& dyn, u32 instr) {
  Registers& regs = dyn.regs;
  Mem& mem = dyn.mem;
  u32 address = regs.gpr[RS(instr)] + (s16)instr;
  mem.Write8(regs, dyn, address, regs.gpr[RT(instr)]);
}

void sc(JIT& dyn, u32 instr) {
  Registers& regs = dyn.regs;
  Mem& mem = dyn.mem;
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
      mem.Write32(regs, dyn, paddr, regs.gpr[RT(instr)]);
      regs.gpr[RT(instr)] = 1;
    }
  } else {
    regs.gpr[RT(instr)] = 0;
  }
}

void scd(JIT& dyn, u32 instr) {
  Registers& regs = dyn.regs;
  Mem& mem = dyn.mem;
  s64 address = regs.gpr[RS(instr)] + (s16)instr;
  if (check_address_error(address, 0b111)) {
    HandleTLBException(regs, address);
    FireException(regs, ExceptionCode::AddressErrorStore, 0, true);
    return;
  }

  if(regs.cop0.llbit) {
    mem.Write64(regs, dyn, address, regs.gpr[RT(instr)]);
  }

  regs.gpr[RT(instr)] = (s64)((u64)regs.cop0.llbit);
  regs.cop0.llbit = false;
}

void sh(JIT& dyn, u32 instr) {
  Registers& regs = dyn.regs;
  Mem& mem = dyn.mem;
  s64 address = regs.gpr[RS(instr)] + (s16)instr;
  if (check_address_error(address, 0b1)) {
    HandleTLBException(regs, address);
    FireException(regs, ExceptionCode::AddressErrorStore, 0, true);
    return;
  }

  u32 physical;
  if(!MapVAddr(regs, STORE, address, physical)) {
    HandleTLBException(regs, address);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, STORE), 0, true);
  } else {
    mem.Write16(regs, dyn, physical, regs.gpr[RT(instr)]);
  }
}

void sw(JIT& dyn, u32 instr) {
  Registers& regs = dyn.regs;
  Mem& mem = dyn.mem;
  s16 offset = instr;
  u64 address = regs.gpr[RS(instr)] + offset;
  if (check_address_error(address, 0b11)) {
    HandleTLBException(regs, address);
    FireException(regs, ExceptionCode::AddressErrorStore, 0, true);
    return;
  }

  u32 physical;
  if(!MapVAddr(regs, STORE, address, physical)) {
    HandleTLBException(regs, address);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, STORE), 0, true);
  } else {
    mem.Write32(regs, dyn, physical, regs.gpr[RT(instr)]);
  }
}

void sd(JIT& dyn, u32 instr) {
  Registers& regs = dyn.regs;
  Mem& mem = dyn.mem;
  s64 address = regs.gpr[RS(instr)] + (s16)instr;
  if (check_address_error(address, 0b11)) {
    HandleTLBException(regs, address);
    FireException(regs, ExceptionCode::AddressErrorStore, 0, true);
    return;
  }

  u32 physical;
  if(!MapVAddr(regs, STORE, address, physical)) {
    HandleTLBException(regs, address);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, STORE), 0, true);
  } else {
    mem.Write64(regs, dyn, physical, regs.gpr[RT(instr)]);
  }

}

void sdl(JIT& dyn, u32 instr) {
  Registers& regs = dyn.regs;
  Mem& mem = dyn.mem;
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
    mem.Write64(regs, dyn, paddr & ~7, (data & ~mask) | (rt >> shift));
  }
}

void sdr(JIT& dyn, u32 instr) {
  Registers& regs = dyn.regs;
  Mem& mem = dyn.mem;
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
    mem.Write64(regs, dyn, paddr & ~7, (data & ~mask) | (rt << shift));
  }
}

void swl(JIT& dyn, u32 instr) {
  Registers& regs = dyn.regs;
  Mem& mem = dyn.mem;
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
    mem.Write32(regs, dyn, paddr & ~3, (data & ~mask) | (rt >> shift));
  }
}

void swr(JIT& dyn, u32 instr) {
  Registers& regs = dyn.regs;
  Mem& mem = dyn.mem;
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
    mem.Write32(regs, dyn, paddr & ~3, (data & ~mask) | (rt << shift));
  }
}

void ori(JIT& dyn, u32 instr) {
  Registers& regs = dyn.regs;
  s64 imm = (u16)instr;
  s64 result = imm | regs.gpr[RS(instr)];
  regs.gpr[RT(instr)] = result;
}

void or_(JIT& dyn, u32 instr) {
  Registers& regs = dyn.regs;
  if(likely(RD(instr) != 0)) {
    regs.gpr[RD(instr)] = regs.gpr[RS(instr)] | regs.gpr[RT(instr)];
  }
}

void nor(JIT& dyn, u32 instr) {
  Registers& regs = dyn.regs;
  if(likely(RD(instr) != 0)) {
    regs.gpr[RD(instr)] = ~(regs.gpr[RS(instr)] | regs.gpr[RT(instr)]);
  }
}

void j(JIT& dyn, u32 instr) {
  Registers& regs = dyn.regs;
  u64 target = (instr & 0x3ffffff) << 2;
  u64 address = ((regs.pc - 4) & ~0xfffffff) | target;

  branch(dyn, true, address);
}

void jal(JIT& dyn, u32 instr) {
  Registers& regs = dyn.regs;
  regs.gpr[31] = regs.nextPC;
  j(dyn, instr);
}

void jalr(JIT& dyn, u32 instr) {
  Registers& regs = dyn.regs;
  branch(dyn, true, regs.gpr[RS(instr)]);
  if(likely(RD(instr) != 0)) {
    regs.gpr[RD(instr)] = regs.pc + 4;
  }
}

void slti(JIT& dyn, u32 instr) {
  Registers& regs = dyn.regs;
  regs.gpr[RT(instr)] = regs.gpr[RS(instr)] < se_imm(instr);
}

void sltiu(JIT& dyn, u32 instr) {
  Registers& regs = dyn.regs;
  regs.gpr[RT(instr)] = (u64)regs.gpr[RS(instr)] < se_imm(instr);
}

void slt(JIT& dyn, u32 instr) {
  Registers& regs = dyn.regs;
  if(likely(RD(instr) != 0)) {
    regs.gpr[RD(instr)] = regs.gpr[RS(instr)] < regs.gpr[RT(instr)];
  }
}

void sltu(JIT& dyn, u32 instr) {
  Registers& regs = dyn.regs;
  if(likely(RD(instr) != 0)) {
    regs.gpr[RD(instr)] = (u64) regs.gpr[RS(instr)] < (u64) regs.gpr[RT(instr)];
  }
}

void xori(JIT& dyn, u32 instr) {
  Registers& regs = dyn.regs;
  s64 imm = (u16)instr;
  regs.gpr[RT(instr)] = regs.gpr[RS(instr)] ^ imm;
}

void xor_(JIT& dyn, u32 instr) {
  Registers& regs = dyn.regs;
  if(likely(RD(instr) != 0)) {
    regs.gpr[RD(instr)] = regs.gpr[RT(instr)] ^ regs.gpr[RS(instr)];
  }
}

void andi(JIT& dyn, u32 instr) {
  Registers& regs = dyn.regs;
  s64 imm = (u16)instr;
  regs.gpr[RT(instr)] = regs.gpr[RS(instr)] & imm;
}

void and_(JIT& dyn, u32 instr) {
  Registers& regs = dyn.regs;
  if(likely(RD(instr) != 0)) {
    regs.gpr[RD(instr)] = regs.gpr[RS(instr)] & regs.gpr[RT(instr)];
  }
}

void sll(JIT& dyn, u32 instr) {
  Registers& regs = dyn.regs;
  if(likely(RD(instr) != 0)) {
    u8 sa = ((instr >> 6) & 0x1f);
    s32 result = regs.gpr[RT(instr)] << sa;
    regs.gpr[RD(instr)] = (s64) result;
  }
}

void sllv(JIT& dyn, u32 instr) {
  Registers& regs = dyn.regs;
  if(likely(RD(instr) != 0)) {
    u8 sa = (regs.gpr[RS(instr)]) & 0x1F;
    u32 rt = regs.gpr[RT(instr)];
    s32 result = rt << sa;
    regs.gpr[RD(instr)] = (s64) result;
  }
}

void dsll32(JIT& dyn, u32 instr) {
  Registers& regs = dyn.regs;
  if(likely(RD(instr) != 0)) {
    u8 sa = ((instr >> 6) & 0x1f);
    s64 result = regs.gpr[RT(instr)] << (sa + 32);
    regs.gpr[RD(instr)] = result;
  }
}

void dsll(JIT& dyn, u32 instr) {
  Registers& regs = dyn.regs;
  if(likely(RD(instr) != 0)) {
    u8 sa = ((instr >> 6) & 0x1f);
    s64 result = regs.gpr[RT(instr)] << sa;
    regs.gpr[RD(instr)] = result;
  }
}

void dsllv(JIT& dyn, u32 instr) {
  Registers& regs = dyn.regs;
  if(likely(RD(instr) != 0)) {
    s64 sa = regs.gpr[RS(instr)] & 63;
    s64 result = regs.gpr[RT(instr)] << sa;
    regs.gpr[RD(instr)] = result;
  }
}

void srl(JIT& dyn, u32 instr) {
  Registers& regs = dyn.regs;
  if(likely(RD(instr) != 0)) {
    u32 rt = regs.gpr[RT(instr)];
    u8 sa = ((instr >> 6) & 0x1f);
    u32 result = rt >> sa;
    regs.gpr[RD(instr)] = (s32) result;
  }
}

void srlv(JIT& dyn, u32 instr) {
  Registers& regs = dyn.regs;
  if(likely(RD(instr) != 0)) {
    u8 sa = (regs.gpr[RS(instr)] & 0x1F);
    u32 rt = regs.gpr[RT(instr)];
    s32 result = rt >> sa;
    regs.gpr[RD(instr)] = (s64) result;
  }
}

void dsrl(JIT& dyn, u32 instr) {
  Registers& regs = dyn.regs;
  if(likely(RD(instr) != 0)) {
    u64 rt = regs.gpr[RT(instr)];
    u8 sa = ((instr >> 6) & 0x1f);
    u64 result = rt >> sa;
    regs.gpr[RD(instr)] = s64(result);
  }
}

void dsrlv(JIT& dyn, u32 instr) {
  Registers& regs = dyn.regs;
  if(likely(RD(instr) != 0)) {
    u8 amount = (regs.gpr[RS(instr)] & 63);
    u64 rt = regs.gpr[RT(instr)];
    u64 result = rt >> amount;
    regs.gpr[RD(instr)] = s64(result);
  }
}

void dsrl32(JIT& dyn, u32 instr) {
  Registers& regs = dyn.regs;
  if(likely(RD(instr) != 0)) {
    u64 rt = regs.gpr[RT(instr)];
    u8 sa = ((instr >> 6) & 0x1f);
    u64 result = rt >> (sa + 32);
    regs.gpr[RD(instr)] = s64(result);
  }
}

void sra(JIT& dyn, u32 instr) {
  Registers& regs = dyn.regs;
  if(likely(RD(instr) != 0)) {
    s64 rt = regs.gpr[RT(instr)];
    u8 sa = ((instr >> 6) & 0x1f);
    s32 result = rt >> sa;
    regs.gpr[RD(instr)] = result;
  }
}

void srav(JIT& dyn, u32 instr) {
  Registers& regs = dyn.regs;
  if(likely(RD(instr) != 0)) {
    s64 rt = regs.gpr[RT(instr)];
    s64 rs = regs.gpr[RS(instr)];
    u8 sa = rs & 0x1f;
    s32 result = rt >> sa;
    regs.gpr[RD(instr)] = result;
  }
}

void dsra(JIT& dyn, u32 instr) {
  Registers& regs = dyn.regs;
  if(likely(RD(instr) != 0)) {
    s64 rt = regs.gpr[RT(instr)];
    u8 sa = ((instr >> 6) & 0x1f);
    s64 result = rt >> sa;
    regs.gpr[RD(instr)] = result;
  }
}

void dsrav(JIT& dyn, u32 instr) {
  Registers& regs = dyn.regs;
  if(likely(RD(instr) != 0)) {
    s64 rt = regs.gpr[RT(instr)];
    s64 rs = regs.gpr[RS(instr)];
    s64 sa = rs & 63;
    s64 result = rt >> sa;
    regs.gpr[RD(instr)] = result;
  }
}

void dsra32(JIT& dyn, u32 instr) {
  Registers& regs = dyn.regs;
  if(likely(RD(instr) != 0)) {
    s64 rt = regs.gpr[RT(instr)];
    u8 sa = ((instr >> 6) & 0x1f);
    s64 result = rt >> (sa + 32);
    regs.gpr[RD(instr)] = result;
  }
}

void jr(JIT& dyn, u32 instr) {
  Registers& regs = dyn.regs;
  s64 address = regs.gpr[RS(instr)];
  if (check_address_error(address, 0b11)) {
    HandleTLBException(regs, address);
    FireException(regs, ExceptionCode::DataBusError, 0, true);
  }

  branch(dyn, true, address);
}

void dsub(JIT& dyn, u32 instr) {
  Registers& regs = dyn.regs;
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

void dsubu(JIT& dyn, u32 instr) {
  Registers& regs = dyn.regs;
  if(likely(RD(instr) != 0)) {
    u64 rt = regs.gpr[RT(instr)];
    u64 rs = regs.gpr[RS(instr)];
    u64 result = rs - rt;
    regs.gpr[RD(instr)] = s64(result);
  }
}

void sub(JIT& dyn, u32 instr) {
  Registers& regs = dyn.regs;
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

void subu(JIT& dyn, u32 instr) {
  Registers& regs = dyn.regs;
  if(likely(RD(instr) != 0)) {
    u32 rt = regs.gpr[RT(instr)];
    u32 rs = regs.gpr[RS(instr)];
    u32 result = rs - rt;
    regs.gpr[RD(instr)] = (s64) ((s32) result);
  }
}

void dmultu(JIT& dyn, u32 instr) {
  Registers& regs = dyn.regs;
  u64 rt = regs.gpr[RT(instr)];
  u64 rs = regs.gpr[RS(instr)];
  u128 result = (u128)rt * (u128)rs;
  regs.lo = (s64)(result & 0xFFFFFFFFFFFFFFFF);
  regs.hi = (s64)(result >> 64);
}

void dmult(JIT& dyn, u32 instr) {
  Registers& regs = dyn.regs;
  s64 rt = regs.gpr[RT(instr)];
  s64 rs = regs.gpr[RS(instr)];
  s128 result = (s128)rt * (s128)rs;
  regs.lo = result & 0xFFFFFFFFFFFFFFFF;
  regs.hi = result >> 64;
}

void multu(JIT& dyn, u32 instr) {
  Registers& regs = dyn.regs;
  u32 rt = regs.gpr[RT(instr)];
  u32 rs = regs.gpr[RS(instr)];
  u64 result = (u64)rt * (u64)rs;
  regs.lo = (s64)((s32)result);
  regs.hi = (s64)((s32)(result >> 32));
}

void mult(JIT& dyn, u32 instr) {
  Registers& regs = dyn.regs;
  s32 rt = regs.gpr[RT(instr)];
  s32 rs = regs.gpr[RS(instr)];
  s64 result = (s64)rt * (s64)rs;
  regs.lo = (s64)((s32)result);
  regs.hi = (s64)((s32)(result >> 32));
}

void mflo(JIT& dyn, u32 instr) {
  Registers& regs = dyn.regs;
  if(likely(RD(instr) != 0)) {
    regs.gpr[RD(instr)] = regs.lo;
  }
}

void mfhi(JIT& dyn, u32 instr) {
  Registers& regs = dyn.regs;
  if(likely(RD(instr) != 0)) {
    regs.gpr[RD(instr)] = regs.hi;
  }
}

void mtlo(JIT& dyn, u32 instr) {
  Registers& regs = dyn.regs;
  regs.lo = regs.gpr[RS(instr)];
}

void mthi(JIT& dyn, u32 instr) {
  Registers& regs = dyn.regs;
  regs.hi = regs.gpr[RS(instr)];
}

void trap(JIT& dyn, bool cond) {
  Registers& regs = dyn.regs;
  if(cond) {
    FireException(regs, ExceptionCode::Trap, 0, true);
  }
}

void mtc2(JIT& dyn, u32 instr) {
  Registers& regs = dyn.regs;
  dyn.cop2Latch = regs.gpr[RT(instr)];
}

void mfc2(JIT& dyn, u32 instr) {
  Registers& regs = dyn.regs;
  s32 value = dyn.cop2Latch;
  regs.gpr[RT(instr)] = value;
}

void dmtc2(JIT& dyn, u32 instr) {
  Registers& regs = dyn.regs;
  dyn.cop2Latch = regs.gpr[RT(instr)];
}

void dmfc2(JIT& dyn, u32 instr) {
  Registers& regs = dyn.regs;
  regs.gpr[RT(instr)] = dyn.cop2Latch;
}

void ctc2(u32) {

}

void cfc2(u32) {

}

}