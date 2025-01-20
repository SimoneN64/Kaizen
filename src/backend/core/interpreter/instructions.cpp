#include <core/Interpreter.hpp>

#define check_signed_overflow(op1, op2, res) (((~((op1) ^ (op2)) & ((op1) ^ (res))) >> ((sizeof(res) * 8) - 1)) & 1)
#define check_signed_underflow(op1, op2, res) (((((op1) ^ (op2)) & ((op1) ^ (res))) >> ((sizeof(res) * 8) - 1)) & 1)

namespace n64 {
void Interpreter::add(const u32 instr) {
  const u32 rs = regs.Read<s32>(RS(instr));
  const u32 rt = regs.Read<s32>(RT(instr));
  if (const u32 result = rs + rt; check_signed_overflow(rs, rt, result)) {
    regs.cop0.FireException(ExceptionCode::Overflow, 0, regs.oldPC);
  } else {
    regs.Write(RD(instr), static_cast<s32>(result));
  }
}

void Interpreter::addu(const u32 instr) {
  const s32 rs = regs.Read<s32>(RS(instr));
  const s32 rt = regs.Read<s32>(RT(instr));
  const s32 result = rs + rt;
  regs.Write(RD(instr), result);
}

void Interpreter::addi(const u32 instr) {
  const u32 rs = regs.Read<s64>(RS(instr));
  const u32 imm = static_cast<s32>(static_cast<s16>(instr));
  if (const u32 result = rs + imm; check_signed_overflow(rs, imm, result)) {
    regs.cop0.FireException(ExceptionCode::Overflow, 0, regs.oldPC);
  } else {
    regs.Write(RT(instr), static_cast<s32>(result));
  }
}

void Interpreter::addiu(const u32 instr) {
  const s32 rs = regs.Read<s32>(RS(instr));
  const s16 imm = static_cast<s16>(instr);
  const s32 result = rs + imm;
  regs.Write(RT(instr), result);
}

void Interpreter::dadd(const u32 instr) {
  const u64 rs = regs.Read<s64>(RS(instr));
  const u64 rt = regs.Read<s64>(RT(instr));
  if (const u64 result = rt + rs; check_signed_overflow(rs, rt, result)) {
    regs.cop0.FireException(ExceptionCode::Overflow, 0, regs.oldPC);
  } else {
    regs.Write(RD(instr), result);
  }
}

void Interpreter::daddu(const u32 instr) {
  const s64 rs = regs.Read<s64>(RS(instr));
  const s64 rt = regs.Read<s64>(RT(instr));
  regs.Write(RD(instr), rs + rt);
}

void Interpreter::daddi(const u32 instr) {
  const u64 imm = s64(s16(instr));
  const u64 rs = regs.Read<s64>(RS(instr));
  if (const u64 result = imm + rs; check_signed_overflow(rs, imm, result)) {
    regs.cop0.FireException(ExceptionCode::Overflow, 0, regs.oldPC);
  } else {
    regs.Write(RT(instr), result);
  }
}

void Interpreter::daddiu(const u32 instr) {
  const s16 imm = static_cast<s16>(instr);
  const s64 rs = regs.Read<s64>(RS(instr));
  regs.Write(RT(instr), rs + imm);
}

void Interpreter::div(const u32 instr) {
  const s64 dividend = regs.Read<s32>(RS(instr));

  if (const s64 divisor = regs.Read<s32>(RT(instr)); divisor == 0) {
    regs.hi = dividend;
    if (dividend >= 0) {
      regs.lo = static_cast<s64>(-1);
    } else {
      regs.lo = static_cast<s64>(1);
    }
  } else {
    const s32 quotient = dividend / divisor;
    const s32 remainder = dividend % divisor;
    regs.lo = quotient;
    regs.hi = remainder;
  }
}

void Interpreter::divu(const u32 instr) {
  const u32 dividend = regs.Read<s64>(RS(instr));
  if (const u32 divisor = regs.Read<s64>(RT(instr)); divisor == 0) {
    regs.lo = -1;
    regs.hi = (s32)dividend;
  } else {
    const s32 quotient = (s32)(dividend / divisor);
    const s32 remainder = (s32)(dividend % divisor);
    regs.lo = quotient;
    regs.hi = remainder;
  }
}

void Interpreter::ddiv(const u32 instr) {
  const s64 dividend = regs.Read<s64>(RS(instr));
  const s64 divisor = regs.Read<s64>(RT(instr));
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
    const s64 quotient = dividend / divisor;
    const s64 remainder = dividend % divisor;
    regs.lo = quotient;
    regs.hi = remainder;
  }
}

