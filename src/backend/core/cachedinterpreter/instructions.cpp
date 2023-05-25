#include <core/CachedInterpreter.hpp>

#define check_address_error(mask, vaddr) (((!regs.cop0.is_64bit_addressing) && (s32)(vaddr) != (vaddr)) || (((vaddr) & (mask)) != 0))
#define check_signed_overflow(op1, op2, res) (((~((op1) ^ (op2)) & ((op1) ^ (res))) >> ((sizeof(res) * 8) - 1)) & 1)
#define check_signed_underflow(op1, op2, res) (((((op1) ^ (op2)) & ((op1) ^ (res))) >> ((sizeof(res) * 8) - 1)) & 1)

namespace n64 {
void CachedInterpreter::add(u32 instr) {
  u32 rs = (s32)regs.gpr[RS(instr)];
  u32 rt = (s32)regs.gpr[RT(instr)];
  u32 result = rs + rt;
  Util::trace("add r{}, r{}, r{} = {:08X}", RS(instr), RT(instr), RD(instr), result);
  if(check_signed_overflow(rs, rt, result)) {
    FireException(regs, ExceptionCode::Overflow, 0, true);
  } else {
    if(RD(instr) != 0) [[likely]] {
      regs.gpr[RD(instr)] = s32(result);
    }
  }
}

void CachedInterpreter::addu(u32 instr) {
  if(RD(instr) != 0) [[likely]] {
    s32 rs = (s32)regs.gpr[RS(instr)];
    s32 rt = (s32)regs.gpr[RT(instr)];
    s32 result = rs + rt;
    regs.gpr[RD(instr)] = result;
    Util::trace("addu r{}, r{}, r{} = {:08X}", RS(instr), RT(instr), RD(instr), (u32)result);
  }
}

void CachedInterpreter::addi(u32 instr) {
  u32 rs = regs.gpr[RS(instr)];
  u32 imm = s32(s16(instr));
  u32 result = rs + imm;
  Util::trace("addi r{}, r{}, {:08X} = {:08X}", RT(instr), RS(instr), imm, (u32)result);
  if(check_signed_overflow(rs, imm, result)) {
    FireException(regs, ExceptionCode::Overflow, 0, true);
  } else {
    regs.gpr[RT(instr)] = s32(result);
  }
}

void CachedInterpreter::addiu(u32 instr) {
  s32 rs = (s32)regs.gpr[RS(instr)];
  s32 imm = s16(instr);
  s32 result = rs + imm;
  regs.gpr[RT(instr)] = result;
  Util::trace("addiu r{}, r{}, {:08X} = {:08X}", RT(instr), RS(instr), (u32)imm, (u32)result);
}

void CachedInterpreter::dadd(u32 instr) {
  u64 rs = regs.gpr[RS(instr)];
  u64 rt = regs.gpr[RT(instr)];
  u64 result = rt + rs;
  Util::trace("dadd r{}, r{}, r{} = {:016X}", RD(instr), RS(instr), RT(instr), result);
  if(check_signed_overflow(rs, rt, result)) {
    FireException(regs, ExceptionCode::Overflow, 0, true);
  } else {
    if(RD(instr) != 0) [[likely]] {
      regs.gpr[RD(instr)] = result;
    }
  }
}

void CachedInterpreter::daddu(u32 instr) {
  if(RD(instr) != 0) [[likely]] {
    s64 rs = regs.gpr[RS(instr)];
    s64 rt = regs.gpr[RT(instr)];
    u64 result = rs + rt;
    regs.gpr[RD(instr)] = result;
    Util::trace("daddu r{}, r{}, r{} = {:016X}", RD(instr), RS(instr), RT(instr), result);
  }
}

void CachedInterpreter::daddi(u32 instr) {
  u64 imm = s64(s16(instr));
  u64 rs = regs.gpr[RS(instr)];
  u64 result = imm + rs;
  Util::trace("daddi r{}, r{}, {:016X} = {:016X}", RT(instr), RS(instr), imm, result);
  if(check_signed_overflow(rs, imm, result)) {
    FireException(regs, ExceptionCode::Overflow, 0, true);
  } else {
    regs.gpr[RT(instr)] = result;
  }
}

void CachedInterpreter::daddiu(u32 instr) {
  s64 imm = s16(instr);
  s64 rs = regs.gpr[RS(instr)];
  u64 result = rs + imm;
  regs.gpr[RT(instr)] = result;
  Util::trace("daddiu r{}, r{}, {:016X} = {:016X}", RT(instr), RS(instr), imm, result);
}

void CachedInterpreter::div(u32 instr) {
  s64 dividend = (s32)regs.gpr[RS(instr)];
  s64 divisor = (s32)regs.gpr[RT(instr)];

  s32 quotient, remainder;

  if(divisor == 0) {
    remainder = dividend;
    if(dividend >= 0) {
      quotient = s64(-1);
    } else {
      quotient = s64(1);
    }
  } else {
    quotient = dividend / divisor;
    remainder = dividend % divisor;
  }

  regs.lo = quotient;
  regs.hi = remainder;

  Util::trace("div r{}, r{} = q:{:08X}, r:{:08X}", RS(instr), RT(instr), (u32)quotient, (u32)remainder);
}

void CachedInterpreter::divu(u32 instr) {
  u32 dividend = regs.gpr[RS(instr)];
  u32 divisor = regs.gpr[RT(instr)];

  s32 quotient, remainder;

  if(divisor == 0) {
    quotient = -1;
    remainder = dividend;
  } else {
    quotient = (s32)(dividend / divisor);
    remainder = (s32)(dividend % divisor);
  }

  regs.lo = quotient;
  regs.hi = remainder;

  Util::trace("div r{}, r{} = q:{:08X}, r:{:08X}", RS(instr), RT(instr), (u32)quotient, (u32)remainder);
}

void CachedInterpreter::ddiv(u32 instr) {
  s64 dividend = regs.gpr[RS(instr)];
  s64 divisor = regs.gpr[RT(instr)];
  s64 quotient, remainder;

  if (dividend == 0x8000000000000000 && divisor == 0xFFFFFFFFFFFFFFFF) {
    quotient = dividend;
    remainder = 0;
    regs.lo = quotient;
    regs.hi = remainder;
  } else if(divisor == 0) {
    remainder = dividend;
    regs.hi = remainder;
    if(dividend >= 0) {
      quotient = -1;
      regs.lo = quotient;
    } else {
      quotient = 1;
      regs.lo = quotient;
    }
  } else {
    quotient = dividend / divisor;
    remainder = dividend % divisor;
    regs.lo = quotient;
    regs.hi = remainder;
  }

  Util::trace("ddiv r{}, r{} = q:{:016X}, r:{:016X}", RS(instr), RT(instr), (u64)quotient, (u64)remainder);
}

void CachedInterpreter::ddivu(u32 instr) {
  u64 dividend = regs.gpr[RS(instr)];
  u64 divisor = regs.gpr[RT(instr)];
  u64 quotient, remainder;
  if(divisor == 0) {
    quotient = -1;
    remainder = dividend;
    regs.lo = s64(quotient);
    regs.hi = s64(remainder);
  } else {
    quotient = dividend / divisor;
    remainder = dividend % divisor;
    regs.lo = s64(quotient);
    regs.hi = s64(remainder);
  }

  Util::trace("ddivu r{}, r{} = q:{:016X}, r:{:016X}", RS(instr), RT(instr), quotient, remainder);
}

void CachedInterpreter::branch(bool cond, s64 address) {
  regs.delaySlot = true;
  if (cond) {
    regs.nextPC = address;
    Util::trace("b {:016X}", (u64)address);
  }
}

void CachedInterpreter::branch_likely(bool cond, s64 address) {
  if (cond) {
    regs.delaySlot = true;
    regs.nextPC = address;
    Util::trace("bl {:016X}", (u64)address);
  } else {
    regs.SetPC64(regs.nextPC);
  }
}

void CachedInterpreter::b(u32 instr, bool cond) {
  s16 imm = instr;
  s64 offset = (s64)imm << 2;
  s64 address = regs.pc + offset;
  branch(cond, address);
}

void CachedInterpreter::blink(u32 instr, bool cond) {
  regs.gpr[31] = regs.nextPC;
  s16 imm = instr;
  s64 offset = (s64)imm << 2;
  s64 address = regs.pc + offset;
  branch(cond, address);
}

void CachedInterpreter::bl(u32 instr, bool cond) {
  s16 imm = instr;
  s64 offset = (s64)imm << 2;
  s64 address = regs.pc + offset;
  branch_likely(cond, address);
}

void CachedInterpreter::bllink(u32 instr, bool cond) {
  regs.gpr[31] = regs.nextPC;
  s16 imm = instr;
  s64 offset = (s64)imm << 2;
  s64 address = regs.pc + offset;
  branch_likely(cond, address);
}

void CachedInterpreter::lui(u32 instr) {
  s64 val = (s16)instr;
  val <<= 16;
  regs.gpr[RT(instr)] = val;
  Util::trace("lui r{}, {:016X}", RT(instr), (u64)val);
}

void CachedInterpreter::lb(u32 instr) {
  u64 address = regs.gpr[RS(instr)] + (s16)instr;
  u32 paddr = 0;
  if(!MapVAddr(regs, LOAD, address, paddr)) {
    HandleTLBException(regs, address);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, LOAD), 0, true);
  } else {
    regs.gpr[RT(instr)] = (s8)mem.Read8(regs, paddr);
    Util::trace("lb r{}, [{:08X}]", RT(instr), paddr);
  }
}

