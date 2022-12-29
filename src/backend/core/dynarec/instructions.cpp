#include <core/Dynarec.hpp>

#define se_imm(x) ((s16)((x) & 0xFFFF))
#define check_address_error(mask, addr) (((!regs.cop0.is_64bit_addressing) && (s32)(addr) != (addr)) || (((addr) & (mask)) != 0))
#define check_signed_overflow(op1, op2, res) (((~((op1) ^ (op2)) & ((op1) ^ (res))) >> ((sizeof(res) * 8) - 1)) & 1)
#define check_signed_underflow(op1, op2, res) (((((op1) ^ (op2)) & ((op1) ^ (res))) >> ((sizeof(res) * 8) - 1)) & 1)

namespace n64 {
void Dynarec::add(Registers& regs, u32 instr) {
  u32 rs = (s32)regs.gpr[RS(instr)];
  u32 rt = (s32)regs.gpr[RT(instr)];
  u32 result = rs + rt;
  if(check_signed_overflow(rs, rt, result)) {
    FireException(regs, ExceptionCode::Overflow, 0, regs.oldPC);
  } else {
    regs.gpr[RD(instr)] = s32(result);
  }
}

void Dynarec::addu(Registers& regs, u32 instr) {
  s32 rs = (s32)regs.gpr[RS(instr)];
  s32 rt = (s32)regs.gpr[RT(instr)];
  s32 result = rs + rt;
  regs.gpr[RD(instr)] = result;
}

void Dynarec::addi(Registers& regs, u32 instr) {
  u32 rs = regs.gpr[RS(instr)];
  u32 imm = s32(s16(instr));
  u32 result = rs + imm;
  if(check_signed_overflow(rs, imm, result)) {
    FireException(regs, ExceptionCode::Overflow, 0, regs.oldPC);
  } else {
    regs.gpr[RT(instr)] = s32(result);
  }
}

void Dynarec::addiu(Registers& regs, u32 instr) {
  s32 rs = (s32)regs.gpr[RS(instr)];
  s16 imm = (s16)(instr);
  s32 result = rs + imm;
  regs.gpr[RT(instr)] = result;
}

void Dynarec::dadd(Registers& regs, u32 instr) {
  u64 rs = regs.gpr[RS(instr)];
  u64 rt = regs.gpr[RT(instr)];
  u64 result = rt + rs;
  if(check_signed_overflow(rs, rt, result)) {
    FireException(regs, ExceptionCode::Overflow, 0, regs.oldPC);
  } else {
    regs.gpr[RD(instr)] = result;
  }
}

void Dynarec::daddu(Registers& regs, u32 instr) {
  s64 rs = regs.gpr[RS(instr)];
  s64 rt = regs.gpr[RT(instr)];
  regs.gpr[RD(instr)] = rs + rt;
}

void Dynarec::daddi(Registers& regs, u32 instr) {
  u64 imm = s64(s16(instr));
  u64 rs = regs.gpr[RS(instr)];
  u64 result = imm + rs;
  if(check_signed_overflow(rs, imm, result)) {
    FireException(regs, ExceptionCode::Overflow, 0, regs.oldPC);
  } else {
    regs.gpr[RT(instr)] = result;
  }
}

void Dynarec::daddiu(Registers& regs, u32 instr) {
  s16 imm = (s16)(instr);
  s64 rs = regs.gpr[RS(instr)];
  regs.gpr[RT(instr)] = rs + imm;
}

void Dynarec::div(Registers& regs, u32 instr) {
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

void Dynarec::divu(Registers& regs, u32 instr) {
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

void Dynarec::ddiv(Registers& regs, u32 instr) {
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

void Dynarec::ddivu(Registers& regs, u32 instr) {
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

void Dynarec::branch(Registers& regs, bool cond, s64 address) {
  regs.delaySlot = true;
  if (cond) {
    regs.nextPC = address;
  }
}

void Dynarec::branch_likely(Registers& regs, bool cond, s64 address) {
  regs.delaySlot = true;
  if (cond) {
    regs.nextPC = address;
  } else {
    regs.SetPC(regs.nextPC);
  }
}

void Dynarec::b(Registers& regs, u32 instr, bool cond) {
  s64 offset = (s64)se_imm(instr) << 2;
  s64 address = regs.pc + offset;
  branch(regs, cond, address);
}

void Dynarec::blink(Registers& regs, u32 instr, bool cond) {
  regs.gpr[31] = regs.nextPC;
  s64 offset = (s64)se_imm(instr) << 2;
  s64 address = regs.pc + offset;
  branch(regs, cond, address);
}

void Dynarec::bl(Registers& regs, u32 instr, bool cond) {
  s64 offset = (s64)se_imm(instr) << 2;
  s64 address = regs.pc + offset;
  branch_likely(regs, cond, address);
}

void Dynarec::bllink(Registers& regs, u32 instr, bool cond) {
  regs.gpr[31] = regs.nextPC;
  s64 offset = (s64)se_imm(instr) << 2;
  s64 address = regs.pc + offset;
  branch_likely(regs, cond, address);
}

void Dynarec::lui(Registers& regs, u32 instr) {
  s64 val = (s16)instr;
  val <<= 16;
  regs.gpr[RT(instr)] = val;
}

void Dynarec::lb(Registers& regs, Mem& mem, u32 instr) {
  u64 address = regs.gpr[RS(instr)] + (s16)instr;
  regs.gpr[RT(instr)] = (s8)mem.Read8(regs, address, regs.oldPC);
}

void Dynarec::lh(Registers& regs, Mem& mem, u32 instr) {
  u64 address = regs.gpr[RS(instr)] + (s16)instr;
  if (check_address_error(address, 0b1)) {
    HandleTLBException(regs, address);
    FireException(regs, ExceptionCode::AddressErrorLoad, 0, regs.oldPC);
    return;
  }

  regs.gpr[RT(instr)] = (s16)mem.Read16(regs, address, regs.oldPC);
}

void Dynarec::lw(Registers& regs, Mem& mem, u32 instr) {
  s16 offset = instr;
  u64 address = regs.gpr[RS(instr)] + offset;
  if (check_address_error(address, 0b11)) {
    HandleTLBException(regs, address);
    FireException(regs, ExceptionCode::AddressErrorLoad, 0, regs.oldPC);
    return;
  }

  u32 physical;
  if (!MapVAddr(regs, LOAD, address, physical)) {
    HandleTLBException(regs, address);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, LOAD), 0, regs.oldPC);
  } else {
    regs.gpr[RT(instr)] = (s32)mem.Read32<false>(regs, physical, regs.oldPC);
  }
}

void Dynarec::ll(Registers& regs, Mem& mem, u32 instr) {
  s64 address = regs.gpr[RS(instr)] + (s16)instr;
  u32 physical;
  if (!MapVAddr(regs, LOAD, address, physical)) {
    HandleTLBException(regs, address);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, LOAD), 0, regs.oldPC);
  } else {
    regs.gpr[RT(instr)] = (s32)mem.Read32<false>(regs, physical, regs.oldPC);
  }

  regs.cop0.llbit = true;
  regs.cop0.LLAddr = physical >> 4;
}

void Dynarec::lwl(Registers& regs, Mem& mem, u32 instr) {
  u64 address = regs.gpr[RS(instr)] + (s16)instr;
  u32 paddr = 0;
  if(!MapVAddr(regs, LOAD, address, paddr)) {
    HandleTLBException(regs, address);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, LOAD), 0, regs.oldPC);
  } else {
    u32 shift = 8 * ((address ^ 0) & 3);
    u32 mask = 0xFFFFFFFF << shift;
    u32 data = mem.Read32<false>(regs, paddr & ~3, regs.oldPC);
    s32 result = s32((regs.gpr[RT(instr)] & ~mask) | (data << shift));
    regs.gpr[RT(instr)] = result;
  }
}