void Interpreter::ddivu(const u32 instr) {
  const u64 dividend = regs.Read<s64>(RS(instr));
  const u64 divisor = regs.Read<s64>(RT(instr));
  if (divisor == 0) {
    regs.lo = -1;
    regs.hi = (s64)dividend;
  } else {
    const u64 quotient = dividend / divisor;
    const u64 remainder = dividend % divisor;
    regs.lo = (s64)quotient;
    regs.hi = (s64)remainder;
  }
}

void Interpreter::branch(const bool cond, const s64 address) {
  regs.delaySlot = true;
  if (cond) {
    regs.nextPC = address;
  }
}

void Interpreter::branch_likely(const bool cond, const s64 address) {
  if (cond) {
    regs.delaySlot = true;
    regs.nextPC = address;
  } else {
    regs.SetPC64(regs.nextPC);
  }
}

void Interpreter::b(const u32 instr, const bool cond) {
  const s16 imm = instr;
  const s64 offset = u64((s64)imm) << 2;
  const s64 address = regs.pc + offset;
  branch(cond, address);
}

void Interpreter::blink(const u32 instr, const bool cond) {
  regs.Write(31, regs.nextPC);
  const s16 imm = instr;
  const s64 offset = u64((s64)imm) << 2;
  const s64 address = regs.pc + offset;
  branch(cond, address);
}

void Interpreter::bl(const u32 instr, const bool cond) {
  const s16 imm = instr;
  const s64 offset = u64((s64)imm) << 2;
  const s64 address = regs.pc + offset;
  branch_likely(cond, address);
}

void Interpreter::bllink(const u32 instr, const bool cond) {
  regs.Write(31, regs.nextPC);
  const s16 imm = instr;
  const s64 offset = u64((s64)imm) << 2;
  const s64 address = regs.pc + offset;
  branch_likely(cond, address);
}

void Interpreter::lui(const u32 instr) {
  u64 val = s64((s16)instr);
  val <<= 16;
  regs.Write(RT(instr), val);
}

void Interpreter::lb(const u32 instr) {
  const u64 address = regs.Read<s64>(RS(instr)) + (s16)instr;
  if (u32 paddr = 0; !regs.cop0.MapVAddr(Cop0::LOAD, address, paddr)) {
    regs.cop0.HandleTLBException(address);
    regs.cop0.FireException(Cop0::GetTLBExceptionCode(regs.cop0.tlbError, Cop0::LOAD), 0, regs.oldPC);
  } else {
    regs.Write(RT(instr), (s8)mem.Read<u8>(regs, paddr));
  }
}

void Interpreter::lh(const u32 instr) {
  const u64 address = regs.Read<s64>(RS(instr)) + (s16)instr;
  if (check_address_error(0b1, address)) {
    regs.cop0.HandleTLBException(address);
    regs.cop0.FireException(ExceptionCode::AddressErrorLoad, 0, regs.oldPC);
    return;
  }

  u32 paddr = 0;
  if (!regs.cop0.MapVAddr(Cop0::LOAD, address, paddr)) {
    regs.cop0.HandleTLBException(address);
    regs.cop0.FireException(Cop0::GetTLBExceptionCode(regs.cop0.tlbError, Cop0::LOAD), 0, regs.oldPC);
  } else {
    regs.Write(RT(instr), (s16)mem.Read<u16>(regs, paddr));
  }
}

void Interpreter::lw(const u32 instr) {
  const s16 offset = instr;
  const u64 address = regs.Read<s64>(RS(instr)) + offset;
  if (check_address_error(0b11, address)) {
    regs.cop0.HandleTLBException(address);
    regs.cop0.FireException(ExceptionCode::AddressErrorLoad, 0, regs.oldPC);
    return;
  }

  u32 physical = 0;
  if (!regs.cop0.MapVAddr(Cop0::LOAD, address, physical)) {
    regs.cop0.HandleTLBException(address);
    regs.cop0.FireException(Cop0::GetTLBExceptionCode(regs.cop0.tlbError, Cop0::LOAD), 0, regs.oldPC);
  } else {
    regs.Write(RT(instr), (s32)mem.Read<u32>(regs, physical));
  }
}

