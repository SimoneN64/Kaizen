#include <core/CachedInterpreter.hpp>

#define check_address_error(mask, vaddr) (((!regs.cop0.is_64bit_addressing) && (s32)(vaddr) != (vaddr)) || (((vaddr) & (mask)) != 0))
#define check_signed_overflow(op1, op2, res) (((~((op1) ^ (op2)) & ((op1) ^ (res))) >> ((sizeof(res) * 8) - 1)) & 1)
#define check_signed_underflow(op1, op2, res) (((((op1) ^ (op2)) & ((op1) ^ (res))) >> ((sizeof(res) * 8) - 1)) & 1)

namespace n64 {
void add(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
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

void addu(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  if(RD(instr) != 0) [[likely]] {
    s32 rs = (s32)regs.gpr[RS(instr)];
    s32 rt = (s32)regs.gpr[RT(instr)];
    s32 result = rs + rt;
    regs.gpr[RD(instr)] = result;
    Util::trace("addu r{}, r{}, r{} = {:08X}", RS(instr), RT(instr), RD(instr), (u32)result);
  }
}

void addi(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
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

void addiu(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  s32 rs = (s32)regs.gpr[RS(instr)];
  s32 imm = s16(instr);
  s32 result = rs + imm;
  regs.gpr[RT(instr)] = result;
  Util::trace("addiu r{}, r{}, {:08X} = {:08X}", RT(instr), RS(instr), (u32)imm, (u32)result);
}

void dadd(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
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

void daddu(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  if(RD(instr) != 0) [[likely]] {
    s64 rs = regs.gpr[RS(instr)];
    s64 rt = regs.gpr[RT(instr)];
    u64 result = rs + rt;
    regs.gpr[RD(instr)] = result;
    Util::trace("daddu r{}, r{}, r{} = {:016X}", RD(instr), RS(instr), RT(instr), result);
  }
}

void daddi(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
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

void daddiu(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  s64 imm = s16(instr);
  s64 rs = regs.gpr[RS(instr)];
  u64 result = rs + imm;
  regs.gpr[RT(instr)] = result;
  Util::trace("daddiu r{}, r{}, {:016X} = {:016X}", RT(instr), RS(instr), imm, result);
}

void div(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
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

void divu(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
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

void ddiv(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
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

void ddivu(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
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

void branch(CachedInterpreter& cpu, bool cond, s64 address) {
  Registers& regs = cpu.regs;
  regs.delaySlot = true;
  if (cond) {
    regs.nextPC = address;
    Util::trace("b {:016X}", (u64)address);
  }
}

void branch_likely(CachedInterpreter& cpu, bool cond, s64 address) {
  Registers& regs = cpu.regs;
  if (cond) {
    regs.delaySlot = true;
    regs.nextPC = address;
    Util::trace("bl {:016X}", (u64)address);
  } else {
    regs.SetPC64(regs.nextPC);
  }
}

void b(CachedInterpreter& cpu, u32 instr, bool cond) {
  Registers& regs = cpu.regs;
  s16 imm = instr;
  s64 offset = (s64)imm << 2;
  s64 address = regs.pc + offset;
  branch(cpu, cond, address);
}

void blink(CachedInterpreter& cpu, u32 instr, bool cond) {
  Registers& regs = cpu.regs;
  regs.gpr[31] = regs.nextPC;
  s16 imm = instr;
  s64 offset = (s64)imm << 2;
  s64 address = regs.pc + offset;
  branch(cpu, cond, address);
}

void bl(CachedInterpreter& cpu, u32 instr, bool cond) {
  Registers& regs = cpu.regs;
  s16 imm = instr;
  s64 offset = (s64)imm << 2;
  s64 address = regs.pc + offset;
  branch_likely(cpu, cond, address);
}

void bllink(CachedInterpreter& cpu, u32 instr, bool cond) {
  Registers& regs = cpu.regs;
  regs.gpr[31] = regs.nextPC;
  s16 imm = instr;
  s64 offset = (s64)imm << 2;
  s64 address = regs.pc + offset;
  branch_likely(cpu, cond, address);
}

void lui(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  s64 val = (s16)instr;
  val <<= 16;
  regs.gpr[RT(instr)] = val;
  Util::trace("lui r{}, {:016X}", RT(instr), (u64)val);
}

void lb(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  u64 address = regs.gpr[RS(instr)] + (s16)instr;
  u32 paddr = 0;
  if(!MapVAddr(regs, LOAD, address, paddr)) {
    HandleTLBException(regs, address);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, LOAD), 0, true);
  } else {
    regs.gpr[RT(instr)] = (s8)cpu.mem.Read8(regs, paddr);
    Util::trace("lb r{}, [{:08X}]", RT(instr), paddr);
  }
}

void lh(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
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
    regs.gpr[RT(instr)] = (s16)cpu.mem.Read16(regs, paddr);
    Util::trace("lh r{}, [{:08X}]", RT(instr), paddr);
  }
}

void lw(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
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
    regs.gpr[RT(instr)] = (s32)cpu.mem.Read32(regs, paddr);
    Util::trace("lw r{}, [{:08X}]", RT(instr), paddr);
  }
}

void ll(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
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
      regs.gpr[RT(instr)] = (s32)cpu.mem.Read32(regs, paddr);
      Util::trace("lw r{}, [{:08X}]", RT(instr), paddr);
    }
  }

  regs.cop0.llbit = true;
  regs.cop0.LLAddr = paddr >> 4;
}

void lwl(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  u64 address = regs.gpr[RS(instr)] + (s16)instr;
  u32 paddr = 0;
  if(!MapVAddr(regs, LOAD, address, paddr)) {
    HandleTLBException(regs, address);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, LOAD), 0, true);
  } else {
    u32 shift = 8 * ((address ^ 0) & 3);
    u32 mask = 0xFFFFFFFF << shift;
    u32 data = cpu.mem.Read32(regs, paddr & ~3);
    s32 result = s32((regs.gpr[RT(instr)] & ~mask) | (data << shift));
    regs.gpr[RT(instr)] = result;
    Util::trace("lwl r{}, [{:08X}]", RT(instr), paddr);
  }
}

void lwr(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  u64 address = regs.gpr[RS(instr)] + (s16)instr;
  u32 paddr = 0;
  if(!MapVAddr(regs, LOAD, address, paddr)) {
    HandleTLBException(regs, address);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, LOAD), 0, true);
  } else {
    u32 shift = 8 * ((address ^ 3) & 3);
    u32 mask = 0xFFFFFFFF >> shift;
    u32 data = cpu.mem.Read32(regs, paddr & ~3);
    s32 result = s32((regs.gpr[RT(instr)] & ~mask) | (data >> shift));
    regs.gpr[RT(instr)] = result;
    Util::trace("lwr r{}, [{:08X}]", RT(instr), paddr);
  }
}