void Dynarec::lwr(Registers& regs, Mem& mem, u32 instr) {
  u64 address = regs.gpr[RS(instr)] + (s16)instr;
  u32 paddr = 0;
  if(!MapVAddr(regs, LOAD, address, paddr)) {
    HandleTLBException(regs, address);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, LOAD), 0, regs.oldPC);
  } else {
    u32 shift = 8 * ((address ^ 3) & 3);
    u32 mask = 0xFFFFFFFF >> shift;
    u32 data = mem.Read32<false>(regs, paddr & ~3, regs.oldPC);
    s32 result = s32((regs.gpr[RT(instr)] & ~mask) | (data >> shift));
    regs.gpr[RT(instr)] = result;
  }
}

void Dynarec::ld(Registers& regs, Mem& mem, u32 instr) {
  s64 address = regs.gpr[RS(instr)] + (s16)instr;
  if (check_address_error(address, 0b111)) {
    HandleTLBException(regs, address);
    FireException(regs, ExceptionCode::AddressErrorLoad, 0, regs.oldPC);
    return;
  }

  s64 value = mem.Read64(regs, address, regs.oldPC);
  regs.gpr[RT(instr)] = value;
}

void Dynarec::lld(Registers& regs, Mem& mem, u32 instr) {
  s64 address = regs.gpr[RS(instr)] + (s16)instr;
  u32 paddr;
  if (!MapVAddr(regs, LOAD, address, paddr)) {
    HandleTLBException(regs, address);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, LOAD), 0, regs.oldPC);
  } else {
    regs.gpr[RT(instr)] = mem.Read64<false>(regs, paddr, regs.oldPC);
  }

  regs.cop0.llbit = true;
  regs.cop0.LLAddr = paddr >> 4;
}