void Interpreter::ll(const u32 instr) {
  const u64 address = regs.Read<s64>(RS(instr)) + (s16)instr;
  u32 physical;
  if (!regs.cop0.MapVAddr(Cop0::LOAD, address, physical)) {
    regs.cop0.HandleTLBException(address);
    regs.cop0.FireException(Cop0::GetTLBExceptionCode(regs.cop0.tlbError, Cop0::LOAD), 0, regs.oldPC);
  } else {
    const s32 result = mem.Read<u32>(regs, physical);
    if (check_address_error(0b11, address)) {
      regs.cop0.FireException(ExceptionCode::AddressErrorLoad, 0, regs.oldPC);
      return;
    }

    regs.Write(RT(instr), result);

    regs.cop0.llbit = true;
    regs.cop0.LLAddr = physical >> 4;
  }
}

void Interpreter::lwl(const u32 instr) {
  const u64 address = regs.Read<s64>(RS(instr)) + (s16)instr;
  u32 paddr = 0;
  if (!regs.cop0.MapVAddr(Cop0::LOAD, address, paddr)) {
    regs.cop0.HandleTLBException(address);
    regs.cop0.FireException(Cop0::GetTLBExceptionCode(regs.cop0.tlbError, Cop0::LOAD), 0, regs.oldPC);
  } else {
    const u32 shift = 8 * ((address ^ 0) & 3);
    const u32 mask = 0xFFFFFFFF << shift;
    const u32 data = mem.Read<u32>(regs, paddr & ~3);
    const s32 result = s32((regs.Read<s64>(RT(instr)) & ~mask) | (data << shift));
    regs.Write(RT(instr), result);
  }
}

void Interpreter::lwr(const u32 instr) {
  const u64 address = regs.Read<s64>(RS(instr)) + (s16)instr;
  u32 paddr = 0;
  if (!regs.cop0.MapVAddr(Cop0::LOAD, address, paddr)) {
    regs.cop0.HandleTLBException(address);
    regs.cop0.FireException(Cop0::GetTLBExceptionCode(regs.cop0.tlbError, Cop0::LOAD), 0, regs.oldPC);
  } else {
    const u32 shift = 8 * ((address ^ 3) & 3);
    const u32 mask = 0xFFFFFFFF >> shift;
    const u32 data = mem.Read<u32>(regs, paddr & ~3);
    const s32 result = s32((regs.Read<s64>(RT(instr)) & ~mask) | (data >> shift));
    regs.Write(RT(instr), result);
  }
}

void Interpreter::ld(const u32 instr) {
  const s64 address = regs.Read<s64>(RS(instr)) + (s16)instr;
  if (check_address_error(0b111, address)) {
    regs.cop0.HandleTLBException(address);
    regs.cop0.FireException(ExceptionCode::AddressErrorLoad, 0, regs.oldPC);
    return;
  }

  u32 paddr = 0;
  if (!regs.cop0.MapVAddr(Cop0::LOAD, address, paddr)) {
    regs.cop0.HandleTLBException(address);
    regs.cop0.FireException(Cop0::GetTLBExceptionCode(regs.cop0.tlbError, Cop0::LOAD), 0, regs.oldPC);
  } else {
    const s64 value = mem.Read<u64>(regs, paddr);
    regs.Write(RT(instr), value);
  }
}

void Interpreter::lld(const u32 instr) {
  if (!regs.cop0.is64BitAddressing && !regs.cop0.kernelMode) {
    regs.cop0.FireException(ExceptionCode::ReservedInstruction, 0, regs.oldPC);
    return;
  }

  const u64 address = regs.Read<s64>(RS(instr)) + (s16)instr;
  u32 paddr;
  if (!regs.cop0.MapVAddr(Cop0::LOAD, address, paddr)) {
    regs.cop0.HandleTLBException(address);
    regs.cop0.FireException(Cop0::GetTLBExceptionCode(regs.cop0.tlbError, Cop0::LOAD), 0, regs.oldPC);
  } else {
    if (check_address_error(0b111, address)) {
      regs.cop0.FireException(ExceptionCode::AddressErrorLoad, 0, regs.oldPC);
    } else {
      regs.Write(RT(instr), mem.Read<u64>(regs, paddr));
      regs.cop0.llbit = true;
      regs.cop0.LLAddr = paddr >> 4;
    }
  }
}