void ld(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
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
    s64 value = cpu.mem.Read64(regs, paddr);
    regs.gpr[RT(instr)] = value;
    Util::trace("ld r{}, [{:08X}]", RT(instr), paddr);
  }
}

void lld(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
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
      regs.gpr[RT(instr)] = cpu.mem.Read64(regs, paddr);
      Util::trace("lld r{}, [{:08X}]", RT(instr), paddr);
      regs.cop0.llbit = true;
      regs.cop0.LLAddr = paddr >> 4;
    }
  }
}

void ldl(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  u64 address = regs.gpr[RS(instr)] + (s16)instr;
  u32 paddr = 0;
  if (!MapVAddr(regs, LOAD, address, paddr)) {
    HandleTLBException(regs, address);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, LOAD), 0, true);
  } else {
    s32 shift = 8 * ((address ^ 0) & 7);
    u64 mask = 0xFFFFFFFFFFFFFFFF << shift;
    u64 data = cpu.mem.Read64(regs, paddr & ~7);
    s64 result = (s64) ((regs.gpr[RT(instr)] & ~mask) | (data << shift));
    regs.gpr[RT(instr)] = result;
    Util::trace("ldl r{}, [{:08X}]", RT(instr), paddr);
  }
}

void ldr(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  u64 address = regs.gpr[RS(instr)] + (s16)instr;
  u32 paddr;
  if (!MapVAddr(regs, LOAD, address, paddr)) {
    HandleTLBException(regs, address);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, LOAD), 0, true);
  } else {
    s32 shift = 8 * ((address ^ 7) & 7);
    u64 mask = 0xFFFFFFFFFFFFFFFF >> shift;
    u64 data = cpu.mem.Read64(regs, paddr & ~7);
    s64 result = (s64) ((regs.gpr[RT(instr)] & ~mask) | (data >> shift));
    regs.gpr[RT(instr)] = result;
    Util::trace("ldr r{}, [{:08X}]", RT(instr), paddr);
  }
}

