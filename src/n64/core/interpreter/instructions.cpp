#include <n64/core/Interpreter.hpp>
#include <util.hpp>

#define se_imm(x) ((s16)((x) & 0xFFFF))
#define check_address_error(mask, addr) (((!regs.cop0.is_64bit_addressing) && (s32)(addr) != (addr)) || (((addr) & (mask)) != 0))
#define check_signed_overflow(op1, op2, res) (((~((op1) ^ (op2)) & ((op1) ^ (res))) >> ((sizeof(res) * 8) - 1)) & 1)
#define check_signed_underflow(op1, op2, res) (((((op1) ^ (op2)) & ((op1) ^ (res))) >> ((sizeof(res) * 8) - 1)) & 1)

namespace n64 {
void Interpreter::add(u32 instr) {
  u32 rs = (s32)regs.gpr[RS(instr)];
  u32 rt = (s32)regs.gpr[RT(instr)];
  u32 result = rs + rt;
  if(check_signed_overflow(rs, rt, result)) {
    FireException(regs, ExceptionCode::Overflow, 0, regs.oldPC);
  } else {
    regs.gpr[RD(instr)] = s32(result);
  }
}

void Interpreter::addu(u32 instr) {
  s32 rs = (s32)regs.gpr[RS(instr)];
  s32 rt = (s32)regs.gpr[RT(instr)];
  s32 result = rs + rt;
  regs.gpr[RD(instr)] = result;
}

void Interpreter::addi(u32 instr) {
  u32 rs = regs.gpr[RS(instr)];
  u32 imm = s32(s16(instr));
  u32 result = rs + imm;
  if(check_signed_overflow(rs, imm, result)) {
    FireException(regs, ExceptionCode::Overflow, 0, regs.oldPC);
  } else {
    regs.gpr[RT(instr)] = s32(result);
  }
}

void Interpreter::addiu(u32 instr) {
  s32 rs = (s32)regs.gpr[RS(instr)];
  s16 imm = (s16)(instr);
  s32 result = rs + imm;
  regs.gpr[RT(instr)] = result;
}

void Interpreter::dadd(u32 instr) {
  u64 rs = regs.gpr[RS(instr)];
  u64 rt = regs.gpr[RT(instr)];
  u64 result = rt + rs;
  if(check_signed_overflow(rs, rt, result)) {
    FireException(regs, ExceptionCode::Overflow, 0, regs.oldPC);
  } else {
    regs.gpr[RD(instr)] = result;
  }
}

void Interpreter::daddu(u32 instr) {
  s64 rs = regs.gpr[RS(instr)];
  s64 rt = regs.gpr[RT(instr)];
  regs.gpr[RD(instr)] = rs + rt;
}

void Interpreter::daddi(u32 instr) {
  u64 imm = s64(s16(instr));
  u64 rs = regs.gpr[RS(instr)];
  u64 result = imm + rs;
  if(check_signed_overflow(rs, imm, result)) {
    FireException(regs, ExceptionCode::Overflow, 0, regs.oldPC);
  } else {
    regs.gpr[RT(instr)] = result;
  }
}

void Interpreter::daddiu(u32 instr) {
  s16 imm = (s16)(instr);
  s64 rs = regs.gpr[RS(instr)];
  regs.gpr[RT(instr)] = rs + imm;
}

void Interpreter::div(u32 instr) {
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

void Interpreter::divu(u32 instr) {
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

void Interpreter::ddiv(u32 instr) {
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

void Interpreter::ddivu(u32 instr) {
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

void Interpreter::branch(bool cond, s64 address) {
  regs.delaySlot = true;
  if (cond) {
    regs.nextPC = address;
  }
}

void Interpreter::branch_likely(bool cond, s64 address) {
  regs.delaySlot = true;
  if (cond) {
    regs.nextPC = address;
  } else {
    regs.SetPC(regs.nextPC);
  }
}

void Interpreter::b(u32 instr, bool cond) {
  s64 offset = (s64)se_imm(instr) << 2;
  s64 address = regs.pc + offset;
  branch(cond, address);
}

void Interpreter::blink(u32 instr, bool cond) {
  regs.gpr[31] = regs.nextPC;
  s64 offset = (s64)se_imm(instr) << 2;
  s64 address = regs.pc + offset;
  branch(cond, address);
}

void Interpreter::bl(u32 instr, bool cond) {
  s64 offset = (s64)se_imm(instr) << 2;
  s64 address = regs.pc + offset;
  branch_likely(cond, address);
}

void Interpreter::bllink(u32 instr, bool cond) {
  regs.gpr[31] = regs.nextPC;
  s64 offset = (s64)se_imm(instr) << 2;
  s64 address = regs.pc + offset;
  branch_likely(cond, address);
}

void Interpreter::lui(u32 instr) {
  s64 val = (s16)instr;
  val <<= 16;
  regs.gpr[RT(instr)] = val;
}

void Interpreter::lb(Mem& mem, u32 instr) {
  u64 address = regs.gpr[RS(instr)] + (s16)instr;
  regs.gpr[RT(instr)] = (s8)mem.Read8(regs, address, regs.oldPC);
}

void Interpreter::lh(Mem& mem, u32 instr) {
  u64 address = regs.gpr[RS(instr)] + (s16)instr;
  if (check_address_error(address, 0b1)) {
    HandleTLBException(regs, address);
    FireException(regs, ExceptionCode::AddressErrorLoad, 0, regs.oldPC);
    return;
  }

  regs.gpr[RT(instr)] = (s16)mem.Read16(regs, address, regs.oldPC);
}

void Interpreter::lw(Mem& mem, u32 instr) {
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

void Interpreter::ll(Mem& mem, u32 instr) {
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

void Interpreter::lwl(Mem& mem, u32 instr) {
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

void Interpreter::lwr(Mem& mem, u32 instr) {
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

void Interpreter::ld(Mem& mem, u32 instr) {
  s64 address = regs.gpr[RS(instr)] + (s16)instr;
  if (check_address_error(address, 0b111)) {
    HandleTLBException(regs, address);
    FireException(regs, ExceptionCode::AddressErrorLoad, 0, regs.oldPC);
    return;
  }

  s64 value = mem.Read64(regs, address, regs.oldPC);
  regs.gpr[RT(instr)] = value;
}

void Interpreter::lld(Mem& mem, u32 instr) {
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

void Interpreter::ldl(Mem& mem, u32 instr) {
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

void Interpreter::ldr(Mem& mem, u32 instr) {
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

void Interpreter::lbu(Mem& mem, u32 instr) {
  u64 address = regs.gpr[RS(instr)] + (s16)instr;
  u8 value = mem.Read8(regs, address, regs.oldPC);
  regs.gpr[RT(instr)] = value;
}

void Interpreter::lhu(Mem& mem, u32 instr) {
  s64 address = regs.gpr[RS(instr)] + (s16)instr;
  if (check_address_error(address, 0b1)) {
    HandleTLBException(regs, address);
    FireException(regs, ExceptionCode::AddressErrorLoad, 0, regs.oldPC);
    return;
  }

  u16 value = mem.Read16(regs, address, regs.oldPC);
  regs.gpr[RT(instr)] = value;
}

void Interpreter::lwu(Mem& mem, u32 instr) {
  s64 address = regs.gpr[RS(instr)] + (s16)instr;
  if (check_address_error(address, 0b11)) {
    HandleTLBException(regs, address);
    FireException(regs, ExceptionCode::AddressErrorLoad, 0, regs.oldPC);
    return;
  }

  u32 value = mem.Read32(regs, address, regs.oldPC);
  regs.gpr[RT(instr)] = value;
}

void Interpreter::sb(Mem& mem, u32 instr) {
  u32 address = regs.gpr[RS(instr)] + (s16)instr;
  mem.Write8(regs, address, regs.gpr[RT(instr)], regs.oldPC);
}

void Interpreter::sc(Mem& mem, u32 instr) {
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

void Interpreter::scd(Mem& mem, u32 instr) {
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

void Interpreter::sh(Mem& mem, u32 instr) {
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

void Interpreter::sw(Mem& mem, u32 instr) {
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

void Interpreter::sd(Mem& mem, u32 instr) {
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

void Interpreter::sdl(Mem& mem, u32 instr) {
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

void Interpreter::sdr(Mem& mem, u32 instr) {
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

void Interpreter::swl(Mem& mem, u32 instr) {
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

void Interpreter::swr(Mem& mem, u32 instr) {
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

void Interpreter::ori(u32 instr) {
  s64 imm = (u16)instr;
  s64 result = imm | regs.gpr[RS(instr)];
  regs.gpr[RT(instr)] = result;
}

void Interpreter::or_(u32 instr) {
  regs.gpr[RD(instr)] = regs.gpr[RS(instr)] | regs.gpr[RT(instr)];
}

void Interpreter::nor(u32 instr) {
  regs.gpr[RD(instr)] = ~(regs.gpr[RS(instr)] | regs.gpr[RT(instr)]);
}

void Interpreter::j(u32 instr) {
  s32 target = (instr & 0x3ffffff) << 2;
  s64 address = (regs.oldPC & ~0xfffffff) | target;
  if (check_address_error(address, 0b11)) {
    HandleTLBException(regs, address);
    FireException(regs, ExceptionCode::DataBusError, 0, regs.oldPC);
  }

  branch(true, address);
}

void Interpreter::jal(u32 instr) {
  regs.gpr[31] = regs.nextPC;
  j(instr);
}

void Interpreter::jalr(u32 instr) {
  branch(true, regs.gpr[RS(instr)]);
  regs.gpr[RD(instr)] = regs.pc + 4;
}

void Interpreter::slti(u32 instr) {
  regs.gpr[RT(instr)] = regs.gpr[RS(instr)] < se_imm(instr);
}

void Interpreter::sltiu(u32 instr) {
  regs.gpr[RT(instr)] = (u64)regs.gpr[RS(instr)] < se_imm(instr);
}

void Interpreter::slt(u32 instr) {
  regs.gpr[RD(instr)] = regs.gpr[RS(instr)] < regs.gpr[RT(instr)];
}

void Interpreter::sltu(u32 instr) {
  regs.gpr[RD(instr)] = (u64)regs.gpr[RS(instr)] < (u64)regs.gpr[RT(instr)];
}

void Interpreter::xori(u32 instr) {
  s64 imm = (u16)instr;
  regs.gpr[RT(instr)] = regs.gpr[RS(instr)] ^ imm;
}

void Interpreter::xor_(u32 instr) {
  regs.gpr[RD(instr)] = regs.gpr[RT(instr)] ^ regs.gpr[RS(instr)];
}

void Interpreter::andi(u32 instr) {
  s64 imm = (u16)instr;
  regs.gpr[RT(instr)] = regs.gpr[RS(instr)] & imm;
}

void Interpreter::and_(u32 instr) {
  regs.gpr[RD(instr)] = regs.gpr[RS(instr)] & regs.gpr[RT(instr)];
}

void Interpreter::sll(u32 instr) {
  u8 sa = ((instr >> 6) & 0x1f);
  s32 result = regs.gpr[RT(instr)] << sa;
  regs.gpr[RD(instr)] = (s64)result;
}

void Interpreter::sllv(u32 instr) {
  u8 sa = (regs.gpr[RS(instr)]) & 0x1F;
  u32 rt = regs.gpr[RT(instr)];
  s32 result = rt << sa;
  regs.gpr[RD(instr)] = (s64)result;
}

void Interpreter::dsll32(u32 instr) {
  u8 sa = ((instr >> 6) & 0x1f);
  s64 result = regs.gpr[RT(instr)] << (sa + 32);
  regs.gpr[RD(instr)] = result;
}

void Interpreter::dsll(u32 instr) {
  u8 sa = ((instr >> 6) & 0x1f);
  s64 result = regs.gpr[RT(instr)] << sa;
  regs.gpr[RD(instr)] = result;
}

void Interpreter::dsllv(u32 instr) {
  s64 sa = regs.gpr[RS(instr)] & 63;
  s64 result = regs.gpr[RT(instr)] << sa;
  regs.gpr[RD(instr)] = result;
}

void Interpreter::srl(u32 instr) {
  u32 rt = regs.gpr[RT(instr)];
  u8 sa = ((instr >> 6) & 0x1f);
  u32 result = rt >> sa;
  regs.gpr[RD(instr)] = (s32)result;
}

void Interpreter::srlv(u32 instr) {
  u8 sa = (regs.gpr[RS(instr)] & 0x1F);
  u32 rt = regs.gpr[RT(instr)];
  s32 result = rt >> sa;
  regs.gpr[RD(instr)] = (s64)result;
}

void Interpreter::dsrl(u32 instr) {
  u64 rt = regs.gpr[RT(instr)];
  u8 sa = ((instr >> 6) & 0x1f);
  u64 result = rt >> sa;
  regs.gpr[RD(instr)] = s64(result);
}

void Interpreter::dsrlv(u32 instr) {
  u8 amount = (regs.gpr[RS(instr)] & 63);
  u64 rt = regs.gpr[RT(instr)];
  u64 result = rt >> amount;
  regs.gpr[RD(instr)] = s64(result);
}

void Interpreter::dsrl32(u32 instr) {
  u64 rt = regs.gpr[RT(instr)];
  u8 sa = ((instr >> 6) & 0x1f);
  u64 result = rt >> (sa + 32);
  regs.gpr[RD(instr)] = s64(result);
}

void Interpreter::sra(u32 instr) {
  s64 rt = regs.gpr[RT(instr)];
  u8 sa = ((instr >> 6) & 0x1f);
  s32 result = rt >> sa;
  regs.gpr[RD(instr)] = result;
}

void Interpreter::srav(u32 instr) {
  s64 rt = regs.gpr[RT(instr)];
  s64 rs = regs.gpr[RS(instr)];
  u8 sa = rs & 0x1f;
  s32 result = rt >> sa;
  regs.gpr[RD(instr)] = result;
}

void Interpreter::dsra(u32 instr) {
  s64 rt = regs.gpr[RT(instr)];
  u8 sa = ((instr >> 6) & 0x1f);
  s64 result = rt >> sa;
  regs.gpr[RD(instr)] = result;
}

void Interpreter::dsrav(u32 instr) {
  s64 rt = regs.gpr[RT(instr)];
  s64 rs = regs.gpr[RS(instr)];
  s64 sa = rs & 63;
  s64 result = rt >> sa;
  regs.gpr[RD(instr)] = result;
}

void Interpreter::dsra32(u32 instr) {
  s64 rt = regs.gpr[RT(instr)];
  u8 sa = ((instr >> 6) & 0x1f);
  s64 result = rt >> (sa + 32);
  regs.gpr[RD(instr)] = result;
}

void Interpreter::jr(u32 instr) {
  s64 address = regs.gpr[RS(instr)];
  if (check_address_error(address, 0b11)) {
    HandleTLBException(regs, address);
    FireException(regs, ExceptionCode::DataBusError, 0, regs.oldPC);
  }

  branch(true, address);
}

void Interpreter::dsub(u32 instr) {
  s64 rt = regs.gpr[RT(instr)];
  s64 rs = regs.gpr[RS(instr)];
  s64 result = rs - rt;
  if(check_signed_underflow(rs, rt, result)) {
    FireException(regs, ExceptionCode::Overflow, 0, regs.oldPC);
  } else {
    regs.gpr[RD(instr)] = result;
  }
}

void Interpreter::dsubu(u32 instr) {
  u64 rt = regs.gpr[RT(instr)];
  u64 rs = regs.gpr[RS(instr)];
  u64 result = rs - rt;
  regs.gpr[RD(instr)] = s64(result);
}

void Interpreter::sub(u32 instr) {
  s32 rt = regs.gpr[RT(instr)];
  s32 rs = regs.gpr[RS(instr)];
  s32 result = rs - rt;
  if(check_signed_underflow(rs, rt, result)) {
    FireException(regs, ExceptionCode::Overflow, 0, regs.oldPC);
  } else {
    regs.gpr[RD(instr)] = result;
  }
}

void Interpreter::subu(u32 instr) {
  u32 rt = regs.gpr[RT(instr)];
  u32 rs = regs.gpr[RS(instr)];
  u32 result = rs - rt;
  regs.gpr[RD(instr)] = (s64)((s32)result);
}

void Interpreter::dmultu(u32 instr) {
  u64 rt = regs.gpr[RT(instr)];
  u64 rs = regs.gpr[RS(instr)];
  u128 result = (u128)rt * (u128)rs;
  regs.lo = (s64)(result & 0xFFFFFFFFFFFFFFFF);
  regs.hi = (s64)(result >> 64);
}

void Interpreter::dmult(u32 instr) {
  s64 rt = regs.gpr[RT(instr)];
  s64 rs = regs.gpr[RS(instr)];
  s128 result = (s128)rt * (s128)rs;
  regs.lo = result & 0xFFFFFFFFFFFFFFFF;
  regs.hi = result >> 64;
}

void Interpreter::multu(u32 instr) {
  u32 rt = regs.gpr[RT(instr)];
  u32 rs = regs.gpr[RS(instr)];
  u64 result = (u64)rt * (u64)rs;
  regs.lo = (s64)((s32)result);
  regs.hi = (s64)((s32)(result >> 32));
}

void Interpreter::mult(u32 instr) {
  s32 rt = regs.gpr[RT(instr)];
  s32 rs = regs.gpr[RS(instr)];
  s64 result = (s64)rt * (s64)rs;
  regs.lo = (s64)((s32)result);
  regs.hi = (s64)((s32)(result >> 32));
}

void Interpreter::mflo(u32 instr) {
  regs.gpr[RD(instr)] = regs.lo;
}

void Interpreter::mfhi(u32 instr) {
  regs.gpr[RD(instr)] = regs.hi;
}

void Interpreter::mtlo(u32 instr) {
  regs.lo = regs.gpr[RS(instr)];
}

void Interpreter::mthi(u32 instr) {
  regs.hi = regs.gpr[RS(instr)];
}

void Interpreter::trap(bool cond) {
  if(cond) {
    FireException(regs, ExceptionCode::Trap, 0, regs.oldPC);
  }
}

void Cpu::mtc2(u32 instr) {
  cop2Latch = regs.gpr[RT(instr)];
}

void Cpu::mfc2(u32 instr) {
  s32 value = cop2Latch;
  regs.gpr[RT(instr)] = value;
}

void Cpu::dmtc2(u32 instr) {
  cop2Latch = regs.gpr[RT(instr)];
}

void Cpu::dmfc2(u32 instr) {
  regs.gpr[RT(instr)] = cop2Latch;
}

void Cpu::ctc2(u32) {

}

void Cpu::cfc2(u32) {

}

}