void Dynarec::ldl(Registers& regs, Mem& mem, u32 instr) {
  u64 address = regs.gpr[RS(instr)] + (s16)instr;
  u32 paddr = 0;
  if (!MapVAddr(regs, LOAD, address, paddr)) {
    HandleTLBException(regs, address);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, LOAD), 0, regs.oldPC);
  } else {
    s32 shift = 8 * ((address ^ 0) & 7);
    u64 mask = 0xFFFFFFFFFFFFFFFF << shift;
    u64 data = mem.Read64<false>(regs, paddr & ~7, regs.oldPC);
    s64 result = (s64) ((regs.gpr[RT(instr)] & ~mask) | (data << shift));
    regs.gpr[RT(instr)] = result;
  }
}

void Dynarec::ldr(Registers& regs, Mem& mem, u32 instr) {
  u64 address = regs.gpr[RS(instr)] + (s16)instr;
  u32 paddr;
  if (!MapVAddr(regs, LOAD, address, paddr)) {
    HandleTLBException(regs, address);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, LOAD), 0, regs.oldPC);
  } else {
    s32 shift = 8 * ((address ^ 7) & 7);
    u64 mask = 0xFFFFFFFFFFFFFFFF >> shift;
    u64 data = mem.Read64<false>(regs, paddr & ~7, regs.oldPC);
    s64 result = (s64) ((regs.gpr[RT(instr)] & ~mask) | (data >> shift));
    regs.gpr[RT(instr)] = result;
  }
}

void Dynarec::lbu(Registers& regs, Mem& mem, u32 instr) {
  u64 address = regs.gpr[RS(instr)] + (s16)instr;
  u8 value = mem.Read8(regs, address, regs.oldPC);
  regs.gpr[RT(instr)] = value;
}

void Dynarec::lhu(Registers& regs, Mem& mem, u32 instr) {
  s64 address = regs.gpr[RS(instr)] + (s16)instr;
  if (check_address_error(address, 0b1)) {
    HandleTLBException(regs, address);
    FireException(regs, ExceptionCode::AddressErrorLoad, 0, regs.oldPC);
    return;
  }

  u16 value = mem.Read16(regs, address, regs.oldPC);
  regs.gpr[RT(instr)] = value;
}

void Dynarec::lwu(Registers& regs, Mem& mem, u32 instr) {
  s64 address = regs.gpr[RS(instr)] + (s16)instr;
  if (check_address_error(address, 0b11)) {
    HandleTLBException(regs, address);
    FireException(regs, ExceptionCode::AddressErrorLoad, 0, regs.oldPC);
    return;
  }

  u32 value = mem.Read32(regs, address, regs.oldPC);
  regs.gpr[RT(instr)] = value;
}

void Dynarec::sb(Registers& regs, Mem& mem, u32 instr) {
  u32 address = regs.gpr[RS(instr)] + (s16)instr;
  mem.Write8(regs, address, regs.gpr[RT(instr)], regs.oldPC);
}

void Dynarec::sc(Registers& regs, Mem& mem, u32 instr) {
  s64 address = regs.gpr[RS(instr)] + (s16)instr;
  if (check_address_error(address, 0b11)) {
    HandleTLBException(regs, address);
    FireException(regs, ExceptionCode::AddressErrorStore, 0, regs.oldPC);
  }

  if(regs.cop0.llbit) {
    mem.Write32(regs, address, regs.gpr[RT(instr)], regs.oldPC);
  }

  regs.gpr[RT(instr)] = (u64)regs.cop0.llbit;
  regs.cop0.llbit = false;
}