void lbu(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  u64 address = regs.gpr[RS(instr)] + (s16)instr;
  u32 paddr;
  if (!MapVAddr(regs, LOAD, address, paddr)) {
    HandleTLBException(regs, address);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, LOAD), 0, true);
  } else {
    u8 value = cpu.mem.Read8(regs, paddr);
    regs.gpr[RT(instr)] = value;
    Util::trace("lbu r{}, [{:08X}]", RT(instr), paddr);
  }
}

void lhu(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
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
    u16 value = cpu.mem.Read16(regs, paddr);
    regs.gpr[RT(instr)] = value;
    Util::trace("lhu r{}, [{:08X}]", RT(instr), paddr);
  }
}

void lwu(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
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
    u32 value = cpu.mem.Read32(regs, paddr);
    regs.gpr[RT(instr)] = value;
    Util::trace("lwu r{}, [{:08X}]", RT(instr), paddr);
  }
}

void sb(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  u64 address = regs.gpr[RS(instr)] + (s16)instr;
  u32 paddr;
  if (!MapVAddr(regs, STORE, address, paddr)) {
    HandleTLBException(regs, address);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, STORE), 0, true);
  } else {
    cpu.mem.Write8(regs, cpu, paddr, regs.gpr[RT(instr)]);
    Util::trace("sb r{}, [{:08X}]", RT(instr), paddr);
  }
}

void sc(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
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
      cpu.mem.Write32(regs, cpu, paddr, regs.gpr[RT(instr)]);
      Util::trace("sc r{}, [{:08X}]", RT(instr), paddr);
      regs.gpr[RT(instr)] = 1;
    }
  } else {
    regs.gpr[RT(instr)] = 0;
  }
}

void scd(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
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
      cpu.mem.Write32(regs, cpu, paddr, regs.gpr[RT(instr)]);
      Util::trace("scd r{}, [{:08X}]", RT(instr), paddr);
      regs.gpr[RT(instr)] = 1;
    }
  } else {
    regs.gpr[RT(instr)] = 0;
  }
}

void sh(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  s64 address = regs.gpr[RS(instr)] + (s16)instr;

  u32 paddr;
  if(!MapVAddr(regs, STORE, address, paddr)) {
    HandleTLBException(regs, address);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, STORE), 0, true);
  } else {
    cpu.mem.Write16(regs, cpu, paddr, regs.gpr[RT(instr)]);
    Util::trace("sh r{}, [{:08X}]", RT(instr), paddr);
  }
}

void sw(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
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
    cpu.mem.Write32(regs, cpu, paddr, regs.gpr[RT(instr)]);
    Util::trace("sw r{}, [{:08X}]", RT(instr), paddr);
  }
}

void sd(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
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
    cpu.mem.Write64(regs, cpu, paddr, regs.gpr[RT(instr)]);
    Util::trace("sd r{}, [{:08X}]", RT(instr), paddr);
  }
}