void Interpreter::ldl(const u32 instr) {
  const u64 address = regs.Read<s64>(RS(instr)) + (s16)instr;
  u32 paddr = 0;
  if (!regs.cop0.MapVAddr(Cop0::LOAD, address, paddr)) {
    regs.cop0.HandleTLBException(address);
    regs.cop0.FireException(Cop0::GetTLBExceptionCode(regs.cop0.tlbError, Cop0::LOAD), 0, regs.oldPC);
  } else {
    const s32 shift = 8 * ((address ^ 0) & 7);
    const u64 mask = 0xFFFFFFFFFFFFFFFF << shift;
    const u64 data = mem.Read<u64>(regs, paddr & ~7);
    const s64 result = (s64)((regs.Read<s64>(RT(instr)) & ~mask) | (data << shift));
    regs.Write(RT(instr), result);
  }
}

void Interpreter::ldr(const u32 instr) {
  const u64 address = regs.Read<s64>(RS(instr)) + (s16)instr;
  u32 paddr;
  if (!regs.cop0.MapVAddr(Cop0::LOAD, address, paddr)) {
    regs.cop0.HandleTLBException(address);
    regs.cop0.FireException(Cop0::GetTLBExceptionCode(regs.cop0.tlbError, Cop0::LOAD), 0, regs.oldPC);
  } else {
    const s32 shift = 8 * ((address ^ 7) & 7);
    const u64 mask = 0xFFFFFFFFFFFFFFFF >> shift;
    const u64 data = mem.Read<u64>(regs, paddr & ~7);
    const s64 result = (s64)((regs.Read<s64>(RT(instr)) & ~mask) | (data >> shift));
    regs.Write(RT(instr), result);
  }
}

void Interpreter::lbu(const u32 instr) {
  const u64 address = regs.Read<s64>(RS(instr)) + (s16)instr;
  u32 paddr;
  if (!regs.cop0.MapVAddr(Cop0::LOAD, address, paddr)) {
    regs.cop0.HandleTLBException(address);
    regs.cop0.FireException(Cop0::GetTLBExceptionCode(regs.cop0.tlbError, Cop0::LOAD), 0, regs.oldPC);
  } else {
    const u8 value = mem.Read<u8>(regs, paddr);
    regs.Write(RT(instr), value);
  }
}

void Interpreter::lhu(const u32 instr) {
  const s64 address = regs.Read<s64>(RS(instr)) + (s16)instr;
  if (check_address_error(0b1, address)) {
    regs.cop0.HandleTLBException(address);
    regs.cop0.FireException(ExceptionCode::AddressErrorLoad, 0, regs.oldPC);
    return;
  }
  u32 paddr;
  if (!regs.cop0.MapVAddr(Cop0::LOAD, address, paddr)) {
    regs.cop0.HandleTLBException(address);
    regs.cop0.FireException(Cop0::GetTLBExceptionCode(regs.cop0.tlbError, Cop0::LOAD), 0, regs.oldPC);
  } else {
    const u16 value = mem.Read<u16>(regs, paddr);
    regs.Write(RT(instr), value);
  }
}

void Interpreter::lwu(const u32 instr) {
  const s64 address = regs.Read<s64>(RS(instr)) + (s16)instr;
  if (check_address_error(0b11, address)) {
    regs.cop0.HandleTLBException(address);
    regs.cop0.FireException(ExceptionCode::AddressErrorLoad, 0, regs.oldPC);
    return;
  }

  u32 paddr;
  if (!regs.cop0.MapVAddr(Cop0::LOAD, address, paddr)) {
    regs.cop0.HandleTLBException(address);
    regs.cop0.FireException(Cop0::GetTLBExceptionCode(regs.cop0.tlbError, Cop0::LOAD), 0, regs.oldPC);
  } else {
    const u32 value = mem.Read<u32>(regs, paddr);
    regs.Write(RT(instr), value);
  }
}

void Interpreter::sb(const u32 instr) {
  const u64 address = regs.Read<s64>(RS(instr)) + (s16)instr;
  u32 paddr;
  if (!regs.cop0.MapVAddr(Cop0::STORE, address, paddr)) {
    regs.cop0.HandleTLBException(address);
    regs.cop0.FireException(Cop0::GetTLBExceptionCode(regs.cop0.tlbError, Cop0::STORE), 0, regs.oldPC);
  } else {
    mem.Write<u8>(regs, paddr, regs.Read<s64>(RT(instr)));
  }
}