void Dynarec::scd(Registers& regs, Mem& mem, u32 instr) {
  s64 address = regs.gpr[RS(instr)] + (s16)instr;
  if (check_address_error(address, 0b111)) {
    HandleTLBException(regs, address);
    FireException(regs, ExceptionCode::AddressErrorStore, 0, regs.oldPC);
    return;
  }

  if(regs.cop0.llbit) {
    mem.Write64(regs, address, regs.gpr[RT(instr)], regs.oldPC);
  }

  regs.gpr[RT(instr)] = (s64)((u64)regs.cop0.llbit);
  regs.cop0.llbit = false;
}

void Dynarec::sh(Registers& regs, Mem& mem, u32 instr) {
  s64 address = regs.gpr[RS(instr)] + (s16)instr;
  if (check_address_error(address, 0b1)) {
    HandleTLBException(regs, address);
    FireException(regs, ExceptionCode::AddressErrorStore, 0, regs.oldPC);
    return;
  }

  u32 physical;
  if(!MapVAddr(regs, STORE, address, physical)) {
    HandleTLBException(regs, address);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, STORE), 0, regs.oldPC);
  } else {
    mem.Write16<false>(regs, physical, regs.gpr[RT(instr)], regs.oldPC);
  }
}

void Dynarec::sw(Registers& regs, Mem& mem, u32 instr) {
  s16 offset = instr;
  u64 address = regs.gpr[RS(instr)] + offset;
  if (check_address_error(address, 0b11)) {
    HandleTLBException(regs, address);
    FireException(regs, ExceptionCode::AddressErrorStore, 0, regs.oldPC);
    return;
  }

  u32 physical;
  if(!MapVAddr(regs, STORE, address, physical)) {
    HandleTLBException(regs, address);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, STORE), 0, regs.oldPC);
  } else {
    mem.Write32<false>(regs, physical, regs.gpr[RT(instr)], regs.oldPC);
  }
}

void Dynarec::sd(Registers& regs, Mem& mem, u32 instr) {
  s64 address = regs.gpr[RS(instr)] + (s16)instr;
  if (check_address_error(address, 0b11)) {
    HandleTLBException(regs, address);
    FireException(regs, ExceptionCode::AddressErrorStore, 0, regs.oldPC);
    return;
  }

  u32 physical;
  if(!MapVAddr(regs, STORE, address, physical)) {
    HandleTLBException(regs, address);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, STORE), 0, regs.oldPC);
  } else {
    mem.Write64<false>(regs, physical, regs.gpr[RT(instr)], regs.oldPC);
  }

}

void Dynarec::sdl(Registers& regs, Mem& mem, u32 instr) {
  u64 address = regs.gpr[RS(instr)] + (s16)instr;
  u32 paddr;
  if (!MapVAddr(regs, STORE, address, paddr)) {
    HandleTLBException(regs, address);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, STORE), 0, regs.oldPC);
  } else {
    s32 shift = 8 * ((address ^ 0) & 7);
    u64 mask = 0xFFFFFFFFFFFFFFFF >> shift;
    u64 data = mem.Read64<false>(regs, paddr & ~7, regs.oldPC);
    u64 rt = regs.gpr[RT(instr)];
    mem.Write64<false>(regs, paddr & ~7, (data & ~mask) | (rt >> shift), regs.oldPC);
  }
}

void Dynarec::sdr(Registers& regs, Mem& mem, u32 instr) {
  u64 address = regs.gpr[RS(instr)] + (s16)instr;
  u32 paddr;
  if (!MapVAddr(regs, STORE, address, paddr)) {
    HandleTLBException(regs, address);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, STORE), 0, regs.oldPC);
  } else {
    s32 shift = 8 * ((address ^ 7) & 7);
    u64 mask = 0xFFFFFFFFFFFFFFFF << shift;
    u64 data = mem.Read64<false>(regs, paddr & ~7, regs.oldPC);
    u64 rt = regs.gpr[RT(instr)];
    mem.Write64<false>(regs, paddr & ~7, (data & ~mask) | (rt << shift), regs.oldPC);
  }
}

