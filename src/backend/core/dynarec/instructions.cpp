#include <dynarec/instructions.hpp>
#include <Registers.hpp>

#define se_imm(x) ((s16)((x) & 0xFFFF))
#define check_address_error(mask, addr) (((!regs.cop0.is_64bit_addressing) && (s32)(addr) != (addr)) || (((addr) & (mask)) != 0))
#define check_signed_overflow(op1, op2, res) (((~((op1) ^ (op2)) & ((op1) ^ (res))) >> ((sizeof(res) * 8) - 1)) & 1)
#define check_signed_underflow(op1, op2, res) (((((op1) ^ (op2)) & ((op1) ^ (res))) >> ((sizeof(res) * 8) - 1)) & 1)

namespace n64::JIT {
void add(Registers& regs, u32 instr) {
  u32 rs = (s32)regs.gpr[RS(instr)];
  u32 rt = (s32)regs.gpr[RT(instr)];
  u32 result = rs + rt;
  if(check_signed_overflow(rs, rt, result)) {
    FireException(regs, ExceptionCode::Overflow, 0, true);
  } else {
    regs.gpr[RD(instr)] = s32(result);
  }
}

void addu(Registers& regs, u32 instr) {
  s32 rs = (s32)regs.gpr[RS(instr)];
  s32 rt = (s32)regs.gpr[RT(instr)];
  s32 result = rs + rt;
  regs.gpr[RD(instr)] = result;
}

void addi(Registers& regs, u32 instr) {
  u32 rs = regs.gpr[RS(instr)];
  u32 imm = s32(s16(instr));
  u32 result = rs + imm;
  if(check_signed_overflow(rs, imm, result)) {
    FireException(regs, ExceptionCode::Overflow, 0, true);
  } else {
    regs.gpr[RT(instr)] = s32(result);
  }
}

void addiu(Registers& regs, u32 instr) {
  s32 rs = (s32)regs.gpr[RS(instr)];
  s16 imm = (s16)(instr);
  s32 result = rs + imm;
  regs.gpr[RT(instr)] = result;
  Util::print("addiu {:08X}, {:04X} = {:08X} [reg: {:016X}]\n", (u32)rs, imm, (u32)result, (u64)regs.gpr[RT(instr)]);
}

void dadd(Registers& regs, u32 instr) {
  u64 rs = regs.gpr[RS(instr)];
  u64 rt = regs.gpr[RT(instr)];
  u64 result = rt + rs;
  if(check_signed_overflow(rs, rt, result)) {
    FireException(regs, ExceptionCode::Overflow, 0, true);
  } else {
    regs.gpr[RD(instr)] = result;
  }
}

void daddu(Registers& regs, u32 instr) {
  s64 rs = regs.gpr[RS(instr)];
  s64 rt = regs.gpr[RT(instr)];
  regs.gpr[RD(instr)] = rs + rt;
}

void daddi(Registers& regs, u32 instr) {
  u64 imm = s64(s16(instr));
  u64 rs = regs.gpr[RS(instr)];
  u64 result = imm + rs;
  if(check_signed_overflow(rs, imm, result)) {
    FireException(regs, ExceptionCode::Overflow, 0, true);
  } else {
    regs.gpr[RT(instr)] = result;
  }
}

void daddiu(Registers& regs, u32 instr) {
  s16 imm = (s16)(instr);
  s64 rs = regs.gpr[RS(instr)];
  regs.gpr[RT(instr)] = rs + imm;
}

void div(Registers& regs, u32 instr) {
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

void divu(Registers& regs, u32 instr) {
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

void ddiv(Registers& regs, u32 instr) {
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

void ddivu(Registers& regs, u32 instr) {
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

void branch(Registers& regs, bool cond, s64 address) {
  //Util::debug("\t\tJIT branch from {:08X} -> {:08X}\n", (u32)regs.oldPC, (u32)address);
  regs.delaySlot = true;
  if (cond) {
    regs.nextPC = address;
  }
}

void branch_likely(Registers& regs, bool cond, s64 address) {
  regs.delaySlot = true;
  if (cond) {
    //Util::debug("\t\tJIT branch likely taken from {:08X} -> {:08X}\n", (u32)regs.oldPC, (u32)address);
    regs.nextPC = address;
  } else {
    //Util::debug("\t\tJIT branch likely not taken from {:08X} -> {:08X}\n", (u32)regs.oldPC, (u32)address);
    regs.SetPC(regs.nextPC);
  }
}

void b(Registers& regs, u32 instr, bool cond) {
  s64 offset = (s64)se_imm(instr) << 2;
  s64 address = regs.pc + offset;
  branch(regs, cond, address);
}

void blink(Registers& regs, u32 instr, bool cond) {
  regs.gpr[31] = regs.nextPC;
  s64 offset = (s64)se_imm(instr) << 2;
  s64 address = regs.pc + offset;
  branch(regs, cond, address);
}

void bl(Registers& regs, u32 instr, bool cond) {
  s64 offset = (s64)se_imm(instr) << 2;
  s64 address = regs.pc + offset;
  branch_likely(regs, cond, address);
}

void bllink(Registers& regs, u32 instr, bool cond) {
  regs.gpr[31] = regs.nextPC;
  s64 offset = (s64)se_imm(instr) << 2;
  s64 address = regs.pc + offset;
  branch_likely(regs, cond, address);
}

void lui(Registers& regs, u32 instr) {
  s64 val = (s16)instr;
  val <<= 16;
  regs.gpr[RT(instr)] = val;
}

void lb(Registers& regs, Mem& mem, u32 instr) {
  u64 address = regs.gpr[RS(instr)] + (s16)instr;
  regs.gpr[RT(instr)] = (s8)mem.Read8(regs, address, regs.oldPC);
}

void lh(Registers& regs, Mem& mem, u32 instr) {
  u64 address = regs.gpr[RS(instr)] + (s16)instr;
  if (check_address_error(address, 0b1)) {
    HandleTLBException(regs, address);
    FireException(regs, ExceptionCode::AddressErrorLoad, 0, true);
    return;
  }

  regs.gpr[RT(instr)] = (s16)mem.Read16(regs, address, regs.oldPC);
}

void lw(Registers& regs, Mem& mem, u32 instr) {
  s16 offset = instr;
  u64 address = regs.gpr[RS(instr)] + offset;
  if (check_address_error(address, 0b11)) {
    HandleTLBException(regs, address);
    FireException(regs, ExceptionCode::AddressErrorLoad, 0, true);
    return;
  }

  u32 physical;
  if (!MapVAddr(regs, LOAD, address, physical)) {
    HandleTLBException(regs, address);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, LOAD), 0, true);
  } else {
    regs.gpr[RT(instr)] = (s32)mem.Read32<false>(regs, physical, regs.oldPC);
  }
}

void ll(Registers& regs, Mem& mem, u32 instr) {
  s64 address = regs.gpr[RS(instr)] + (s16)instr;
  u32 physical;
  if (!MapVAddr(regs, LOAD, address, physical)) {
    HandleTLBException(regs, address);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, LOAD), 0, true);
  } else {
    regs.gpr[RT(instr)] = (s32)mem.Read32<false>(regs, physical, regs.oldPC);
  }

  regs.cop0.llbit = true;
  regs.cop0.LLAddr = physical >> 4;
}

void lwl(Registers& regs, Mem& mem, u32 instr) {
  u64 address = regs.gpr[RS(instr)] + (s16)instr;
  u32 paddr = 0;
  if(!MapVAddr(regs, LOAD, address, paddr)) {
    HandleTLBException(regs, address);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, LOAD), 0, true);
  } else {
    u32 shift = 8 * ((address ^ 0) & 3);
    u32 mask = 0xFFFFFFFF << shift;
    u32 data = mem.Read32<false>(regs, paddr & ~3, regs.oldPC);
    s32 result = s32((regs.gpr[RT(instr)] & ~mask) | (data << shift));
    regs.gpr[RT(instr)] = result;
  }
}