void CachedInterpreter::lh(u32 instr) {
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
    Util::trace("lh r{}, [{:08X}]", RT(instr), paddr);
  }
}

void CachedInterpreter::lw(u32 instr) {
  s16 offset = instr;
  u64 address = regs.gpr[RS(instr)] + offset;
  if (check_address_error(0b11, address)) {
    HandleTLBException(regs, address);
    FireException(regs, ExceptionCode::AddressErrorLoad, 0, true);
    return;
  }

  u32 paddr = 0;
  if (!MapVAddr(regs, LOAD, address, paddr)) {
    HandleTLBException(regs, address);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, LOAD), 0, true);
  } else {
    regs.gpr[RT(instr)] = (s32)mem.Read32(regs, paddr);
    Util::trace("lw r{}, [{:08X}]", RT(instr), paddr);
  }
}

void CachedInterpreter::ll(u32 instr) {
  u64 address = regs.gpr[RS(instr)] + (s16)instr;
  u32 paddr;
  if (!MapVAddr(regs, LOAD, address, paddr)) {
    HandleTLBException(regs, address);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, LOAD), 0, true);
  } else {
    if ((address & 0b11) > 0) {
      FireException(regs, ExceptionCode::AddressErrorLoad, 0, true);
      return;
    } else {
      regs.gpr[RT(instr)] = (s32)mem.Read32(regs, paddr);
      Util::trace("lw r{}, [{:08X}]", RT(instr), paddr);
    }
  }

  regs.cop0.llbit = true;
  regs.cop0.LLAddr = paddr >> 4;
}