void sdl(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  u64 address = regs.gpr[RS(instr)] + (s16)instr;
  u32 paddr;
  if (!MapVAddr(regs, STORE, address, paddr)) {
    HandleTLBException(regs, address);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, STORE), 0, true);
  } else {
    s32 shift = 8 * ((address ^ 0) & 7);
    u64 mask = 0xFFFFFFFFFFFFFFFF >> shift;
    u64 data = cpu.mem.Read64(regs, paddr & ~7);
    u64 rt = regs.gpr[RT(instr)];
    cpu.mem.Write64(regs, cpu, paddr & ~7, (data & ~mask) | (rt >> shift));
    Util::trace("sdl r{}, [{:08X}]", RT(instr), paddr);
  }
}

void sdr(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  u64 address = regs.gpr[RS(instr)] + (s16)instr;
  u32 paddr;
  if (!MapVAddr(regs, STORE, address, paddr)) {
    HandleTLBException(regs, address);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, STORE), 0, true);
  } else {
    s32 shift = 8 * ((address ^ 7) & 7);
    u64 mask = 0xFFFFFFFFFFFFFFFF << shift;
    u64 data = cpu.mem.Read64(regs, paddr & ~7);
    u64 rt = regs.gpr[RT(instr)];
    cpu.mem.Write64(regs, cpu, paddr & ~7, (data & ~mask) | (rt << shift));
    Util::trace("sdr r{}, [{:08X}]", RT(instr), paddr);
  }
}

void swl(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  u64 address = regs.gpr[RS(instr)] + (s16)instr;
  u32 paddr;
  if (!MapVAddr(regs, STORE, address, paddr)) {
    HandleTLBException(regs, address);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, STORE), 0, true);
  } else {
    u32 shift = 8 * ((address ^ 0) & 3);
    u32 mask = 0xFFFFFFFF >> shift;
    u32 data = cpu.mem.Read32(regs, paddr & ~3);
    u32 rt = regs.gpr[RT(instr)];
    cpu.mem.Write32(regs, cpu, paddr & ~3, (data & ~mask) | (rt >> shift));
    Util::trace("swl r{}, [{:08X}]", RT(instr), paddr);
  }
}

void swr(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  u64 address = regs.gpr[RS(instr)] + (s16)instr;
  u32 paddr;
  if (!MapVAddr(regs, STORE, address, paddr)) {
    HandleTLBException(regs, address);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, STORE), 0, true);
  } else {
    u32 shift = 8 * ((address ^ 3) & 3);
    u32 mask = 0xFFFFFFFF << shift;
    u32 data = cpu.mem.Read32(regs, paddr & ~3);
    u32 rt = regs.gpr[RT(instr)];
    cpu.mem.Write32(regs, cpu, paddr & ~3, (data & ~mask) | (rt << shift));
    Util::trace("swr r{}, [{:08X}]", RT(instr), paddr);
  }
}

void ori(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  s64 imm = (u16)instr;
  s64 result = imm | regs.gpr[RS(instr)];
  regs.gpr[RT(instr)] = result;
  Util::trace("ori r{}, r{}, {:016X}", RT(instr), RS(instr), imm);
}

void or_(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  if (RD(instr) != 0) [[likely]] {
    regs.gpr[RD(instr)] = regs.gpr[RS(instr)] | regs.gpr[RT(instr)];
    Util::trace("or r{}, r{}, r{}", RD(instr), RS(instr), RT(instr));
  }
}

void nor(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  if(RD(instr) != 0) [[likely]] {
    regs.gpr[RD(instr)] = ~(regs.gpr[RS(instr)] | regs.gpr[RT(instr)]);
    Util::trace("nor r{}, r{}, r{}", RD(instr), RS(instr), RT(instr));
  }
}

void j(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  s32 target = (instr & 0x3ffffff) << 2;
  s64 address = (regs.oldPC & ~0xfffffff) | target;

  branch(cpu, true, address);
}

void jal(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  regs.gpr[31] = regs.nextPC;
  j(cpu, instr);
}

void jalr(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  branch(cpu, true, regs.gpr[RS(instr)]);
  if(RD(instr) != 0) [[likely]] {
    regs.gpr[RD(instr)] = regs.pc + 4;
  }
}