void lwr(Registers& regs, Mem& mem, u32 instr) {
  u64 address = regs.gpr[RS(instr)] + (s16)instr;
  u32 paddr = 0;
  if(!MapVAddr(regs, LOAD, address, paddr)) {
    HandleTLBException(regs, address);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, LOAD), 0, true);
  } else {
    u32 shift = 8 * ((address ^ 3) & 3);
    u32 mask = 0xFFFFFFFF >> shift;
    u32 data = mem.Read32<false>(regs, paddr & ~3, regs.oldPC);
    s32 result = s32((regs.gpr[RT(instr)] & ~mask) | (data >> shift));
    regs.gpr[RT(instr)] = result;
  }
}

void ld(Registers& regs, Mem& mem, u32 instr) {
  s64 address = regs.gpr[RS(instr)] + (s16)instr;
  if (check_address_error(address, 0b111)) {
    HandleTLBException(regs, address);
    FireException(regs, ExceptionCode::AddressErrorLoad, 0, true);
    return;
  }

  s64 value = mem.Read64(regs, address, regs.oldPC);
  regs.gpr[RT(instr)] = value;
}

void lld(Registers& regs, Mem& mem, u32 instr) {
  s64 address = regs.gpr[RS(instr)] + (s16)instr;
  u32 paddr;
  if (!MapVAddr(regs, LOAD, address, paddr)) {
    HandleTLBException(regs, address);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, LOAD), 0, true);
  } else {
    regs.gpr[RT(instr)] = mem.Read64<false>(regs, paddr, regs.oldPC);
  }

  regs.cop0.llbit = true;
  regs.cop0.LLAddr = paddr >> 4;
}