void CachedInterpreter::lwl(u32 instr) {
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
    Util::trace("lwl r{}, [{:08X}]", RT(instr), paddr);
  }
}

void CachedInterpreter::lwr(u32 instr) {
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
    Util::trace("lwr r{}, [{:08X}]", RT(instr), paddr);
  }
}

void CachedInterpreter::ld(u32 instr) {
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
    Util::trace("ld r{}, [{:08X}]", RT(instr), paddr);
  }
}

void CachedInterpreter::lld(u32 instr) {
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
      Util::trace("lld r{}, [{:08X}]", RT(instr), paddr);
      regs.cop0.llbit = true;
      regs.cop0.LLAddr = paddr >> 4;
    }
  }
}

void CachedInterpreter::ldl(u32 instr) {
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
    Util::trace("ldl r{}, [{:08X}]", RT(instr), paddr);
  }
}

void CachedInterpreter::ldr(u32 instr) {
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
    Util::trace("ldr r{}, [{:08X}]", RT(instr), paddr);
  }
}

void CachedInterpreter::lbu(u32 instr) {
  u64 address = regs.gpr[RS(instr)] + (s16)instr;
  u32 paddr;
  if (!MapVAddr(regs, LOAD, address, paddr)) {
    HandleTLBException(regs, address);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, LOAD), 0, true);
  } else {
    u8 value = mem.Read8(regs, paddr);
    regs.gpr[RT(instr)] = value;
    Util::trace("lbu r{}, [{:08X}]", RT(instr), paddr);
  }
}