void Interpreter::sc(const u32 instr) {
  const u64 address = regs.Read<s64>(RS(instr)) + (s16)instr;

  if (regs.cop0.llbit) {
    regs.cop0.llbit = false;

    if (check_address_error(0b11, address)) {
      regs.Write(RT(instr), 0);
      regs.cop0.HandleTLBException(address);
      regs.cop0.FireException(ExceptionCode::AddressErrorStore, 0, regs.oldPC);
      return;
    }

    u32 paddr = 0;
    if (!regs.cop0.MapVAddr(Cop0::STORE, address, paddr)) {
      regs.Write(RT(instr), 0);
      regs.cop0.HandleTLBException(address);
      regs.cop0.FireException(Cop0::GetTLBExceptionCode(regs.cop0.tlbError, Cop0::STORE), 0, regs.oldPC);
    } else {
      mem.Write<u32>(regs, paddr, regs.Read<s64>(RT(instr)));
      regs.Write(RT(instr), 1);
    }
  } else {
    regs.Write(RT(instr), 0);
  }
}

void Interpreter::scd(const u32 instr) {
  if (!regs.cop0.is64BitAddressing && !regs.cop0.kernelMode) {
    regs.cop0.FireException(ExceptionCode::ReservedInstruction, 0, regs.oldPC);
    return;
  }

  const s64 address = regs.Read<s64>(RS(instr)) + (s16)instr;

  if (regs.cop0.llbit) {
    regs.cop0.llbit = false;

    if (check_address_error(0b111, address)) {
      regs.Write(RT(instr), 0);
      regs.cop0.HandleTLBException(address);
      regs.cop0.FireException(ExceptionCode::AddressErrorStore, 0, regs.oldPC);
      return;
    }

    u32 paddr = 0;
    if (!regs.cop0.MapVAddr(Cop0::STORE, address, paddr)) {
      regs.Write(RT(instr), 0);
      regs.cop0.HandleTLBException(address);
      regs.cop0.FireException(Cop0::GetTLBExceptionCode(regs.cop0.tlbError, Cop0::STORE), 0, regs.oldPC);
    } else {
      mem.Write<u32>(regs, paddr, regs.Read<s64>(RT(instr)));
      regs.Write(RT(instr), 1);
    }
  } else {
    regs.Write(RT(instr), 0);
  }
}

void Interpreter::sh(const u32 instr) {
  const s64 address = regs.Read<s64>(RS(instr)) + (s16)instr;

  u32 physical;
  if (!regs.cop0.MapVAddr(Cop0::STORE, address, physical)) {
    regs.cop0.HandleTLBException(address);
    regs.cop0.FireException(Cop0::GetTLBExceptionCode(regs.cop0.tlbError, Cop0::STORE), 0, regs.oldPC);
  } else {
    mem.Write<u16>(regs, physical, regs.Read<s64>(RT(instr)));
  }
}

void Interpreter::sw(const u32 instr) {
  const s16 offset = instr;
  const u64 address = regs.Read<s64>(RS(instr)) + offset;
  if (check_address_error(0b11, address)) {
    regs.cop0.HandleTLBException(address);
    regs.cop0.FireException(ExceptionCode::AddressErrorStore, 0, regs.oldPC);
    return;
  }

  u32 physical;
  if (!regs.cop0.MapVAddr(Cop0::STORE, address, physical)) {
    regs.cop0.HandleTLBException(address);
    regs.cop0.FireException(Cop0::GetTLBExceptionCode(regs.cop0.tlbError, Cop0::STORE), 0, regs.oldPC);
  } else {
    mem.Write<u32>(regs, physical, regs.Read<s64>(RT(instr)));
  }
}

void Interpreter::sd(const u32 instr) {
  const s64 address = regs.Read<s64>(RS(instr)) + (s16)instr;
  if (check_address_error(0b111, address)) {
    regs.cop0.HandleTLBException(address);
    regs.cop0.FireException(ExceptionCode::AddressErrorStore, 0, regs.oldPC);
    return;
  }

  u32 physical;
  if (!regs.cop0.MapVAddr(Cop0::STORE, address, physical)) {
    regs.cop0.HandleTLBException(address);
    regs.cop0.FireException(Cop0::GetTLBExceptionCode(regs.cop0.tlbError, Cop0::STORE), 0, regs.oldPC);
  } else {
    mem.Write(regs, physical, regs.Read<s64>(RT(instr)));
  }
}

void Interpreter::sdl(const u32 instr) {
  const u64 address = regs.Read<s64>(RS(instr)) + (s16)instr;
  u32 paddr;
  if (!regs.cop0.MapVAddr(Cop0::STORE, address, paddr)) {
    regs.cop0.HandleTLBException(address);
    regs.cop0.FireException(Cop0::GetTLBExceptionCode(regs.cop0.tlbError, Cop0::STORE), 0, regs.oldPC);
  } else {
    const s32 shift = 8 * ((address ^ 0) & 7);
    const u64 mask = 0xFFFFFFFFFFFFFFFF >> shift;
    const u64 data = mem.Read<u64>(regs, paddr & ~7);
    const u64 rt = regs.Read<s64>(RT(instr));
    mem.Write(regs, paddr & ~7, (data & ~mask) | (rt >> shift));
  }
}