void ldl(Registers& regs, Mem& mem, u32 instr) {
  u64 address = regs.gpr[RS(instr)] + (s16)instr;
  u32 paddr = 0;
  if (!MapVAddr(regs, LOAD, address, paddr)) {
    HandleTLBException(regs, address);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, LOAD), 0, true);
  } else {
    s32 shift = 8 * ((address ^ 0) & 7);
    u64 mask = 0xFFFFFFFFFFFFFFFF << shift;
    u64 data = mem.Read64<false>(regs, paddr & ~7, regs.oldPC);
    s64 result = (s64) ((regs.gpr[RT(instr)] & ~mask) | (data << shift));
    regs.gpr[RT(instr)] = result;
  }
}

void ldr(Registers& regs, Mem& mem, u32 instr) {
  u64 address = regs.gpr[RS(instr)] + (s16)instr;
  u32 paddr;
  if (!MapVAddr(regs, LOAD, address, paddr)) {
    HandleTLBException(regs, address);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, LOAD), 0, true);
  } else {
    s32 shift = 8 * ((address ^ 7) & 7);
    u64 mask = 0xFFFFFFFFFFFFFFFF >> shift;
    u64 data = mem.Read64<false>(regs, paddr & ~7, regs.oldPC);
    s64 result = (s64) ((regs.gpr[RT(instr)] & ~mask) | (data >> shift));
    regs.gpr[RT(instr)] = result;
  }
}

void lbu(Registers& regs, Mem& mem, u32 instr) {
  u64 address = regs.gpr[RS(instr)] + (s16)instr;
  u8 value = mem.Read8(regs, address, regs.oldPC);
  regs.gpr[RT(instr)] = value;
}

void lhu(Registers& regs, Mem& mem, u32 instr) {
  s64 address = regs.gpr[RS(instr)] + (s16)instr;
  if (check_address_error(address, 0b1)) {
    HandleTLBException(regs, address);
    FireException(regs, ExceptionCode::AddressErrorLoad, 0, true);
    return;
  }

  u16 value = mem.Read16(regs, address, regs.oldPC);
  regs.gpr[RT(instr)] = value;
}

void lwu(Registers& regs, Mem& mem, u32 instr) {
  s64 address = regs.gpr[RS(instr)] + (s16)instr;
  if (check_address_error(address, 0b11)) {
    HandleTLBException(regs, address);
    FireException(regs, ExceptionCode::AddressErrorLoad, 0, true);
    return;
  }

  u32 value = mem.Read32(regs, address, regs.oldPC);
  regs.gpr[RT(instr)] = value;
}