void CachedInterpreter::lhu(u32 instr) {
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
    Util::trace("lhu r{}, [{:08X}]", RT(instr), paddr);
  }
}

void CachedInterpreter::lwu(u32 instr) {
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
    Util::trace("lwu r{}, [{:08X}]", RT(instr), paddr);
  }
}

void CachedInterpreter::sb(u32 instr) {
  u64 address = regs.gpr[RS(instr)] + (s16)instr;
  u32 paddr;
  if (!MapVAddr(regs, STORE, address, paddr)) {
    HandleTLBException(regs, address);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, STORE), 0, true);
  } else {
    mem.Write8(regs, paddr, regs.gpr[RT(instr)]);
    Util::trace("sb r{}, [{:08X}]", RT(instr), paddr);
  }
}

void CachedInterpreter::sc(u32 instr) {
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
      Util::trace("sc r{}, [{:08X}]", RT(instr), paddr);
      regs.gpr[RT(instr)] = 1;
    }
  } else {
    regs.gpr[RT(instr)] = 0;
  }
}

void CachedInterpreter::scd(u32 instr) {
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
      Util::trace("scd r{}, [{:08X}]", RT(instr), paddr);
      regs.gpr[RT(instr)] = 1;
    }
  } else {
    regs.gpr[RT(instr)] = 0;
  }
}

void CachedInterpreter::sh(u32 instr) {
  s64 address = regs.gpr[RS(instr)] + (s16)instr;

  u32 paddr;
  if(!MapVAddr(regs, STORE, address, paddr)) {
    HandleTLBException(regs, address);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, STORE), 0, true);
  } else {
    mem.Write16(regs, paddr, regs.gpr[RT(instr)]);
    Util::trace("sh r{}, [{:08X}]", RT(instr), paddr);
  }
}

void CachedInterpreter::sw(u32 instr) {
  s16 offset = instr;
  u64 address = regs.gpr[RS(instr)] + offset;
  if (check_address_error(0b11, address)) {
    HandleTLBException(regs, address);
    FireException(regs, ExceptionCode::AddressErrorStore, 0, true);
    return;
  }

  u32 paddr;
  if(!MapVAddr(regs, STORE, address, paddr)) {
    HandleTLBException(regs, address);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, STORE), 0, true);
  } else {
    mem.Write32(regs, paddr, regs.gpr[RT(instr)]);
    Util::trace("sw r{}, [{:08X}]", RT(instr), paddr);
  }
}

void CachedInterpreter::sd(u32 instr) {
  s64 address = regs.gpr[RS(instr)] + (s16)instr;
  if (check_address_error(0b111, address)) {
    HandleTLBException(regs, address);
    FireException(regs, ExceptionCode::AddressErrorStore, 0, true);
    return;
  }

  u32 paddr;
  if(!MapVAddr(regs, STORE, address, paddr)) {
    HandleTLBException(regs, address);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, STORE), 0, true);
  } else {
    mem.Write64(regs, paddr, regs.gpr[RT(instr)]);
    Util::trace("sd r{}, [{:08X}]", RT(instr), paddr);
  }
}

void CachedInterpreter::sdl(u32 instr) {
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
    Util::trace("sdl r{}, [{:08X}]", RT(instr), paddr);
  }
}

void CachedInterpreter::sdr(u32 instr) {
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
    Util::trace("sdr r{}, [{:08X}]", RT(instr), paddr);
  }
}