void slti(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  s16 imm = instr;
  regs.gpr[RT(instr)] = regs.gpr[RS(instr)] < imm;
  Util::trace("slti r{}, r{}, {:04X}", RT(instr), RS(instr), u16(imm));
}

void sltiu(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  s16 imm = instr;
  regs.gpr[RT(instr)] = (u64)regs.gpr[RS(instr)] < imm;
  Util::trace("sltiu r{}, r{}, {:04X}", RT(instr), RS(instr), u16(imm));
}

void slt(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  if(RD(instr) != 0) [[likely]] {
    regs.gpr[RD(instr)] = regs.gpr[RS(instr)] < regs.gpr[RT(instr)];
    Util::trace("slt r{}, r{}, r{}", RD(instr), RS(instr), RS(instr));
  }
}

void sltu(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  if(RD(instr) != 0) [[likely]] {
    regs.gpr[RD(instr)] = (u64) regs.gpr[RS(instr)] < (u64) regs.gpr[RT(instr)];
    Util::trace("sltu r{}, r{}, r{}", RD(instr), RS(instr), RS(instr));
  }
}

void xori(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  s64 imm = (u16)instr;
  regs.gpr[RT(instr)] = regs.gpr[RS(instr)] ^ imm;
  Util::trace("xori r{}, r{}, {:04X}", RT(instr), RS(instr), (u16)imm);
}

void xor_(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  if(RD(instr) != 0) [[likely]] {
    regs.gpr[RD(instr)] = regs.gpr[RT(instr)] ^ regs.gpr[RS(instr)];
    Util::trace("xor r{}, r{}, r{}", RD(instr), RS(instr), RS(instr));
  }
}

void andi(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  s64 imm = (u16)instr;
  regs.gpr[RT(instr)] = regs.gpr[RS(instr)] & imm;
  Util::trace("andi r{}, r{}, {:04X}", RT(instr), RS(instr), (u16)imm);
}

void and_(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  if(RD(instr) != 0) [[likely]] {
    regs.gpr[RD(instr)] = regs.gpr[RS(instr)] & regs.gpr[RT(instr)];
    Util::trace("and r{}, r{}, r{}", RD(instr), RS(instr), RS(instr));
  }
}

void sll(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  if(RD(instr) != 0) [[likely]] {
    u8 sa = ((instr >> 6) & 0x1f);
    s32 result = regs.gpr[RT(instr)] << sa;
    regs.gpr[RD(instr)] = (s64) result;
    Util::trace("sll r{}, r{}, {:02X}", RD(instr), RT(instr), sa);
  }
}

void sllv(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  if(RD(instr) != 0) [[likely]] {
    u8 sa = (regs.gpr[RS(instr)]) & 0x1F;
    u32 rt = regs.gpr[RT(instr)];
    s32 result = rt << sa;
    regs.gpr[RD(instr)] = (s64) result;
    Util::trace("sllv r{}, r{}, r{}", RD(instr), RT(instr), RS(instr));
  }
}

void dsll32(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  if(RD(instr) != 0) [[likely]] {
    u8 sa = ((instr >> 6) & 0x1f);
    s64 result = regs.gpr[RT(instr)] << (sa + 32);
    regs.gpr[RD(instr)] = result;
    Util::trace("dsll32 r{}, r{}, {:02X}", RD(instr), RT(instr), sa);
  }
}

void dsll(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  if(RD(instr) != 0) [[likely]] {
    u8 sa = ((instr >> 6) & 0x1f);
    s64 result = regs.gpr[RT(instr)] << sa;
    regs.gpr[RD(instr)] = result;
    Util::trace("dsll r{}, r{}, {:02X}", RD(instr), RT(instr), sa);
  }
}

void dsllv(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  if(RD(instr) != 0) [[likely]] {
    s64 sa = regs.gpr[RS(instr)] & 63;
    s64 result = regs.gpr[RT(instr)] << sa;
    regs.gpr[RD(instr)] = result;
    Util::trace("dsllv r{}, r{}, r{}", RD(instr), RT(instr), RS(instr));
  }
}