void sb(Registers& regs, Mem& mem, Dynarec& dyn, u32 instr) {
  u32 address = regs.gpr[RS(instr)] + (s16)instr;
  mem.Write8(regs, dyn, address, regs.gpr[RT(instr)], regs.oldPC);
}

void sc(Registers& regs, Mem& mem, Dynarec& dyn, u32 instr) {
  s64 address = regs.gpr[RS(instr)] + (s16)instr;
  if (check_address_error(address, 0b11)) {
    HandleTLBException(regs, address);
    FireException(regs, ExceptionCode::AddressErrorStore, 0, true);
  }

  if(regs.cop0.llbit) {
    mem.Write32(regs, dyn, address, regs.gpr[RT(instr)], regs.oldPC);
  }

  regs.gpr[RT(instr)] = (u64)regs.cop0.llbit;
  regs.cop0.llbit = false;
}

void scd(Registers& regs, Mem& mem, Dynarec& dyn, u32 instr) {
  s64 address = regs.gpr[RS(instr)] + (s16)instr;
  if (check_address_error(address, 0b111)) {
    HandleTLBException(regs, address);
    FireException(regs, ExceptionCode::AddressErrorStore, 0, true);
    return;
  }

  if(regs.cop0.llbit) {
    mem.Write64(regs, dyn, address, regs.gpr[RT(instr)], regs.oldPC);
  }

  regs.gpr[RT(instr)] = (s64)((u64)regs.cop0.llbit);
  regs.cop0.llbit = false;
}

void sh(Registers& regs, Mem& mem, Dynarec& dyn, u32 instr) {
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
    mem.Write16<false>(regs, dyn, physical, regs.gpr[RT(instr)], regs.oldPC);
  }
}

void sw(Registers& regs, Mem& mem, Dynarec& dyn, u32 instr) {
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
    mem.Write32<false>(regs, dyn, physical, regs.gpr[RT(instr)], regs.oldPC);
  }
}

void sd(Registers& regs, Mem& mem, Dynarec& dyn, u32 instr) {
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
    mem.Write64<false>(regs, dyn, physical, regs.gpr[RT(instr)], regs.oldPC);
  }

}

void sdl(Registers& regs, Mem& mem, Dynarec& dyn, u32 instr) {
  u64 address = regs.gpr[RS(instr)] + (s16)instr;
  u32 paddr;
  if (!MapVAddr(regs, STORE, address, paddr)) {
    HandleTLBException(regs, address);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, STORE), 0, true);
  } else {
    s32 shift = 8 * ((address ^ 0) & 7);
    u64 mask = 0xFFFFFFFFFFFFFFFF >> shift;
    u64 data = mem.Read64<false>(regs, paddr & ~7, regs.oldPC);
    u64 rt = regs.gpr[RT(instr)];
    mem.Write64<false>(regs, dyn, paddr & ~7, (data & ~mask) | (rt >> shift), regs.oldPC);
  }
}

void sdr(Registers& regs, Mem& mem, Dynarec& dyn, u32 instr) {
  u64 address = regs.gpr[RS(instr)] + (s16)instr;
  u32 paddr;
  if (!MapVAddr(regs, STORE, address, paddr)) {
    HandleTLBException(regs, address);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, STORE), 0, true);
  } else {
    s32 shift = 8 * ((address ^ 7) & 7);
    u64 mask = 0xFFFFFFFFFFFFFFFF << shift;
    u64 data = mem.Read64<false>(regs, paddr & ~7, regs.oldPC);
    u64 rt = regs.gpr[RT(instr)];
    mem.Write64<false>(regs, dyn, paddr & ~7, (data & ~mask) | (rt << shift), regs.oldPC);
  }
}