void Dynarec::swl(Registers& regs, Mem& mem, u32 instr) {
  u64 address = regs.gpr[RS(instr)] + (s16)instr;
  u32 paddr;
  if (!MapVAddr(regs, STORE, address, paddr)) {
    HandleTLBException(regs, address);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, STORE), 0, regs.oldPC);
  } else {
    u32 shift = 8 * ((address ^ 0) & 3);
    u32 mask = 0xFFFFFFFF >> shift;
    u32 data = mem.Read32<false>(regs, paddr & ~3, regs.oldPC);
    u32 rt = regs.gpr[RT(instr)];
    mem.Write32<false>(regs, paddr & ~3, (data & ~mask) | (rt >> shift), regs.oldPC);
  }
}

void Dynarec::swr(Registers& regs, Mem& mem, u32 instr) {
  u64 address = regs.gpr[RS(instr)] + (s16)instr;
  u32 paddr;
  if (!MapVAddr(regs, STORE, address, paddr)) {
    HandleTLBException(regs, address);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, STORE), 0, regs.oldPC);
  } else {
    u32 shift = 8 * ((address ^ 3) & 3);
    u32 mask = 0xFFFFFFFF << shift;
    u32 data = mem.Read32<false>(regs, paddr & ~3, regs.oldPC);
    u32 rt = regs.gpr[RT(instr)];
    mem.Write32<false>(regs, paddr & ~3, (data & ~mask) | (rt << shift), regs.oldPC);
  }
}

void Dynarec::ori(Registers& regs, u32 instr) {
  s64 imm = (u16)instr;
  s64 result = imm | regs.gpr[RS(instr)];
  regs.gpr[RT(instr)] = result;
}

void Dynarec::or_(Registers& regs, u32 instr) {
  regs.gpr[RD(instr)] = regs.gpr[RS(instr)] | regs.gpr[RT(instr)];
}

void Dynarec::nor(Registers& regs, u32 instr) {
  regs.gpr[RD(instr)] = ~(regs.gpr[RS(instr)] | regs.gpr[RT(instr)]);
}

void Dynarec::j(Registers& regs, u32 instr) {
  s32 target = (instr & 0x3ffffff) << 2;
  s64 address = (regs.oldPC & ~0xfffffff) | target;
  if (check_address_error(address, 0b11)) {
    HandleTLBException(regs, address);
    FireException(regs, ExceptionCode::DataBusError, 0, regs.oldPC);
  }

  branch(regs, true, address);
}

void Dynarec::jal(Registers& regs, u32 instr) {
  regs.gpr[31] = regs.nextPC;
  j(regs, instr);
}

void Dynarec::jalr(Registers& regs, u32 instr) {
  branch(regs, true, regs.gpr[RS(instr)]);
  regs.gpr[RD(instr)] = regs.pc + 4;
}

void Dynarec::slti(Registers& regs, u32 instr) {
  regs.gpr[RT(instr)] = regs.gpr[RS(instr)] < se_imm(instr);
}

void Dynarec::sltiu(Registers& regs, u32 instr) {
  regs.gpr[RT(instr)] = (u64)regs.gpr[RS(instr)] < se_imm(instr);
}

void Dynarec::slt(Registers& regs, u32 instr) {
  regs.gpr[RD(instr)] = regs.gpr[RS(instr)] < regs.gpr[RT(instr)];
}

void Dynarec::sltu(Registers& regs, u32 instr) {
  regs.gpr[RD(instr)] = (u64)regs.gpr[RS(instr)] < (u64)regs.gpr[RT(instr)];
}

void Dynarec::xori(Registers& regs, u32 instr) {
  s64 imm = (u16)instr;
  regs.gpr[RT(instr)] = regs.gpr[RS(instr)] ^ imm;
}

void Dynarec::xor_(Registers& regs, u32 instr) {
  regs.gpr[RD(instr)] = regs.gpr[RT(instr)] ^ regs.gpr[RS(instr)];
}

void Dynarec::andi(Registers& regs, u32 instr) {
  s64 imm = (u16)instr;
  regs.gpr[RT(instr)] = regs.gpr[RS(instr)] & imm;
}

void Dynarec::and_(Registers& regs, u32 instr) {
  regs.gpr[RD(instr)] = regs.gpr[RS(instr)] & regs.gpr[RT(instr)];
}

