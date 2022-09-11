#include <n64/core/Cpu.hpp>
#include <util.hpp>

#define se_imm(x) ((s16)((x) & 0xFFFF))

namespace n64 {

void Cpu::add(u32 instr) {
  s32 rs = (s32)regs.gpr[RS(instr)];
  s32 rt = (s32)regs.gpr[RT(instr)];
  s32 result = rs + rt;
  regs.gpr[RD(instr)] = result;
}

void Cpu::addu(u32 instr) {
  s32 rs = (s32)regs.gpr[RS(instr)];
  s32 rt = (s32)regs.gpr[RT(instr)];
  s32 result = rs + rt;
  regs.gpr[RD(instr)] = result;
}

void Cpu::addi(u32 instr) {
  s32 rs = (s32)regs.gpr[RS(instr)];
  s16 imm = (s16)(instr);
  s32 result = rs + imm;
  regs.gpr[RT(instr)] = result;
}

void Cpu::addiu(u32 instr) {
  s32 rs = (s32)regs.gpr[RS(instr)];
  s16 imm = (s16)(instr);
  s32 result = rs + imm;
  regs.gpr[RT(instr)] = result;
}

void Cpu::dadd(u32 instr) {
  s64 rs = regs.gpr[RS(instr)];
  s64 rt = regs.gpr[RT(instr)];
  regs.gpr[RD(instr)] = rs + rt;
}

void Cpu::daddu(u32 instr) {
  s64 rs = regs.gpr[RS(instr)];
  s64 rt = regs.gpr[RT(instr)];
  regs.gpr[RD(instr)] = rs + rt;
}

void Cpu::daddi(u32 instr) {
  s16 imm = (s16)(instr);
  s64 rs = regs.gpr[RS(instr)];
  regs.gpr[RT(instr)] = rs + imm;
}

void Cpu::daddiu(u32 instr) {
  s16 imm = (s16)(instr);
  s64 rs = regs.gpr[RS(instr)];
  regs.gpr[RT(instr)] = rs + imm;
}

void Cpu::div_(u32 instr) {
  s64 dividend = (s32)regs.gpr[RS(instr)];
  s64 divisor = (s32)regs.gpr[RT(instr)];

  if(divisor == 0) {
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

void Cpu::divu(u32 instr) {
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

void Cpu::ddiv(u32 instr) {
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

void Cpu::ddivu(u32 instr) {
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

void Cpu::branch(bool cond, s64 address) {
  regs.delaySlot = true;
  if (cond) {
    regs.nextPC = address;
  }
}

void Cpu::branch_likely(bool cond, s64 address) {
  regs.delaySlot = true;
  if (cond) {
    regs.nextPC = address;
  } else {
    regs.SetPC(regs.nextPC);
  }
}

void Cpu::b(u32 instr, bool cond) {
  s64 offset = (s64)se_imm(instr) << 2;
  s64 address = regs.pc + offset;
  branch(cond, address);
}

void Cpu::blink(u32 instr, bool cond) {
  regs.gpr[31] = regs.nextPC;
  s64 offset = (s64)se_imm(instr) << 2;
  s64 address = regs.pc + offset;
  branch(cond, address);
}

void Cpu::bl(u32 instr, bool cond) {
  s64 offset = (s64)se_imm(instr) << 2;
  s64 address = regs.pc + offset;
  branch_likely(cond, address);
}

void Cpu::bllink(u32 instr, bool cond) {
  regs.gpr[31] = regs.nextPC;
  s64 offset = (s64)se_imm(instr) << 2;
  s64 address = regs.pc + offset;
  branch_likely(cond, address);
}

void Cpu::lui(u32 instr) {
  s64 val = (s16)instr;
  val <<= 16;
  regs.gpr[RT(instr)] = val;
}

void Cpu::lb(Mem& mem, u32 instr) {
  u32 address = regs.gpr[RS(instr)] + (s16)instr;
  regs.gpr[RT(instr)] = mem.Read<s8>(regs, address, regs.oldPC);
}

void Cpu::lh(Mem& mem, u32 instr) {
  s64 address = regs.gpr[RS(instr)] + (s16)instr;
  if (((address & 1) != 0) || (address > 0)) {
    HandleTLBException(regs, address);
    FireException(regs, ExceptionCode::AddressErrorLoad, 0, regs.oldPC);
  }

  regs.gpr[RT(instr)] = mem.Read<s16>(regs, address, regs.oldPC);
}

void Cpu::lw(Mem& mem, u32 instr) {
  s64 address = regs.gpr[RS(instr)] + (s16)instr;
  if (((address & 3) != 0) || (address > 0)) {
    HandleTLBException(regs, address);
    FireException(regs, ExceptionCode::AddressErrorLoad, 0, regs.oldPC);
  }

  regs.gpr[RT(instr)] = mem.Read<s32>(regs, address, regs.oldPC);
}

void Cpu::ll(Mem& mem, u32 instr) {
  s64 address = regs.gpr[RS(instr)] + (s16)instr;
  if ((address & 3) != 0 || (address > 0)) {
    HandleTLBException(regs, address);
    FireException(regs, ExceptionCode::AddressErrorLoad, 0, regs.oldPC);
  }

  regs.cop0.llbit = true;
  regs.cop0.LLAddr = address;

  regs.gpr[RT(instr)] = mem.Read<s32>(regs, address, regs.oldPC);
}

void Cpu::lwl(Mem& mem, u32 instr) {
  u32 address = regs.gpr[RS(instr)] + (s16)instr;
  u32 shift = 8 * ((address ^ 0) & 3);
  u32 mask = 0xFFFFFFFF << shift;
  u32 data = mem.Read<u32>(regs, address & ~3, regs.oldPC);
  s64 rt = regs.gpr[RT(instr)];
  s32 result = (s32)((rt & ~mask) | (data << shift));
  regs.gpr[RT(instr)] = result;
}

void Cpu::lwr(Mem& mem, u32 instr) {
  u32 address = regs.gpr[RS(instr)] + (s16)instr;
  u32 shift = 8 * ((address ^ 3) & 3);
  u32 mask = 0xFFFFFFFF >> shift;
  u32 data = mem.Read<u32>(regs, address & ~3, regs.oldPC);
  s64 rt = regs.gpr[RT(instr)];
  s32 result = (s32)((rt & ~mask) | (data >> shift));
  regs.gpr[RT(instr)] = result;
}

void Cpu::ld(Mem& mem, u32 instr) {
  s64 address = regs.gpr[RS(instr)] + (s16)instr;
  if ((address & 7) != 0 || (address > 0)) {
    HandleTLBException(regs, address);
    FireException(regs, ExceptionCode::AddressErrorLoad, 0, regs.oldPC);
  }

  s64 value = mem.Read<s64>(regs, address, regs.oldPC);
  regs.gpr[RT(instr)] = value;
}

void Cpu::lld(Mem& mem, u32 instr) {
  s64 address = regs.gpr[RS(instr)] + (s16)instr;
  if ((address & 7) != 0 || (address > 0)) {
    HandleTLBException(regs, address);
    FireException(regs, ExceptionCode::AddressErrorLoad, 0, regs.oldPC);
  }

  regs.cop0.llbit = true;
  regs.cop0.LLAddr = address;

  s64 value = mem.Read<s64>(regs, address, regs.oldPC);
  regs.gpr[RT(instr)] = value;
}

void Cpu::ldl(Mem& mem, u32 instr) {
  u32 address = regs.gpr[RS(instr)] + (s16)instr;
  s32 shift = 8 * ((address ^ 0) & 7);
  u64 mask = 0xFFFFFFFFFFFFFFFF << shift;
  u64 data = mem.Read<u64>(regs, address & ~7, regs.oldPC);
  s64 rt = regs.gpr[RT(instr)];
  s64 result = (s64)((rt & ~mask) | (data << shift));
  regs.gpr[RT(instr)] = result;
}

void Cpu::ldr(Mem& mem, u32 instr) {
  u32 address = regs.gpr[RS(instr)] + (s16)instr;
  s32 shift = 8 * ((address ^ 7) & 7);
  u64 mask = 0xFFFFFFFFFFFFFFFF >> shift;
  u64 data = mem.Read<u64>(regs, address & ~7, regs.oldPC);
  s64 rt = regs.gpr[RT(instr)];
  s64 result = (s64)((rt & ~mask) | (data >> shift));
  regs.gpr[RT(instr)] = result;
}

void Cpu::lbu(Mem& mem, u32 instr) {
  u32 address = regs.gpr[RS(instr)] + (s16)instr;
  u8 value = mem.Read<u8>(regs, address, regs.oldPC);
  regs.gpr[RT(instr)] = value;
}

void Cpu::lhu(Mem& mem, u32 instr) {
  s64 address = regs.gpr[RS(instr)] + (s16)instr;
  if ((address & 1) != 0 || (address > 0)) {
    HandleTLBException(regs, address);
    FireException(regs, ExceptionCode::AddressErrorLoad, 0, regs.oldPC);
  }

  u16 value = mem.Read<u16>(regs, address, regs.oldPC);
  regs.gpr[RT(instr)] = value;
}

void Cpu::lwu(Mem& mem, u32 instr) {
  s64 address = regs.gpr[RS(instr)] + (s16)instr;
  if ((address & 3) != 0 || (address > 0)) {
    HandleTLBException(regs, address);
    FireException(regs, ExceptionCode::AddressErrorLoad, 0, regs.oldPC);
  }

  u32 value = mem.Read<u32>(regs, address, regs.oldPC);
  regs.gpr[RT(instr)] = value;
}

void Cpu::sb(Mem& mem, u32 instr) {
  u32 address = regs.gpr[RS(instr)] + (s16)instr;
  mem.Write<u8>(regs, address, regs.gpr[RT(instr)], regs.oldPC);
}

void Cpu::sc(Mem& mem, u32 instr) {
  s64 address = regs.gpr[RS(instr)] + (s16)instr;
  if ((address & 3) != 0 || (address > 0)) {
    HandleTLBException(regs, address);
    FireException(regs, ExceptionCode::AddressErrorStore, 0, regs.oldPC);
  }

  if(regs.cop0.llbit) {
    mem.Write<u32>(regs, address, regs.gpr[RT(instr)], regs.oldPC);
  }

  regs.gpr[RT(instr)] = (s64)((u64)regs.cop0.llbit);
  regs.cop0.llbit = false;
}

void Cpu::scd(Mem& mem, u32 instr) {
  s64 address = regs.gpr[RS(instr)] + (s16)instr;
  if ((address & 7) != 0 || (address > 0)) {
    HandleTLBException(regs, address);
    FireException(regs, ExceptionCode::AddressErrorStore, 0, regs.oldPC);
  }

  if(regs.cop0.llbit) {
    mem.Write<u64>(regs, address, regs.gpr[RT(instr)], regs.oldPC);
  }

  regs.gpr[RT(instr)] = (s64)((u64)regs.cop0.llbit);
  regs.cop0.llbit = false;
}

void Cpu::sh(Mem& mem, u32 instr) {
  s64 address = regs.gpr[RS(instr)] + (s16)instr;
  if ((address & 1) != 0 || (address > 0)) {
    HandleTLBException(regs, address);
    FireException(regs, ExceptionCode::AddressErrorStore, 0, regs.oldPC);
  }

  mem.Write<u16>(regs, address, regs.gpr[RT(instr)], regs.oldPC);
}

void Cpu::sw(Mem& mem, u32 instr) {
  s64 address = regs.gpr[RS(instr)] + (s16)instr;
  if ((address & 3) != 0 || (address > 0)) {
    HandleTLBException(regs, address);
    FireException(regs, ExceptionCode::AddressErrorStore, 0, regs.oldPC);
  }

  mem.Write<u32>(regs, address, regs.gpr[RT(instr)], regs.oldPC);
}

void Cpu::sd(Mem& mem, u32 instr) {
  s64 address = regs.gpr[RS(instr)] + (s16)instr;
  if ((address & 7) != 0 || (address > 0)) {
    HandleTLBException(regs, address);
    FireException(regs, ExceptionCode::AddressErrorStore, 0, regs.oldPC);
  }

  mem.Write<u64>(regs, address, regs.gpr[RT(instr)], regs.oldPC);
}

void Cpu::sdl(Mem& mem, u32 instr) {
  u32 address = regs.gpr[RS(instr)] + (s16)instr;
  s32 shift = 8 * ((address ^ 0) & 7);
  u64 mask = 0xFFFFFFFFFFFFFFFF >> shift;
  u64 data = mem.Read<u64>(regs, address & ~7, regs.oldPC);
  s64 rt = regs.gpr[RT(instr)];
  mem.Write<u64>(regs, address & ~7, (data & ~mask) | (rt >> shift), regs.oldPC);
}

void Cpu::sdr(Mem& mem, u32 instr) {
  u32 address = regs.gpr[RS(instr)] + (s16)instr;
  s32 shift = 8 * ((address ^ 7) & 7);
  u64 mask = 0xFFFFFFFFFFFFFFFF << shift;
  u64 data = mem.Read<u64>(regs, address & ~7, regs.oldPC);
  s64 rt = regs.gpr[RT(instr)];
  mem.Write<u64>(regs, address & ~7, (data & ~mask) | (rt << shift), regs.oldPC);
}

void Cpu::swl(Mem& mem, u32 instr) {
  u32 address = regs.gpr[RS(instr)] + (s16)instr;
  u32 shift = 8 * ((address ^ 0) & 3);
  u32 mask = 0xFFFFFFFF >> shift;
  u32 data = mem.Read<u32>(regs, address & ~3, regs.oldPC);
  u32 rt = regs.gpr[RT(instr)];
  mem.Write<u32>(regs, address & ~3, (data & ~mask) | (rt >> shift), regs.oldPC);
}

void Cpu::swr(Mem& mem, u32 instr) {
  u32 address = regs.gpr[RS(instr)] + (s16)instr;
  u32 shift = 8 * ((address ^ 3) & 3);
  u32 mask = 0xFFFFFFFF << shift;
  u32 data = mem.Read<u32>(regs, address & ~3, regs.oldPC);
  u32 rt = regs.gpr[RT(instr)];
  mem.Write<u32>(regs, address & ~3, (data & ~mask) | (rt << shift), regs.oldPC);
}

void Cpu::ori(u32 instr) {
  s64 imm = (u16)instr;
  s64 result = imm | regs.gpr[RS(instr)];
  regs.gpr[RT(instr)] = result;
}

void Cpu::or_(u32 instr) {
  regs.gpr[RD(instr)] = regs.gpr[RS(instr)] | regs.gpr[RT(instr)];
}

void Cpu::nor(u32 instr) {
  regs.gpr[RD(instr)] = ~(regs.gpr[RS(instr)] | regs.gpr[RT(instr)]);
}

void Cpu::j(u32 instr) {
  s32 target = (instr & 0x3ffffff) << 2;
  s64 address = (regs.oldPC & ~0xfffffff) | target;
  if ((address & 3) != 0 || (address > 0)) {
    HandleTLBException(regs, address);
    FireException(regs, ExceptionCode::DataBusError, 0, regs.oldPC);
  }

  branch(true, address);
}

void Cpu::jal(u32 instr) {
  regs.gpr[31] = regs.nextPC;
  j(instr);
}

void Cpu::jalr(u32 instr) {
  branch(true, regs.gpr[RS(instr)]);
  regs.gpr[RD(instr)] = regs.pc + 4;
}

void Cpu::slti(u32 instr) {
  regs.gpr[RT(instr)] = regs.gpr[RS(instr)] < se_imm((s64)instr);
}

void Cpu::sltiu(u32 instr) {
  regs.gpr[RT(instr)] = (u64)regs.gpr[RS(instr)] < se_imm((s64)instr);
}

void Cpu::slt(u32 instr) {
  regs.gpr[RD(instr)] = regs.gpr[RS(instr)] < regs.gpr[RT(instr)];
}

void Cpu::sltu(u32 instr) {
  regs.gpr[RD(instr)] = (u64)regs.gpr[RS(instr)] < (u64)regs.gpr[RT(instr)];
}

void Cpu::xori(u32 instr) {
  s64 imm = (u16)instr;
  regs.gpr[RT(instr)] = regs.gpr[RS(instr)] ^ imm;
}

void Cpu::xor_(u32 instr) {
  regs.gpr[RD(instr)] = regs.gpr[RT(instr)] ^ regs.gpr[RS(instr)];
}

void Cpu::andi(u32 instr) {
  s64 imm = (u16)instr;
  regs.gpr[RT(instr)] = regs.gpr[RS(instr)] & imm;
}

void Cpu::and_(u32 instr) {
  regs.gpr[RD(instr)] = regs.gpr[RS(instr)] & regs.gpr[RT(instr)];
}

void Cpu::sll(u32 instr) {
  u8 sa = ((instr >> 6) & 0x1f);
  s32 result = regs.gpr[RT(instr)] << sa;
  regs.gpr[RD(instr)] = (s64)result;
}

void Cpu::sllv(u32 instr) {
  u8 sa = (regs.gpr[RS(instr)]) & 0x1F;
  u32 rt = regs.gpr[RT(instr)];
  s32 result = rt << sa;
  regs.gpr[RD(instr)] = (s64)result;
}

void Cpu::dsll32(u32 instr) {
  u8 sa = ((instr >> 6) & 0x1f);
  s64 result = regs.gpr[RT(instr)] << (sa + 32);
  regs.gpr[RD(instr)] = result;
}

void Cpu::dsll(u32 instr) {
  u8 sa = ((instr >> 6) & 0x1f);
  s64 result = regs.gpr[RT(instr)] << sa;
  regs.gpr[RD(instr)] = result;
}

void Cpu::dsllv(u32 instr) {
  s64 sa = regs.gpr[RS(instr)] & 63;
  s64 result = regs.gpr[RT(instr)] << sa;
  regs.gpr[RD(instr)] = result;
}

void Cpu::srl(u32 instr) {
  u32 rt = regs.gpr[RT(instr)];
  u8 sa = ((instr >> 6) & 0x1f);
  u32 result = rt >> sa;
  regs.gpr[RD(instr)] = (s32)result;
}

void Cpu::srlv(u32 instr) {
  u8 sa = (regs.gpr[RS(instr)] & 0x1F);
  u32 rt = regs.gpr[RT(instr)];
  s32 result = rt >> sa;
  regs.gpr[RD(instr)] = (s64)result;
}

void Cpu::dsrl(u32 instr) {
  u64 rt = regs.gpr[RT(instr)];
  u8 sa = ((instr >> 6) & 0x1f);
  u64 result = rt >> sa;
  regs.gpr[RD(instr)] = s64(result);
}

void Cpu::dsrlv(u32 instr) {
  u8 amount = (regs.gpr[RS(instr)] & 63);
  u64 rt = regs.gpr[RT(instr)];
  u64 result = rt >> amount;
  regs.gpr[RD(instr)] = s64(result);
}

void Cpu::dsrl32(u32 instr) {
  u64 rt = regs.gpr[RT(instr)];
  u8 sa = ((instr >> 6) & 0x1f);
  u64 result = rt >> (sa + 32);
  regs.gpr[RD(instr)] = s64(result);
}

void Cpu::sra(u32 instr) {
  s64 rt = regs.gpr[RT(instr)];
  u8 sa = ((instr >> 6) & 0x1f);
  s32 result = rt >> sa;
  regs.gpr[RD(instr)] = result;
}

void Cpu::srav(u32 instr) {
  s64 rt = regs.gpr[RT(instr)];
  s64 rs = regs.gpr[RS(instr)];
  u8 sa = rs & 0x1f;
  s32 result = rt >> sa;
  regs.gpr[RD(instr)] = result;
}

void Cpu::dsra(u32 instr) {
  s64 rt = regs.gpr[RT(instr)];
  u8 sa = ((instr >> 6) & 0x1f);
  s64 result = rt >> sa;
  regs.gpr[RD(instr)] = result;
}

void Cpu::dsrav(u32 instr) {
  s64 rt = regs.gpr[RT(instr)];
  s64 rs = regs.gpr[RS(instr)];
  s64 sa = rs & 63;
  s64 result = rt >> sa;
  regs.gpr[RD(instr)] = result;
}

void Cpu::dsra32(u32 instr) {
  s64 rt = regs.gpr[RT(instr)];
  u8 sa = ((instr >> 6) & 0x1f);
  s64 result = rt >> (sa + 32);
  regs.gpr[RD(instr)] = result;
}

void Cpu::jr(u32 instr) {
  s64 address = regs.gpr[RS(instr)];
  if ((address & 3) != 0 || (address > 0)) {
    HandleTLBException(regs, address);
    FireException(regs, ExceptionCode::AddressErrorStore, 0, regs.oldPC);
  }

  branch(true, address);
}

void Cpu::dsub(u32 instr) {
  s64 rt = regs.gpr[RT(instr)];
  s64 rs = regs.gpr[RS(instr)];
  s64 result = rs - rt;
  regs.gpr[RD(instr)] = result;
}

void Cpu::dsubu(u32 instr) {
  u64 rt = regs.gpr[RT(instr)];
  u64 rs = regs.gpr[RS(instr)];
  u64 result = rs - rt;
  regs.gpr[RD(instr)] = s64(result);
}

void Cpu::sub(u32 instr) {
  s32 rt = regs.gpr[RT(instr)];
  s32 rs = regs.gpr[RS(instr)];
  s32 result = rs - rt;
  regs.gpr[RD(instr)] = (s64)result;
}

void Cpu::subu(u32 instr) {
  u32 rt = regs.gpr[RT(instr)];
  u32 rs = regs.gpr[RS(instr)];
  u32 result = rs - rt;
  regs.gpr[RD(instr)] = (s64)((s32)result);
}

void Cpu::dmultu(u32 instr) {
  u64 rt = regs.gpr[RT(instr)];
  u64 rs = regs.gpr[RS(instr)];
  u128 result = (u128)rt * (u128)rs;
  regs.lo = (s64)(result & 0xFFFFFFFFFFFFFFFF);
  regs.hi = (s64)(result >> 64);
}

void Cpu::dmult(u32 instr) {
  s64 rt = regs.gpr[RT(instr)];
  s64 rs = regs.gpr[RS(instr)];
  s128 result = (s128)rt * (s128)rs;
  regs.lo = result & 0xFFFFFFFFFFFFFFFF;
  regs.hi = result >> 64;
}

void Cpu::multu(u32 instr) {
  u32 rt = regs.gpr[RT(instr)];
  u32 rs = regs.gpr[RS(instr)];
  u64 result = (u64)rt * (u64)rs;
  regs.lo = (s64)((s32)result);
  regs.hi = (s64)((s32)(result >> 32));
}

void Cpu::mult(u32 instr) {
  s32 rt = regs.gpr[RT(instr)];
  s32 rs = regs.gpr[RS(instr)];
  s64 result = (s64)rt * (s64)rs;
  regs.lo = (s64)((s32)result);
  regs.hi = (s64)((s32)(result >> 32));
}

void Cpu::mflo(u32 instr) {
  regs.gpr[RD(instr)] = regs.lo;
}

void Cpu::mfhi(u32 instr) {
  regs.gpr[RD(instr)] = regs.hi;
}

void Cpu::mtlo(u32 instr) {
  regs.lo = regs.gpr[RS(instr)];
}

void Cpu::mthi(u32 instr) {
  regs.hi = regs.gpr[RS(instr)];
}

void Cpu::trap(bool cond) {
  if(cond) {
    FireException(regs, ExceptionCode::Trap, 0, regs.oldPC);
  }
}
}