void swl(Registers& regs, Mem& mem, Dynarec& dyn, u32 instr) {
  u64 address = regs.gpr[RS(instr)] + (s16)instr;
  u32 paddr;
  if (!MapVAddr(regs, STORE, address, paddr)) {
    HandleTLBException(regs, address);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, STORE), 0, true);
  } else {
    u32 shift = 8 * ((address ^ 0) & 3);
    u32 mask = 0xFFFFFFFF >> shift;
    u32 data = mem.Read32<false>(regs, paddr & ~3, regs.oldPC);
    u32 rt = regs.gpr[RT(instr)];
    mem.Write32<false>(regs, dyn, paddr & ~3, (data & ~mask) | (rt >> shift), regs.oldPC);
  }
}

void swr(Registers& regs, Mem& mem, Dynarec& dyn, u32 instr) {
  u64 address = regs.gpr[RS(instr)] + (s16)instr;
  u32 paddr;
  if (!MapVAddr(regs, STORE, address, paddr)) {
    HandleTLBException(regs, address);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, STORE), 0, true);
  } else {
    u32 shift = 8 * ((address ^ 3) & 3);
    u32 mask = 0xFFFFFFFF << shift;
    u32 data = mem.Read32<false>(regs, paddr & ~3, regs.oldPC);
    u32 rt = regs.gpr[RT(instr)];
    mem.Write32<false>(regs, dyn, paddr & ~3, (data & ~mask) | (rt << shift), regs.oldPC);
  }
}

void ori(Registers& regs, u32 instr) {
  s64 imm = (u16)instr;
  s64 result = imm | regs.gpr[RS(instr)];
  regs.gpr[RT(instr)] = result;
}

void or_(Registers& regs, u32 instr) {
  regs.gpr[RD(instr)] = regs.gpr[RS(instr)] | regs.gpr[RT(instr)];
}

void nor(Registers& regs, u32 instr) {
  regs.gpr[RD(instr)] = ~(regs.gpr[RS(instr)] | regs.gpr[RT(instr)]);
}

void j(Registers& regs, u32 instr) {
  s32 target = (instr & 0x3ffffff) << 2;
  s64 address = (regs.oldPC & ~0xfffffff) | target;
  if (check_address_error(address, 0b11)) {
    HandleTLBException(regs, address);
    FireException(regs, ExceptionCode::DataBusError, 0, true);
  }

  branch(regs, true, address);
}

void jal(Registers& regs, u32 instr) {
  regs.gpr[31] = regs.nextPC;
  j(regs, instr);
}

void jalr(Registers& regs, u32 instr) {
  branch(regs, true, regs.gpr[RS(instr)]);
  regs.gpr[RD(instr)] = regs.pc + 4;
}

void slti(Registers& regs, u32 instr) {
  regs.gpr[RT(instr)] = regs.gpr[RS(instr)] < se_imm(instr);
}

void sltiu(Registers& regs, u32 instr) {
  regs.gpr[RT(instr)] = (u64)regs.gpr[RS(instr)] < se_imm(instr);
}

void slt(Registers& regs, u32 instr) {
  regs.gpr[RD(instr)] = regs.gpr[RS(instr)] < regs.gpr[RT(instr)];
}

void sltu(Registers& regs, u32 instr) {
  regs.gpr[RD(instr)] = (u64)regs.gpr[RS(instr)] < (u64)regs.gpr[RT(instr)];
}

void xori(Registers& regs, u32 instr) {
  s64 imm = (u16)instr;
  regs.gpr[RT(instr)] = regs.gpr[RS(instr)] ^ imm;
}

void xor_(Registers& regs, u32 instr) {
  regs.gpr[RD(instr)] = regs.gpr[RT(instr)] ^ regs.gpr[RS(instr)];
}

void andi(Registers& regs, u32 instr) {
  s64 imm = (u16)instr;
  regs.gpr[RT(instr)] = regs.gpr[RS(instr)] & imm;
}

void and_(Registers& regs, u32 instr) {
  regs.gpr[RD(instr)] = regs.gpr[RS(instr)] & regs.gpr[RT(instr)];
}