void Interpreter::sdr(const u32 instr) {
  const u64 address = regs.Read<s64>(RS(instr)) + (s16)instr;
  u32 paddr;
  if (!regs.cop0.MapVAddr(Cop0::STORE, address, paddr)) {
    regs.cop0.HandleTLBException(address);
    regs.cop0.FireException(Cop0::GetTLBExceptionCode(regs.cop0.tlbError, Cop0::STORE), 0, regs.oldPC);
  } else {
    const s32 shift = 8 * ((address ^ 7) & 7);
    const u64 mask = 0xFFFFFFFFFFFFFFFF << shift;
    const u64 data = mem.Read<u64>(regs, paddr & ~7);
    const u64 rt = regs.Read<s64>(RT(instr));
    mem.Write(regs, paddr & ~7, (data & ~mask) | (rt << shift));
  }
}

void Interpreter::swl(const u32 instr) {
  const u64 address = regs.Read<s64>(RS(instr)) + (s16)instr;
  u32 paddr;
  if (!regs.cop0.MapVAddr(Cop0::STORE, address, paddr)) {
    regs.cop0.HandleTLBException(address);
    regs.cop0.FireException(Cop0::GetTLBExceptionCode(regs.cop0.tlbError, Cop0::STORE), 0, regs.oldPC);
  } else {
    const u32 shift = 8 * ((address ^ 0) & 3);
    const u32 mask = 0xFFFFFFFF >> shift;
    const u32 data = mem.Read<u32>(regs, paddr & ~3);
    const u32 rt = regs.Read<s64>(RT(instr));
    mem.Write<u32>(regs, paddr & ~3, (data & ~mask) | (rt >> shift));
  }
}

void Interpreter::swr(const u32 instr) {
  const u64 address = regs.Read<s64>(RS(instr)) + (s16)instr;
  u32 paddr;
  if (!regs.cop0.MapVAddr(Cop0::STORE, address, paddr)) {
    regs.cop0.HandleTLBException(address);
    regs.cop0.FireException(Cop0::GetTLBExceptionCode(regs.cop0.tlbError, Cop0::STORE), 0, regs.oldPC);
  } else {
    const u32 shift = 8 * ((address ^ 3) & 3);
    const u32 mask = 0xFFFFFFFF << shift;
    const u32 data = mem.Read<u32>(regs, paddr & ~3);
    const u32 rt = regs.Read<s64>(RT(instr));
    mem.Write<u32>(regs, paddr & ~3, (data & ~mask) | (rt << shift));
  }
}

void Interpreter::ori(const u32 instr) {
  const s64 imm = (u16)instr;
  const s64 result = imm | regs.Read<s64>(RS(instr));
  regs.Write(RT(instr), result);
}

void Interpreter::or_(const u32 instr) { regs.Write(RD(instr), regs.Read<s64>(RS(instr)) | regs.Read<s64>(RT(instr))); }

void Interpreter::nor(const u32 instr) {
  regs.Write(RD(instr), ~(regs.Read<s64>(RS(instr)) | regs.Read<s64>(RT(instr))));
}

void Interpreter::j(const u32 instr) {
  const s32 target = (instr & 0x3ffffff) << 2;
  const s64 address = (regs.oldPC & ~0xfffffff) | target;

  branch(true, address);
}

void Interpreter::jal(const u32 instr) {
  regs.Write(31, regs.nextPC);
  j(instr);
}

void Interpreter::jalr(const u32 instr) {
  regs.Write(RD(instr), regs.nextPC);
  jr(instr);
}

void Interpreter::jr(const u32 instr) {
  const u64 address = regs.Read<s64>(RS(instr));
  branch(true, address);
}

void Interpreter::slti(const u32 instr) {
  const s16 imm = instr;
  regs.Write(RT(instr), regs.Read<s64>(RS(instr)) < imm);
}

void Interpreter::sltiu(const u32 instr) {
  const s16 imm = instr;
  regs.Write(RT(instr), regs.Read<u64>(RS(instr)) < imm);
}

void Interpreter::slt(const u32 instr) { regs.Write(RD(instr), regs.Read<s64>(RS(instr)) < regs.Read<s64>(RT(instr))); }