void srl(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  if(RD(instr) != 0) [[likely]] {
    u32 rt = regs.gpr[RT(instr)];
    u8 sa = ((instr >> 6) & 0x1f);
    u32 result = rt >> sa;
    regs.gpr[RD(instr)] = (s32) result;
    Util::trace("srl r{}, r{}, {:02X}", RD(instr), RT(instr), sa);
  }
}

void srlv(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  if(RD(instr) != 0) [[likely]] {
    u8 sa = (regs.gpr[RS(instr)] & 0x1F);
    u32 rt = regs.gpr[RT(instr)];
    s32 result = rt >> sa;
    regs.gpr[RD(instr)] = (s64) result;
    Util::trace("srlv r{}, r{}, r{}", RD(instr), RT(instr), RS(instr));
  }
}

void dsrl(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  if(RD(instr) != 0) [[likely]] {
    u64 rt = regs.gpr[RT(instr)];
    u8 sa = ((instr >> 6) & 0x1f);
    u64 result = rt >> sa;
    regs.gpr[RD(instr)] = s64(result);
    Util::trace("dsrl r{}, r{}, {:02X}", RD(instr), RT(instr), sa);
  }
}

void dsrlv(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  if(RD(instr) != 0) [[likely]] {
    u8 amount = (regs.gpr[RS(instr)] & 63);
    u64 rt = regs.gpr[RT(instr)];
    u64 result = rt >> amount;
    regs.gpr[RD(instr)] = s64(result);
    Util::trace("dsrlv r{}, r{}, r{}", RD(instr), RT(instr), RS(instr));
  }
}

void dsrl32(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  if(RD(instr) != 0) [[likely]] {
    u64 rt = regs.gpr[RT(instr)];
    u8 sa = ((instr >> 6) & 0x1f);
    u64 result = rt >> (sa + 32);
    regs.gpr[RD(instr)] = s64(result);
    Util::trace("dsrl32 r{}, r{}, {:02X}", RD(instr), RT(instr), sa);
  }
}

void sra(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  if(RD(instr) != 0) [[likely]] {
    s64 rt = regs.gpr[RT(instr)];
    u8 sa = ((instr >> 6) & 0x1f);
    s32 result = rt >> sa;
    regs.gpr[RD(instr)] = result;
    Util::trace("sra r{}, r{}, {:02X}", RD(instr), RT(instr), sa);
  }
}

void srav(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  if(RD(instr) != 0) [[likely]] {
    s64 rt = regs.gpr[RT(instr)];
    s64 rs = regs.gpr[RS(instr)];
    u8 sa = rs & 0x1f;
    s32 result = rt >> sa;
    regs.gpr[RD(instr)] = result;
    Util::trace("srav r{}, r{}, r{}", RD(instr), RT(instr), RS(instr));
  }
}

void dsra(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  if(RD(instr) != 0) [[likely]] {
    s64 rt = regs.gpr[RT(instr)];
    u8 sa = ((instr >> 6) & 0x1f);
    s64 result = rt >> sa;
    regs.gpr[RD(instr)] = result;
    Util::trace("dsra r{}, r{}, {:02X}", RD(instr), RT(instr), sa);
  }
}

void dsrav(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  if(RD(instr) != 0) [[likely]] {
    s64 rt = regs.gpr[RT(instr)];
    s64 rs = regs.gpr[RS(instr)];
    s64 sa = rs & 63;
    s64 result = rt >> sa;
    regs.gpr[RD(instr)] = result;
    Util::trace("dsrav r{}, r{}, r{}", RD(instr), RT(instr), RS(instr));
  }
}

void dsra32(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  if(RD(instr) != 0) [[likely]] {
    s64 rt = regs.gpr[RT(instr)];
    u8 sa = ((instr >> 6) & 0x1f);
    s64 result = rt >> (sa + 32);
    regs.gpr[RD(instr)] = result;
    Util::trace("dsra32 r{}, r{}, {:02X}", RD(instr), RT(instr), sa);
  }
}