void CachedInterpreter::swl(u32 instr) {
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
    Util::trace("swl r{}, [{:08X}]", RT(instr), paddr);
  }
}

void CachedInterpreter::swr(u32 instr) {
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
    Util::trace("swr r{}, [{:08X}]", RT(instr), paddr);
  }
}

void CachedInterpreter::ori(u32 instr) {
  s64 imm = (u16)instr;
  s64 result = imm | regs.gpr[RS(instr)];
  regs.gpr[RT(instr)] = result;
  Util::trace("ori r{}, r{}, {:016X}", RT(instr), RS(instr), imm);
}

void CachedInterpreter::or_(u32 instr) {
  if (RD(instr) != 0) [[likely]] {
    regs.gpr[RD(instr)] = regs.gpr[RS(instr)] | regs.gpr[RT(instr)];
    Util::trace("or r{}, r{}, r{}", RD(instr), RS(instr), RT(instr));
  }
}

void CachedInterpreter::nor(u32 instr) {
  if(RD(instr) != 0) [[likely]] {
    regs.gpr[RD(instr)] = ~(regs.gpr[RS(instr)] | regs.gpr[RT(instr)]);
    Util::trace("nor r{}, r{}, r{}", RD(instr), RS(instr), RT(instr));
  }
}

void CachedInterpreter::j(u32 instr) {
  s32 target = (instr & 0x3ffffff) << 2;
  s64 address = (regs.oldPC & ~0xfffffff) | target;

  branch(true, address);
}

void CachedInterpreter::jal(u32 instr) {
  regs.gpr[31] = regs.nextPC;
  j(instr);
}

void CachedInterpreter::jalr(u32 instr) {
  branch(true, regs.gpr[RS(instr)]);
  if(RD(instr) != 0) [[likely]] {
    regs.gpr[RD(instr)] = regs.pc + 4;
  }
}

void CachedInterpreter::slti(u32 instr) {
  s16 imm = instr;
  regs.gpr[RT(instr)] = regs.gpr[RS(instr)] < imm;
  Util::trace("slti r{}, r{}, {:04X}", RT(instr), RS(instr), u16(imm));
}

void CachedInterpreter::sltiu(u32 instr) {
  s16 imm = instr;
  regs.gpr[RT(instr)] = (u64)regs.gpr[RS(instr)] < imm;
  Util::trace("sltiu r{}, r{}, {:04X}", RT(instr), RS(instr), u16(imm));
}

void CachedInterpreter::slt(u32 instr) {
  if(RD(instr) != 0) [[likely]] {
    regs.gpr[RD(instr)] = regs.gpr[RS(instr)] < regs.gpr[RT(instr)];
    Util::trace("slt r{}, r{}, r{}", RD(instr), RS(instr), RS(instr));
  }
}

void CachedInterpreter::sltu(u32 instr) {
  if(RD(instr) != 0) [[likely]] {
    regs.gpr[RD(instr)] = (u64) regs.gpr[RS(instr)] < (u64) regs.gpr[RT(instr)];
    Util::trace("sltu r{}, r{}, r{}", RD(instr), RS(instr), RS(instr));
  }
}

void CachedInterpreter::xori(u32 instr) {
  s64 imm = (u16)instr;
  regs.gpr[RT(instr)] = regs.gpr[RS(instr)] ^ imm;
  Util::trace("xori r{}, r{}, {:04X}", RT(instr), RS(instr), (u16)imm);
}

void CachedInterpreter::xor_(u32 instr) {
  if(RD(instr) != 0) [[likely]] {
    regs.gpr[RD(instr)] = regs.gpr[RT(instr)] ^ regs.gpr[RS(instr)];
    Util::trace("xor r{}, r{}, r{}", RD(instr), RS(instr), RS(instr));
  }
}

void CachedInterpreter::andi(u32 instr) {
  s64 imm = (u16)instr;
  regs.gpr[RT(instr)] = regs.gpr[RS(instr)] & imm;
  Util::trace("andi r{}, r{}, {:04X}", RT(instr), RS(instr), (u16)imm);
}