void Dynarec::sll(Registers& regs, u32 instr) {
  u8 sa = ((instr >> 6) & 0x1f);
  s32 result = regs.gpr[RT(instr)] << sa;
  regs.gpr[RD(instr)] = (s64)result;
}

void Dynarec::sllv(Registers& regs, u32 instr) {
  u8 sa = (regs.gpr[RS(instr)]) & 0x1F;
  u32 rt = regs.gpr[RT(instr)];
  s32 result = rt << sa;
  regs.gpr[RD(instr)] = (s64)result;
}

void Dynarec::dsll32(Registers& regs, u32 instr) {
  u8 sa = ((instr >> 6) & 0x1f);
  s64 result = regs.gpr[RT(instr)] << (sa + 32);
  regs.gpr[RD(instr)] = result;
}

void Dynarec::dsll(Registers& regs, u32 instr) {
  u8 sa = ((instr >> 6) & 0x1f);
  s64 result = regs.gpr[RT(instr)] << sa;
  regs.gpr[RD(instr)] = result;
}

void Dynarec::dsllv(Registers& regs, u32 instr) {
  s64 sa = regs.gpr[RS(instr)] & 63;
  s64 result = regs.gpr[RT(instr)] << sa;
  regs.gpr[RD(instr)] = result;
}

void Dynarec::srl(Registers& regs, u32 instr) {
  u32 rt = regs.gpr[RT(instr)];
  u8 sa = ((instr >> 6) & 0x1f);
  u32 result = rt >> sa;
  regs.gpr[RD(instr)] = (s32)result;
}

void Dynarec::srlv(Registers& regs, u32 instr) {
  u8 sa = (regs.gpr[RS(instr)] & 0x1F);
  u32 rt = regs.gpr[RT(instr)];
  s32 result = rt >> sa;
  regs.gpr[RD(instr)] = (s64)result;
}

void Dynarec::dsrl(Registers& regs, u32 instr) {
  u64 rt = regs.gpr[RT(instr)];
  u8 sa = ((instr >> 6) & 0x1f);
  u64 result = rt >> sa;
  regs.gpr[RD(instr)] = s64(result);
}

void Dynarec::dsrlv(Registers& regs, u32 instr) {
  u8 amount = (regs.gpr[RS(instr)] & 63);
  u64 rt = regs.gpr[RT(instr)];
  u64 result = rt >> amount;
  regs.gpr[RD(instr)] = s64(result);
}

void Dynarec::dsrl32(Registers& regs, u32 instr) {
  u64 rt = regs.gpr[RT(instr)];
  u8 sa = ((instr >> 6) & 0x1f);
  u64 result = rt >> (sa + 32);
  regs.gpr[RD(instr)] = s64(result);
}

void Dynarec::sra(Registers& regs, u32 instr) {
  s64 rt = regs.gpr[RT(instr)];
  u8 sa = ((instr >> 6) & 0x1f);
  s32 result = rt >> sa;
  regs.gpr[RD(instr)] = result;
}

void Dynarec::srav(Registers& regs, u32 instr) {
  s64 rt = regs.gpr[RT(instr)];
  s64 rs = regs.gpr[RS(instr)];
  u8 sa = rs & 0x1f;
  s32 result = rt >> sa;
  regs.gpr[RD(instr)] = result;
}

void Dynarec::dsra(Registers& regs, u32 instr) {
  s64 rt = regs.gpr[RT(instr)];
  u8 sa = ((instr >> 6) & 0x1f);
  s64 result = rt >> sa;
  regs.gpr[RD(instr)] = result;
}

void Dynarec::dsrav(Registers& regs, u32 instr) {
  s64 rt = regs.gpr[RT(instr)];
  s64 rs = regs.gpr[RS(instr)];
  s64 sa = rs & 63;
  s64 result = rt >> sa;
  regs.gpr[RD(instr)] = result;
}

void Dynarec::dsra32(Registers& regs, u32 instr) {
  s64 rt = regs.gpr[RT(instr)];
  u8 sa = ((instr >> 6) & 0x1f);
  s64 result = rt >> (sa + 32);
  regs.gpr[RD(instr)] = result;
}