void sll(Registers& regs, u32 instr) {
  u8 sa = ((instr >> 6) & 0x1f);
  s32 result = regs.gpr[RT(instr)] << sa;
  regs.gpr[RD(instr)] = (s64)result;
}

void sllv(Registers& regs, u32 instr) {
  u8 sa = (regs.gpr[RS(instr)]) & 0x1F;
  u32 rt = regs.gpr[RT(instr)];
  s32 result = rt << sa;
  regs.gpr[RD(instr)] = (s64)result;
}

void dsll32(Registers& regs, u32 instr) {
  u8 sa = ((instr >> 6) & 0x1f);
  s64 result = regs.gpr[RT(instr)] << (sa + 32);
  regs.gpr[RD(instr)] = result;
}

void dsll(Registers& regs, u32 instr) {
  u8 sa = ((instr >> 6) & 0x1f);
  s64 result = regs.gpr[RT(instr)] << sa;
  regs.gpr[RD(instr)] = result;
}

void dsllv(Registers& regs, u32 instr) {
  s64 sa = regs.gpr[RS(instr)] & 63;
  s64 result = regs.gpr[RT(instr)] << sa;
  regs.gpr[RD(instr)] = result;
}

void srl(Registers& regs, u32 instr) {
  u32 rt = regs.gpr[RT(instr)];
  u8 sa = ((instr >> 6) & 0x1f);
  u32 result = rt >> sa;
  regs.gpr[RD(instr)] = (s32)result;
}

void srlv(Registers& regs, u32 instr) {
  u8 sa = (regs.gpr[RS(instr)] & 0x1F);
  u32 rt = regs.gpr[RT(instr)];
  s32 result = rt >> sa;
  regs.gpr[RD(instr)] = (s64)result;
}

void dsrl(Registers& regs, u32 instr) {
  u64 rt = regs.gpr[RT(instr)];
  u8 sa = ((instr >> 6) & 0x1f);
  u64 result = rt >> sa;
  regs.gpr[RD(instr)] = s64(result);
}

void dsrlv(Registers& regs, u32 instr) {
  u8 amount = (regs.gpr[RS(instr)] & 63);
  u64 rt = regs.gpr[RT(instr)];
  u64 result = rt >> amount;
  regs.gpr[RD(instr)] = s64(result);
}

void dsrl32(Registers& regs, u32 instr) {
  u64 rt = regs.gpr[RT(instr)];
  u8 sa = ((instr >> 6) & 0x1f);
  u64 result = rt >> (sa + 32);
  regs.gpr[RD(instr)] = s64(result);
}

void sra(Registers& regs, u32 instr) {
  s64 rt = regs.gpr[RT(instr)];
  u8 sa = ((instr >> 6) & 0x1f);
  s32 result = rt >> sa;
  regs.gpr[RD(instr)] = result;
}

void srav(Registers& regs, u32 instr) {
  s64 rt = regs.gpr[RT(instr)];
  s64 rs = regs.gpr[RS(instr)];
  u8 sa = rs & 0x1f;
  s32 result = rt >> sa;
  regs.gpr[RD(instr)] = result;
}

void dsra(Registers& regs, u32 instr) {
  s64 rt = regs.gpr[RT(instr)];
  u8 sa = ((instr >> 6) & 0x1f);
  s64 result = rt >> sa;
  regs.gpr[RD(instr)] = result;
}

void dsrav(Registers& regs, u32 instr) {
  s64 rt = regs.gpr[RT(instr)];
  s64 rs = regs.gpr[RS(instr)];
  s64 sa = rs & 63;
  s64 result = rt >> sa;
  regs.gpr[RD(instr)] = result;
}

void dsra32(Registers& regs, u32 instr) {
  s64 rt = regs.gpr[RT(instr)];
  u8 sa = ((instr >> 6) & 0x1f);
  s64 result = rt >> (sa + 32);
  regs.gpr[RD(instr)] = result;
}

void jr(Registers& regs, u32 instr) {
  s64 address = regs.gpr[RS(instr)];
  if (check_address_error(address, 0b11)) {
    HandleTLBException(regs, address);
    FireException(regs, ExceptionCode::DataBusError, 0, regs.oldPC);
  }

  branch(regs, true, address);
}