void CachedInterpreter::and_(u32 instr) {
  if(RD(instr) != 0) [[likely]] {
    regs.gpr[RD(instr)] = regs.gpr[RS(instr)] & regs.gpr[RT(instr)];
    Util::trace("and r{}, r{}, r{}", RD(instr), RS(instr), RS(instr));
  }
}

void CachedInterpreter::sll(u32 instr) {
  if(RD(instr) != 0) [[likely]] {
    u8 sa = ((instr >> 6) & 0x1f);
    s32 result = regs.gpr[RT(instr)] << sa;
    regs.gpr[RD(instr)] = (s64) result;
    Util::trace("sll r{}, r{}, {:02X}", RD(instr), RT(instr), sa);
  }
}

void CachedInterpreter::sllv(u32 instr) {
  if(RD(instr) != 0) [[likely]] {
    u8 sa = (regs.gpr[RS(instr)]) & 0x1F;
    u32 rt = regs.gpr[RT(instr)];
    s32 result = rt << sa;
    regs.gpr[RD(instr)] = (s64) result;
    Util::trace("sllv r{}, r{}, r{}", RD(instr), RT(instr), RS(instr));
  }
}

void CachedInterpreter::dsll32(u32 instr) {
  if(RD(instr) != 0) [[likely]] {
    u8 sa = ((instr >> 6) & 0x1f);
    s64 result = regs.gpr[RT(instr)] << (sa + 32);
    regs.gpr[RD(instr)] = result;
    Util::trace("dsll32 r{}, r{}, {:02X}", RD(instr), RT(instr), sa);
  }
}

void CachedInterpreter::dsll(u32 instr) {
  if(RD(instr) != 0) [[likely]] {
    u8 sa = ((instr >> 6) & 0x1f);
    s64 result = regs.gpr[RT(instr)] << sa;
    regs.gpr[RD(instr)] = result;
    Util::trace("dsll r{}, r{}, {:02X}", RD(instr), RT(instr), sa);
  }
}

void CachedInterpreter::dsllv(u32 instr) {
  if(RD(instr) != 0) [[likely]] {
    s64 sa = regs.gpr[RS(instr)] & 63;
    s64 result = regs.gpr[RT(instr)] << sa;
    regs.gpr[RD(instr)] = result;
    Util::trace("dsllv r{}, r{}, r{}", RD(instr), RT(instr), RS(instr));
  }
}

void CachedInterpreter::srl(u32 instr) {
  if(RD(instr) != 0) [[likely]] {
    u32 rt = regs.gpr[RT(instr)];
    u8 sa = ((instr >> 6) & 0x1f);
    u32 result = rt >> sa;
    regs.gpr[RD(instr)] = (s32) result;
    Util::trace("srl r{}, r{}, {:02X}", RD(instr), RT(instr), sa);
  }
}

void CachedInterpreter::srlv(u32 instr) {
  if(RD(instr) != 0) [[likely]] {
    u8 sa = (regs.gpr[RS(instr)] & 0x1F);
    u32 rt = regs.gpr[RT(instr)];
    s32 result = rt >> sa;
    regs.gpr[RD(instr)] = (s64) result;
    Util::trace("srlv r{}, r{}, r{}", RD(instr), RT(instr), RS(instr));
  }
}

void CachedInterpreter::dsrl(u32 instr) {
  if(RD(instr) != 0) [[likely]] {
    u64 rt = regs.gpr[RT(instr)];
    u8 sa = ((instr >> 6) & 0x1f);
    u64 result = rt >> sa;
    regs.gpr[RD(instr)] = s64(result);
    Util::trace("dsrl r{}, r{}, {:02X}", RD(instr), RT(instr), sa);
  }
}

void CachedInterpreter::dsrlv(u32 instr) {
  if(RD(instr) != 0) [[likely]] {
    u8 amount = (regs.gpr[RS(instr)] & 63);
    u64 rt = regs.gpr[RT(instr)];
    u64 result = rt >> amount;
    regs.gpr[RD(instr)] = s64(result);
    Util::trace("dsrlv r{}, r{}, r{}", RD(instr), RT(instr), RS(instr));
  }
}