void Dynarec::jr(Registers& regs, u32 instr) {
  s64 address = regs.gpr[RS(instr)];
  if (check_address_error(address, 0b11)) {
    HandleTLBException(regs, address);
    FireException(regs, ExceptionCode::DataBusError, 0, regs.oldPC);
  }

  branch(regs, true, address);
}

void Dynarec::dsub(Registers& regs, u32 instr) {
  s64 rt = regs.gpr[RT(instr)];
  s64 rs = regs.gpr[RS(instr)];
  s64 result = rs - rt;
  if(check_signed_underflow(rs, rt, result)) {
    FireException(regs, ExceptionCode::Overflow, 0, regs.oldPC);
  } else {
    regs.gpr[RD(instr)] = result;
  }
}

void Dynarec::dsubu(Registers& regs, u32 instr) {
  u64 rt = regs.gpr[RT(instr)];
  u64 rs = regs.gpr[RS(instr)];
  u64 result = rs - rt;
  regs.gpr[RD(instr)] = s64(result);
}

void Dynarec::sub(Registers& regs, u32 instr) {
  s32 rt = regs.gpr[RT(instr)];
  s32 rs = regs.gpr[RS(instr)];
  s32 result = rs - rt;
  if(check_signed_underflow(rs, rt, result)) {
    FireException(regs, ExceptionCode::Overflow, 0, regs.oldPC);
  } else {
    regs.gpr[RD(instr)] = result;
  }
}

void Dynarec::subu(Registers& regs, u32 instr) {
  u32 rt = regs.gpr[RT(instr)];
  u32 rs = regs.gpr[RS(instr)];
  u32 result = rs - rt;
  regs.gpr[RD(instr)] = (s64)((s32)result);
}

void Dynarec::dmultu(Registers& regs, u32 instr) {
  u64 rt = regs.gpr[RT(instr)];
  u64 rs = regs.gpr[RS(instr)];
  u128 result = (u128)rt * (u128)rs;
  regs.lo = (s64)(result & 0xFFFFFFFFFFFFFFFF);
  regs.hi = (s64)(result >> 64);
}

void Dynarec::dmult(Registers& regs, u32 instr) {
  s64 rt = regs.gpr[RT(instr)];
  s64 rs = regs.gpr[RS(instr)];
  s128 result = (s128)rt * (s128)rs;
  regs.lo = result & 0xFFFFFFFFFFFFFFFF;
  regs.hi = result >> 64;
}

void Dynarec::multu(Registers& regs, u32 instr) {
  u32 rt = regs.gpr[RT(instr)];
  u32 rs = regs.gpr[RS(instr)];
  u64 result = (u64)rt * (u64)rs;
  regs.lo = (s64)((s32)result);
  regs.hi = (s64)((s32)(result >> 32));
}

void Dynarec::mult(Registers& regs, u32 instr) {
  s32 rt = regs.gpr[RT(instr)];
  s32 rs = regs.gpr[RS(instr)];
  s64 result = (s64)rt * (s64)rs;
  regs.lo = (s64)((s32)result);
  regs.hi = (s64)((s32)(result >> 32));
}

void Dynarec::mflo(Registers& regs, u32 instr) {
  regs.gpr[RD(instr)] = regs.lo;
}

void Dynarec::mfhi(Registers& regs, u32 instr) {
  regs.gpr[RD(instr)] = regs.hi;
}

void Dynarec::mtlo(Registers& regs, u32 instr) {
  regs.lo = regs.gpr[RS(instr)];
}

void Dynarec::mthi(Registers& regs, u32 instr) {
  regs.hi = regs.gpr[RS(instr)];
}

void Dynarec::trap(Registers& regs, bool cond) {
  if(cond) {
    FireException(regs, ExceptionCode::Trap, 0, regs.oldPC);
  }
}

void Dynarec::mtc2(Registers& regs, u32 instr) {
  cop2Latch = regs.gpr[RT(instr)];
}

void Dynarec::mfc2(Registers& regs, u32 instr) {
  s32 value = cop2Latch;
  regs.gpr[RT(instr)] = value;
}

void Dynarec::dmtc2(Registers& regs, u32 instr) {
  cop2Latch = regs.gpr[RT(instr)];
}

void Dynarec::dmfc2(Registers& regs, u32 instr) {
  regs.gpr[RT(instr)] = cop2Latch;
}

void Dynarec::ctc2(Registers& regs, u32) {

}

void Dynarec::cfc2(Registers& regs, u32) {

}

}