void dsub(Registers& regs, u32 instr) {
  s64 rt = regs.gpr[RT(instr)];
  s64 rs = regs.gpr[RS(instr)];
  s64 result = rs - rt;
  if(check_signed_underflow(rs, rt, result)) {
    FireException(regs, ExceptionCode::Overflow, 0, regs.oldPC);
  } else {
    regs.gpr[RD(instr)] = result;
  }
}

void dsubu(Registers& regs, u32 instr) {
  u64 rt = regs.gpr[RT(instr)];
  u64 rs = regs.gpr[RS(instr)];
  u64 result = rs - rt;
  regs.gpr[RD(instr)] = s64(result);
}

void sub(Registers& regs, u32 instr) {
  s32 rt = regs.gpr[RT(instr)];
  s32 rs = regs.gpr[RS(instr)];
  s32 result = rs - rt;
  if(check_signed_underflow(rs, rt, result)) {
    FireException(regs, ExceptionCode::Overflow, 0, regs.oldPC);
  } else {
    regs.gpr[RD(instr)] = result;
  }
}

void subu(Registers& regs, u32 instr) {
  u32 rt = regs.gpr[RT(instr)];
  u32 rs = regs.gpr[RS(instr)];
  u32 result = rs - rt;
  regs.gpr[RD(instr)] = (s64)((s32)result);
}

void dmultu(Registers& regs, u32 instr) {
  u64 rt = regs.gpr[RT(instr)];
  u64 rs = regs.gpr[RS(instr)];
  u128 result = (u128)rt * (u128)rs;
  regs.lo = (s64)(result & 0xFFFFFFFFFFFFFFFF);
  regs.hi = (s64)(result >> 64);
}

void dmult(Registers& regs, u32 instr) {
  s64 rt = regs.gpr[RT(instr)];
  s64 rs = regs.gpr[RS(instr)];
  s128 result = (s128)rt * (s128)rs;
  regs.lo = result & 0xFFFFFFFFFFFFFFFF;
  regs.hi = result >> 64;
}

void multu(Registers& regs, u32 instr) {
  u32 rt = regs.gpr[RT(instr)];
  u32 rs = regs.gpr[RS(instr)];
  u64 result = (u64)rt * (u64)rs;
  regs.lo = (s64)((s32)result);
  regs.hi = (s64)((s32)(result >> 32));
}

void mult(Registers& regs, u32 instr) {
  s32 rt = regs.gpr[RT(instr)];
  s32 rs = regs.gpr[RS(instr)];
  s64 result = (s64)rt * (s64)rs;
  regs.lo = (s64)((s32)result);
  regs.hi = (s64)((s32)(result >> 32));
}

void mflo(Registers& regs, u32 instr) {
  regs.gpr[RD(instr)] = regs.lo;
}

void mfhi(Registers& regs, u32 instr) {
  regs.gpr[RD(instr)] = regs.hi;
}

void mtlo(Registers& regs, u32 instr) {
  regs.lo = regs.gpr[RS(instr)];
}

void mthi(Registers& regs, u32 instr) {
  regs.hi = regs.gpr[RS(instr)];
}

void trap(Registers& regs, bool cond) {
  if(cond) {
    FireException(regs, ExceptionCode::Trap, 0, regs.oldPC);
  }
}

void mtc2(Dynarec& dyn, Registers& regs, u32 instr) {
  dyn.cop2Latch = regs.gpr[RT(instr)];
}

void mfc2(Dynarec& dyn, Registers& regs, u32 instr) {
  s32 value = dyn.cop2Latch;
  regs.gpr[RT(instr)] = value;
}

void dmtc2(Dynarec& dyn, Registers& regs, u32 instr) {
  dyn.cop2Latch = regs.gpr[RT(instr)];
}

void dmfc2(Dynarec& dyn, Registers& regs, u32 instr) {
  regs.gpr[RT(instr)] = dyn.cop2Latch;
}
}