void Interpreter::sltu(const u32 instr) {
  regs.Write(RD(instr), regs.Read<u64>(RS(instr)) < regs.Read<u64>(RT(instr)));
}

void Interpreter::xori(const u32 instr) {
  const s64 imm = (u16)instr;
  regs.Write(RT(instr), regs.Read<s64>(RS(instr)) ^ imm);
}

void Interpreter::xor_(const u32 instr) {
  regs.Write(RD(instr), regs.Read<s64>(RT(instr)) ^ regs.Read<s64>(RS(instr)));
}

void Interpreter::andi(const u32 instr) {
  const s64 imm = (u16)instr;
  regs.Write(RT(instr), regs.Read<s64>(RS(instr)) & imm);
}

void Interpreter::and_(const u32 instr) {
  regs.Write(RD(instr), regs.Read<s64>(RS(instr)) & regs.Read<s64>(RT(instr)));
}

void Interpreter::sll(const u32 instr) {
  const u8 sa = ((instr >> 6) & 0x1f);
  const s32 result = regs.Read<s64>(RT(instr)) << sa;
  regs.Write(RD(instr), (s64)result);
}

void Interpreter::sllv(const u32 instr) {
  const u8 sa = (regs.Read<s64>(RS(instr))) & 0x1F;
  const u32 rt = regs.Read<s64>(RT(instr));
  const s32 result = rt << sa;
  regs.Write(RD(instr), (s64)result);
}

void Interpreter::dsll32(const u32 instr) {
  const u8 sa = ((instr >> 6) & 0x1f);
  const s64 result = regs.Read<s64>(RT(instr)) << (sa + 32);
  regs.Write(RD(instr), result);
}

void Interpreter::dsll(const u32 instr) {
  const u8 sa = ((instr >> 6) & 0x1f);
  const s64 result = regs.Read<s64>(RT(instr)) << sa;
  regs.Write(RD(instr), result);
}

void Interpreter::dsllv(const u32 instr) {
  const s64 sa = regs.Read<s64>(RS(instr)) & 63;
  const s64 result = regs.Read<s64>(RT(instr)) << sa;
  regs.Write(RD(instr), result);
}

void Interpreter::srl(const u32 instr) {
  const u32 rt = regs.Read<s64>(RT(instr));
  const u8 sa = ((instr >> 6) & 0x1f);
  const u32 result = rt >> sa;
  regs.Write(RD(instr), (s32)result);
}

void Interpreter::srlv(const u32 instr) {
  const u8 sa = (regs.Read<s64>(RS(instr)) & 0x1F);
  const u32 rt = regs.Read<s64>(RT(instr));
  const s32 result = rt >> sa;
  regs.Write(RD(instr), (s64)result);
}

void Interpreter::dsrl(const u32 instr) {
  const u64 rt = regs.Read<s64>(RT(instr));
  const u8 sa = ((instr >> 6) & 0x1f);
  const u64 result = rt >> sa;
  regs.Write(RD(instr), s64(result));
}

void Interpreter::dsrlv(const u32 instr) {
  const u8 amount = (regs.Read<s64>(RS(instr)) & 63);
  const u64 rt = regs.Read<s64>(RT(instr));
  const u64 result = rt >> amount;
  regs.Write(RD(instr), s64(result));
}

void Interpreter::dsrl32(const u32 instr) {
  const u64 rt = regs.Read<s64>(RT(instr));
  const u8 sa = ((instr >> 6) & 0x1f);
  const u64 result = rt >> (sa + 32);
  regs.Write(RD(instr), s64(result));
}

void Interpreter::sra(const u32 instr) {
  const s64 rt = regs.Read<s64>(RT(instr));
  const u8 sa = ((instr >> 6) & 0x1f);
  const s32 result = rt >> sa;
  regs.Write(RD(instr), result);
}

void Interpreter::srav(const u32 instr) {
  const s64 rs = regs.Read<s64>(RS(instr));
  const s64 rt = regs.Read<s64>(RT(instr));
  const u8 sa = rs & 0x1f;
  const s32 result = rt >> sa;
  regs.Write(RD(instr), result);
}

void Interpreter::dsra(const u32 instr) {
  const s64 rt = regs.Read<s64>(RT(instr));
  const u8 sa = ((instr >> 6) & 0x1f);
  const s64 result = rt >> sa;
  regs.Write(RD(instr), result);
}