void jr(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  s64 address = regs.gpr[RS(instr)];
  branch(cpu, true, address);
}

void dsub(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
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

void dsubu(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  if(RD(instr) != 0) [[likely]] {
    u64 rt = regs.gpr[RT(instr)];
    u64 rs = regs.gpr[RS(instr)];
    u64 result = rs - rt;
    regs.gpr[RD(instr)] = s64(result);
    Util::trace("dsubu r{}, r{}, r{}", RD(instr), RT(instr), RS(instr));
  }
}

void sub(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
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

void subu(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  if(RD(instr) != 0) [[likely]] {
    u32 rt = regs.gpr[RT(instr)];
    u32 rs = regs.gpr[RS(instr)];
    u32 result = rs - rt;
    regs.gpr[RD(instr)] = (s64) ((s32) result);
    Util::trace("subu r{}, r{}, r{}", RD(instr), RT(instr), RS(instr));
  }
}

void dmultu(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  u64 rt = regs.gpr[RT(instr)];
  u64 rs = regs.gpr[RS(instr)];
  u128 result = (u128)rt * (u128)rs;
  regs.lo = (s64)(result & 0xFFFFFFFFFFFFFFFF);
  regs.hi = (s64)(result >> 64);
  Util::trace("dmultu r{}, r{}", RT(instr), RS(instr));
}

void dmult(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  s64 rt = regs.gpr[RT(instr)];
  s64 rs = regs.gpr[RS(instr)];
  s128 result = (s128)rt * (s128)rs;
  regs.lo = result & 0xFFFFFFFFFFFFFFFF;
  regs.hi = result >> 64;
  Util::trace("dmult r{}, r{}", RT(instr), RS(instr));
}

void multu(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  u32 rt = regs.gpr[RT(instr)];
  u32 rs = regs.gpr[RS(instr)];
  u64 result = (u64)rt * (u64)rs;
  regs.lo = (s64)((s32)result);
  regs.hi = (s64)((s32)(result >> 32));
  Util::trace("multu r{}, r{}", RT(instr), RS(instr));
}

void mult(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  s32 rt = regs.gpr[RT(instr)];
  s32 rs = regs.gpr[RS(instr)];
  s64 result = (s64)rt * (s64)rs;
  regs.lo = (s64)((s32)result);
  regs.hi = (s64)((s32)(result >> 32));
  Util::trace("mult r{}, r{}", RT(instr), RS(instr));
}

void mflo(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  if(RD(instr) != 0) [[likely]] {
    regs.gpr[RD(instr)] = regs.lo;
    Util::trace("mflo r{}", RD(instr));
  }
}

void mfhi(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  if(RD(instr) != 0) [[likely]] {
    regs.gpr[RD(instr)] = regs.hi;
    Util::trace("mfhi r{}", RD(instr));
  }
}

void mtlo(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  regs.lo = regs.gpr[RS(instr)];
  Util::trace("mtlo r{}", RS(instr));
}

void mthi(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  regs.hi = regs.gpr[RS(instr)];
  Util::trace("mthi r{}", RS(instr));
}

void trap(CachedInterpreter& cpu, bool cond) {
  Registers& regs = cpu.regs;
  if(cond) {
    FireException(regs, ExceptionCode::Trap, 0, true);
    Util::trace("trap");
  }
}

void mtc2(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  cpu.cop2Latch = regs.gpr[RT(instr)];
}

void mfc2(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  s32 value = cpu.cop2Latch;
  regs.gpr[RT(instr)] = value;
}

void dmtc2(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  cpu.cop2Latch = regs.gpr[RT(instr)];
}

void dmfc2(CachedInterpreter& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  regs.gpr[RT(instr)] = cpu.cop2Latch;
}

void ctc2(CachedInterpreter& cpu, u32) {
  Registers& regs = cpu.regs;
}

void cfc2(CachedInterpreter& cpu, u32) {
  Registers& regs = cpu.regs;
}
}