void CachedInterpreter::dsrl32(u32 instr) {
  if(RD(instr) != 0) [[likely]] {
    u64 rt = regs.gpr[RT(instr)];
    u8 sa = ((instr >> 6) & 0x1f);
    u64 result = rt >> (sa + 32);
    regs.gpr[RD(instr)] = s64(result);
    Util::trace("dsrl32 r{}, r{}, {:02X}", RD(instr), RT(instr), sa);
  }
}

void CachedInterpreter::sra(u32 instr) {
  if(RD(instr) != 0) [[likely]] {
    s64 rt = regs.gpr[RT(instr)];
    u8 sa = ((instr >> 6) & 0x1f);
    s32 result = rt >> sa;
    regs.gpr[RD(instr)] = result;
    Util::trace("sra r{}, r{}, {:02X}", RD(instr), RT(instr), sa);
  }
}

void CachedInterpreter::srav(u32 instr) {
  if(RD(instr) != 0) [[likely]] {
    s64 rt = regs.gpr[RT(instr)];
    s64 rs = regs.gpr[RS(instr)];
    u8 sa = rs & 0x1f;
    s32 result = rt >> sa;
    regs.gpr[RD(instr)] = result;
    Util::trace("srav r{}, r{}, r{}", RD(instr), RT(instr), RS(instr));
  }
}

void CachedInterpreter::dsra(u32 instr) {
  if(RD(instr) != 0) [[likely]] {
    s64 rt = regs.gpr[RT(instr)];
    u8 sa = ((instr >> 6) & 0x1f);
    s64 result = rt >> sa;
    regs.gpr[RD(instr)] = result;
    Util::trace("dsra r{}, r{}, {:02X}", RD(instr), RT(instr), sa);
  }
}

void CachedInterpreter::dsrav(u32 instr) {
  if(RD(instr) != 0) [[likely]] {
    s64 rt = regs.gpr[RT(instr)];
    s64 rs = regs.gpr[RS(instr)];
    s64 sa = rs & 63;
    s64 result = rt >> sa;
    regs.gpr[RD(instr)] = result;
    Util::trace("dsrav r{}, r{}, r{}", RD(instr), RT(instr), RS(instr));
  }
}

void CachedInterpreter::dsra32(u32 instr) {
  if(RD(instr) != 0) [[likely]] {
    s64 rt = regs.gpr[RT(instr)];
    u8 sa = ((instr >> 6) & 0x1f);
    s64 result = rt >> (sa + 32);
    regs.gpr[RD(instr)] = result;
    Util::trace("dsra32 r{}, r{}, {:02X}", RD(instr), RT(instr), sa);
  }
}

void CachedInterpreter::jr(u32 instr) {
  s64 address = regs.gpr[RS(instr)];
  branch(true, address);
}

void CachedInterpreter::dsub(u32 instr) {
  s64 rt = regs.gpr[RT(instr)];
  s64 rs = regs.gpr[RS(instr)];
  s64 result = rs - rt;
  if(check_signed_underflow(rs, rt, result)) {
    FireException(regs, ExceptionCode::Overflow, 0, true);
  } else {
    if(RD(instr) != 0) [[likely]] {
      regs.gpr[RD(instr)] = result;
      Util::trace("dsub r{}, r{}, r{}", RD(instr), RT(instr), RS(instr));
    }
  }
}

void CachedInterpreter::dsubu(u32 instr) {
  if(RD(instr) != 0) [[likely]] {
    u64 rt = regs.gpr[RT(instr)];
    u64 rs = regs.gpr[RS(instr)];
    u64 result = rs - rt;
    regs.gpr[RD(instr)] = s64(result);
    Util::trace("dsubu r{}, r{}, r{}", RD(instr), RT(instr), RS(instr));
  }
}