void Interpreter::dsrav(const u32 instr) {
  const s64 rt = regs.Read<s64>(RT(instr));
  const s64 rs = regs.Read<s64>(RS(instr));
  const s64 sa = rs & 63;
  const s64 result = rt >> sa;
  regs.Write(RD(instr), result);
}

void Interpreter::dsra32(const u32 instr) {
  const s64 rt = regs.Read<s64>(RT(instr));
  const u8 sa = ((instr >> 6) & 0x1f);
  const s64 result = rt >> (sa + 32);
  regs.Write(RD(instr), result);
}

void Interpreter::dsub(const u32 instr) {
  const s64 rt = regs.Read<s64>(RT(instr));
  const s64 rs = regs.Read<s64>(RS(instr));
  if (const s64 result = rs - rt; check_signed_underflow(rs, rt, result)) {
    regs.cop0.FireException(ExceptionCode::Overflow, 0, regs.oldPC);
  } else {
    regs.Write(RD(instr), result);
  }
}

void Interpreter::dsubu(const u32 instr) {
  const u64 rt = regs.Read<s64>(RT(instr));
  const u64 rs = regs.Read<s64>(RS(instr));
  const u64 result = rs - rt;
  regs.Write(RD(instr), s64(result));
}

void Interpreter::sub(const u32 instr) {
  const s32 rt = regs.Read<s64>(RT(instr));
  const s32 rs = regs.Read<s64>(RS(instr));
  const s32 result = rs - rt;
  if (check_signed_underflow(rs, rt, result)) {
    regs.cop0.FireException(ExceptionCode::Overflow, 0, regs.oldPC);
  } else {
    regs.Write(RD(instr), result);
  }
}

void Interpreter::subu(const u32 instr) {
  const u32 rt = regs.Read<s64>(RT(instr));
  const u32 rs = regs.Read<s64>(RS(instr));
  const u32 result = rs - rt;
  regs.Write(RD(instr), (s64)((s32)result));
}

void Interpreter::dmultu(const u32 instr) {
  const u64 rt = regs.Read<s64>(RT(instr));
  const u64 rs = regs.Read<s64>(RS(instr));
  const u128 result = (u128)rt * (u128)rs;
  regs.lo = (s64)(result & 0xFFFFFFFFFFFFFFFF);
  regs.hi = (s64)(result >> 64);
}

void Interpreter::dmult(const u32 instr) {
  const s64 rt = regs.Read<s64>(RT(instr));
  const s64 rs = regs.Read<s64>(RS(instr));
  const s128 result = (s128)rt * (s128)rs;
  regs.lo = result & 0xFFFFFFFFFFFFFFFF;
  regs.hi = result >> 64;
}

void Interpreter::multu(const u32 instr) {
  const u32 rt = regs.Read<s64>(RT(instr));
  const u32 rs = regs.Read<s64>(RS(instr));
  const u64 result = (u64)rt * (u64)rs;
  regs.lo = (s64)((s32)result);
  regs.hi = (s64)((s32)(result >> 32));
}

void Interpreter::mult(const u32 instr) {
  const s32 rt = regs.Read<s64>(RT(instr));
  const s32 rs = regs.Read<s64>(RS(instr));
  const s64 result = (s64)rt * (s64)rs;
  regs.lo = (s64)((s32)result);
  regs.hi = (s64)((s32)(result >> 32));
}

void Interpreter::mflo(const u32 instr) { regs.Write(RD(instr), regs.lo); }

void Interpreter::mfhi(const u32 instr) { regs.Write(RD(instr), regs.hi); }

void Interpreter::mtlo(const u32 instr) { regs.lo = regs.Read<s64>(RS(instr)); }

void Interpreter::mthi(const u32 instr) { regs.hi = regs.Read<s64>(RS(instr)); }

void Interpreter::trap(const bool cond) const {
  if (cond) {
    regs.cop0.FireException(ExceptionCode::Trap, 0, regs.oldPC);
  }
}

void Interpreter::mtc2(const u32 instr) { cop2Latch = regs.Read<s64>(RT(instr)); }

void Interpreter::mfc2(const u32 instr) {
  const s32 value = cop2Latch;
  regs.Write(RT(instr), value);
}

void Interpreter::dmtc2(const u32 instr) { cop2Latch = regs.Read<s64>(RT(instr)); }

void Interpreter::dmfc2(const u32 instr) { regs.Write(RT(instr), cop2Latch); }

void Interpreter::ctc2(u32) {}

void Interpreter::cfc2(u32) {}
} // namespace n64