void CachedInterpreter::sub(u32 instr) {
  s32 rt = regs.gpr[RT(instr)];
  s32 rs = regs.gpr[RS(instr)];
  s32 result = rs - rt;
  if(check_signed_underflow(rs, rt, result)) {
    FireException(regs, ExceptionCode::Overflow, 0, true);
  } else {
    if(RD(instr) != 0) [[likely]] {
      regs.gpr[RD(instr)] = result;
      Util::trace("sub r{}, r{}, r{}", RD(instr), RT(instr), RS(instr));
    }
  }
}

void CachedInterpreter::subu(u32 instr) {
  if(RD(instr) != 0) [[likely]] {
    u32 rt = regs.gpr[RT(instr)];
    u32 rs = regs.gpr[RS(instr)];
    u32 result = rs - rt;
    regs.gpr[RD(instr)] = (s64) ((s32) result);
    Util::trace("subu r{}, r{}, r{}", RD(instr), RT(instr), RS(instr));
  }
}

void CachedInterpreter::dmultu(u32 instr) {
  u64 rt = regs.gpr[RT(instr)];
  u64 rs = regs.gpr[RS(instr)];
  u128 result = (u128)rt * (u128)rs;
  regs.lo = (s64)(result & 0xFFFFFFFFFFFFFFFF);
  regs.hi = (s64)(result >> 64);
  Util::trace("dmultu r{}, r{}", RT(instr), RS(instr));
}

void CachedInterpreter::dmult(u32 instr) {
  s64 rt = regs.gpr[RT(instr)];
  s64 rs = regs.gpr[RS(instr)];
  s128 result = (s128)rt * (s128)rs;
  regs.lo = result & 0xFFFFFFFFFFFFFFFF;
  regs.hi = result >> 64;
  Util::trace("dmult r{}, r{}", RT(instr), RS(instr));
}

void CachedInterpreter::multu(u32 instr) {
  u32 rt = regs.gpr[RT(instr)];
  u32 rs = regs.gpr[RS(instr)];
  u64 result = (u64)rt * (u64)rs;
  regs.lo = (s64)((s32)result);
  regs.hi = (s64)((s32)(result >> 32));
  Util::trace("multu r{}, r{}", RT(instr), RS(instr));
}

void CachedInterpreter::mult(u32 instr) {
  s32 rt = regs.gpr[RT(instr)];
  s32 rs = regs.gpr[RS(instr)];
  s64 result = (s64)rt * (s64)rs;
  regs.lo = (s64)((s32)result);
  regs.hi = (s64)((s32)(result >> 32));
  Util::trace("mult r{}, r{}", RT(instr), RS(instr));
}

void CachedInterpreter::mflo(u32 instr) {
  if(RD(instr) != 0) [[likely]] {
    regs.gpr[RD(instr)] = regs.lo;
    Util::trace("mflo r{}", RD(instr));
  }
}

void CachedInterpreter::mfhi(u32 instr) {
  if(RD(instr) != 0) [[likely]] {
    regs.gpr[RD(instr)] = regs.hi;
    Util::trace("mfhi r{}", RD(instr));
  }
}

void CachedInterpreter::mtlo(u32 instr) {
  regs.lo = regs.gpr[RS(instr)];
  Util::trace("mtlo r{}", RS(instr));
}

void CachedInterpreter::mthi(u32 instr) {
  regs.hi = regs.gpr[RS(instr)];
  Util::trace("mthi r{}", RS(instr));
}

void CachedInterpreter::trap(bool cond) {
  if(cond) {
    FireException(regs, ExceptionCode::Trap, 0, true);
    Util::trace("trap");
  }
}

void CachedInterpreter::mtc2(u32 instr) {
  cop2Latch = regs.gpr[RT(instr)];
}

void CachedInterpreter::mfc2(u32 instr) {
  s32 value = cop2Latch;
  regs.gpr[RT(instr)] = value;
}

void CachedInterpreter::dmtc2(u32 instr) {
  cop2Latch = regs.gpr[RT(instr)];
}

void CachedInterpreter::dmfc2(u32 instr) {
  regs.gpr[RT(instr)] = cop2Latch;
}

void CachedInterpreter::ctc2(u32) {

}

void CachedInterpreter::cfc2(u32) {

}

}