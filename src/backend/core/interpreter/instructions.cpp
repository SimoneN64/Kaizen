#include <core/Interpreter.hpp>

#define check_signed_overflow(op1, op2, res) (((~((op1) ^ (op2)) & ((op1) ^ (res))) >> ((sizeof(res) * 8) - 1)) & 1)
#define check_signed_underflow(op1, op2, res) (((((op1) ^ (op2)) & ((op1) ^ (res))) >> ((sizeof(res) * 8) - 1)) & 1)

namespace n64 {
void Interpreter::add(u32 instr) {
  u32 rs = (s32)regs.gpr[RS(instr)];
  u32 rt = (s32)regs.gpr[RT(instr)];
  u32 result = rs + rt;
  if(check_signed_overflow(rs, rt, result)) {
    regs.cop0.FireException(ExceptionCode::Overflow, 0, regs.oldPC);
  } else {
    if (RD(instr) != 0) [[likely]] {
      regs.gpr[RD(instr)] = s32(result);
    }
  }
}

void Interpreter::addu(u32 instr) {
  if (RD(instr) != 0) [[likely]] {
    s32 rs = (s32)regs.gpr[RS(instr)];
    s32 rt = (s32)regs.gpr[RT(instr)];
    s32 result = rs + rt;
    regs.gpr[RD(instr)] = result;
  }
}

void Interpreter::addi(u32 instr) {
  u32 rs = regs.gpr[RS(instr)];
  u32 imm = s32(s16(instr));
  u32 result = rs + imm;
  if(check_signed_overflow(rs, imm, result)) {
    regs.cop0.FireException(ExceptionCode::Overflow, 0, regs.oldPC);
  } else {
    if (RT(instr) != 0) [[likely]] {
      regs.gpr[RT(instr)] = s32(result);
    }
  }
}

void Interpreter::addiu(u32 instr) {
  s32 rs = (s32)regs.gpr[RS(instr)];
  s16 imm = (s16)(instr);
  s32 result = rs + imm;
  if (RT(instr) != 0) [[likely]] {
    regs.gpr[RT(instr)] = result;
  }
}

void Interpreter::dadd(u32 instr) {
  u64 rs = regs.gpr[RS(instr)];
  u64 rt = regs.gpr[RT(instr)];
  u64 result = rt + rs;
  if(check_signed_overflow(rs, rt, result)) {
    regs.cop0.FireException(ExceptionCode::Overflow, 0, regs.oldPC);
  } else {
    if (RD(instr) != 0) [[likely]] {
      regs.gpr[RD(instr)] = result;
    }
  }
}

void Interpreter::daddu(u32 instr) {
  if (RD(instr) != 0) [[likely]] {
    s64 rs = regs.gpr[RS(instr)];
    s64 rt = regs.gpr[RT(instr)];
    regs.gpr[RD(instr)] = rs + rt;
  }
}

void Interpreter::daddi(u32 instr) {
  u64 imm = s64(s16(instr));
  u64 rs = regs.gpr[RS(instr)];
  u64 result = imm + rs;
  if(check_signed_overflow(rs, imm, result)) {
    regs.cop0.FireException(ExceptionCode::Overflow, 0, regs.oldPC);
  } else {
    if (RT(instr) != 0) [[likely]] {
      regs.gpr[RT(instr)] = result;
    }
  }
}

void Interpreter::daddiu(u32 instr) {
  s16 imm = (s16)(instr);
  s64 rs = regs.gpr[RS(instr)];
  if (RT(instr) != 0) [[likely]] {
    regs.gpr[RT(instr)] = rs + imm;
  }
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
  if (cond) {
    regs.delaySlot = true;
    regs.nextPC = address;
  } else {
    regs.SetPC64(regs.nextPC);
  }
}

void Interpreter::b(u32 instr, bool cond) {
  s16 imm = instr;
  s64 offset = u64((s64)imm) << 2;
  s64 address = regs.pc + offset;
  branch(cond, address);
}

void Interpreter::blink(u32 instr, bool cond) {
  regs.gpr[31] = regs.nextPC;
  s16 imm = instr;
  s64 offset = u64((s64)imm) << 2;
  s64 address = regs.pc + offset;
  branch(cond, address);
}

void Interpreter::bl(u32 instr, bool cond) {
  s16 imm = instr;
  s64 offset = u64((s64)imm) << 2;
  s64 address = regs.pc + offset;
  branch_likely(cond, address);
}

void Interpreter::bllink(u32 instr, bool cond) {
  regs.gpr[31] = regs.nextPC;
  s16 imm = instr;
  s64 offset = u64((s64)imm) << 2;
  s64 address = regs.pc + offset;
  branch_likely(cond, address);
}

void Interpreter::lui(u32 instr) {
  u64 val = s64((s16)instr);
  val <<= 16;
  if (RT(instr) != 0) [[likely]] {
    regs.gpr[RT(instr)] = val;
  }
}

void Interpreter::lb(u32 instr) {
  u64 address = regs.gpr[RS(instr)] + (s16)instr;
  u32 paddr = 0;
  if(!regs.cop0.MapVAddr(Cop0::LOAD, address, paddr)) {
    regs.cop0.HandleTLBException(address);
    regs.cop0.FireException(regs.cop0.GetTLBExceptionCode(regs.cop0.tlbError, Cop0::LOAD), 0, regs.oldPC);
  } else {
    if (RT(instr) != 0) [[likely]] {
      regs.gpr[RT(instr)] = (s8) mem.Read<u8>(regs, paddr);
    }
  }
}

void Interpreter::lh(u32 instr) {
  u64 address = regs.gpr[RS(instr)] + (s16)instr;
  if (check_address_error(0b1, address)) {
    regs.cop0.HandleTLBException(address);
    regs.cop0.FireException(ExceptionCode::AddressErrorLoad, 0, regs.oldPC);
    return;
  }

  u32 paddr = 0;
  if(!regs.cop0.MapVAddr(Cop0::LOAD, address, paddr)) {
    regs.cop0.HandleTLBException(address);
    regs.cop0.FireException(regs.cop0.GetTLBExceptionCode(regs.cop0.tlbError, Cop0::LOAD), 0, regs.oldPC);
  } else {
    if (RT(instr) != 0) [[likely]] {
      regs.gpr[RT(instr)] = (s16) mem.Read<u16>(regs, paddr);
    }
  }
}

void Interpreter::lw(u32 instr) {
  s16 offset = instr;
  u64 address = regs.gpr[RS(instr)] + offset;
  if (check_address_error(0b11, address)) {
    regs.cop0.HandleTLBException(address);
    regs.cop0.FireException(ExceptionCode::AddressErrorLoad, 0, regs.oldPC);
    return;
  }

  u32 physical = 0;
  if (!regs.cop0.MapVAddr(Cop0::LOAD, address, physical)) {
    regs.cop0.HandleTLBException(address);
    regs.cop0.FireException(regs.cop0.GetTLBExceptionCode(regs.cop0.tlbError, Cop0::LOAD), 0, regs.oldPC);
  } else {
    if(RT(instr) != 0) [[likely]] {
      regs.gpr[RT(instr)] = (s32) mem.Read<u32>(regs, physical);
    }
  }
}

void Interpreter::ll(u32 instr) {
  u64 address = regs.gpr[RS(instr)] + (s16)instr;
  u32 physical;
  if (!regs.cop0.MapVAddr(Cop0::LOAD, address, physical)) {
    regs.cop0.HandleTLBException(address);
    regs.cop0.FireException(regs.cop0.GetTLBExceptionCode(regs.cop0.tlbError, Cop0::LOAD), 0, regs.oldPC);
  } else {
    s32 result = mem.Read<u32>(regs, physical);
    if (check_address_error(0b11, address)) {
      regs.cop0.FireException(ExceptionCode::AddressErrorLoad, 0, regs.oldPC);
      return;
    }

    if (RT(instr) != 0) [[likely]] {
      regs.gpr[RT(instr)] = result;
    }

    regs.cop0.llbit = true;
    regs.cop0.LLAddr = physical >> 4;
  }
}

void Interpreter::lwl(u32 instr) {
  u64 address = regs.gpr[RS(instr)] + (s16)instr;
  u32 paddr = 0;
  if(!regs.cop0.MapVAddr(Cop0::LOAD, address, paddr)) {
    regs.cop0.HandleTLBException(address);
    regs.cop0.FireException(regs.cop0.GetTLBExceptionCode(regs.cop0.tlbError, Cop0::LOAD), 0, regs.oldPC);
  } else {
    u32 shift = 8 * ((address ^ 0) & 3);
    u32 mask = 0xFFFFFFFF << shift;
    u32 data = mem.Read<u32>(regs, paddr & ~3);
    s32 result = s32((regs.gpr[RT(instr)] & ~mask) | (data << shift));
    if (RT(instr) != 0) [[likely]] {
      regs.gpr[RT(instr)] = result;
    }
  }
}

void Interpreter::lwr(u32 instr) {
  u64 address = regs.gpr[RS(instr)] + (s16)instr;
  u32 paddr = 0;
  if(!regs.cop0.MapVAddr(Cop0::LOAD, address, paddr)) {
    regs.cop0.HandleTLBException(address);
    regs.cop0.FireException(regs.cop0.GetTLBExceptionCode(regs.cop0.tlbError, Cop0::LOAD), 0, regs.oldPC);
  } else {
    u32 shift = 8 * ((address ^ 3) & 3);
    u32 mask = 0xFFFFFFFF >> shift;
    u32 data = mem.Read<u32>(regs, paddr & ~3);
    s32 result = s32((regs.gpr[RT(instr)] & ~mask) | (data >> shift));
    if (RT(instr) != 0) [[likely]] {
      regs.gpr[RT(instr)] = result;
    }
  }
}

void Interpreter::ld(u32 instr) {
  s64 address = regs.gpr[RS(instr)] + (s16)instr;
  if (check_address_error(0b111, address)) {
    regs.cop0.HandleTLBException(address);
    regs.cop0.FireException(ExceptionCode::AddressErrorLoad, 0, regs.oldPC);
    return;
  }

  u32 paddr = 0;
  if(!regs.cop0.MapVAddr(Cop0::LOAD, address, paddr)) {
    regs.cop0.HandleTLBException(address);
    regs.cop0.FireException(regs.cop0.GetTLBExceptionCode(regs.cop0.tlbError, Cop0::LOAD), 0, regs.oldPC);
  } else {
    s64 value = mem.Read<u64>(regs, paddr);
    if (RT(instr) != 0) [[likely]] {
      regs.gpr[RT(instr)] = value;
    }
  }
}

void Interpreter::lld(u32 instr) {
  if (!regs.cop0.is64BitAddressing && !regs.cop0.kernelMode) {
    regs.cop0.FireException(ExceptionCode::ReservedInstruction, 0, regs.oldPC);
    return;
  }

  u64 address = regs.gpr[RS(instr)] + (s16)instr;
  u32 paddr;
  if (!regs.cop0.MapVAddr(Cop0::LOAD, address, paddr)) {
    regs.cop0.HandleTLBException(address);
    regs.cop0.FireException(regs.cop0.GetTLBExceptionCode(regs.cop0.tlbError, Cop0::LOAD), 0, regs.oldPC);
  } else {
    if (check_address_error(0b111, address)) {
      regs.cop0.FireException(ExceptionCode::AddressErrorLoad, 0, regs.oldPC);
    } else {
      if (RT(instr) != 0) [[likely]] {
        regs.gpr[RT(instr)] = mem.Read<u64>(regs, paddr);
      }
      regs.cop0.llbit = true;
      regs.cop0.LLAddr = paddr >> 4;
    }
  }
}

void Interpreter::ldl(u32 instr) {
  u64 address = regs.gpr[RS(instr)] + (s16)instr;
  u32 paddr = 0;
  if (!regs.cop0.MapVAddr(Cop0::LOAD, address, paddr)) {
    regs.cop0.HandleTLBException(address);
    regs.cop0.FireException(regs.cop0.GetTLBExceptionCode(regs.cop0.tlbError, Cop0::LOAD), 0, regs.oldPC);
  } else {
    s32 shift = 8 * ((address ^ 0) & 7);
    u64 mask = 0xFFFFFFFFFFFFFFFF << shift;
    u64 data = mem.Read<u64>(regs, paddr & ~7);
    s64 result = (s64) ((regs.gpr[RT(instr)] & ~mask) | (data << shift));
    if (RT(instr) != 0) [[likely]] {
      regs.gpr[RT(instr)] = result;
    }
  }
}

void Interpreter::ldr(u32 instr) {
  u64 address = regs.gpr[RS(instr)] + (s16)instr;
  u32 paddr;
  if (!regs.cop0.MapVAddr(Cop0::LOAD, address, paddr)) {
    regs.cop0.HandleTLBException(address);
    regs.cop0.FireException(regs.cop0.GetTLBExceptionCode(regs.cop0.tlbError, Cop0::LOAD), 0, regs.oldPC);
  } else {
    s32 shift = 8 * ((address ^ 7) & 7);
    u64 mask = 0xFFFFFFFFFFFFFFFF >> shift;
    u64 data = mem.Read<u64>(regs, paddr & ~7);
    s64 result = (s64) ((regs.gpr[RT(instr)] & ~mask) | (data >> shift));
    if (RT(instr) != 0) [[likely]] {
      regs.gpr[RT(instr)] = result;
    }
  }
}

void Interpreter::lbu(u32 instr) {
  u64 address = regs.gpr[RS(instr)] + (s16)instr;
  u32 paddr;
  if (!regs.cop0.MapVAddr(Cop0::LOAD, address, paddr)) {
    regs.cop0.HandleTLBException(address);
    regs.cop0.FireException(regs.cop0.GetTLBExceptionCode(regs.cop0.tlbError, Cop0::LOAD), 0, regs.oldPC);
  } else {
    u8 value = mem.Read<u8>(regs, paddr);
    if (RT(instr) != 0) [[likely]] {
      regs.gpr[RT(instr)] = value;
    }
  }
}

void Interpreter::lhu(u32 instr) {
  s64 address = regs.gpr[RS(instr)] + (s16)instr;
  if (check_address_error(0b1, address)) {
    regs.cop0.HandleTLBException(address);
    regs.cop0.FireException(ExceptionCode::AddressErrorLoad, 0, regs.oldPC);
    return;
  }
  u32 paddr;
  if (!regs.cop0.MapVAddr(Cop0::LOAD, address, paddr)) {
    regs.cop0.HandleTLBException(address);
    regs.cop0.FireException(regs.cop0.GetTLBExceptionCode(regs.cop0.tlbError, Cop0::LOAD), 0, regs.oldPC);
  } else {
    u16 value = mem.Read<u16>(regs, paddr);
    if (RT(instr) != 0) [[likely]] {
      regs.gpr[RT(instr)] = value;
    }
  }
}

void Interpreter::lwu(u32 instr) {
  s64 address = regs.gpr[RS(instr)] + (s16)instr;
  if (check_address_error(0b11, address)) {
    regs.cop0.HandleTLBException(address);
    regs.cop0.FireException(ExceptionCode::AddressErrorLoad, 0, regs.oldPC);
    return;
  }

  u32 paddr;
  if (!regs.cop0.MapVAddr(Cop0::LOAD, address, paddr)) {
    regs.cop0.HandleTLBException(address);
    regs.cop0.FireException(regs.cop0.GetTLBExceptionCode(regs.cop0.tlbError, Cop0::LOAD), 0, regs.oldPC);
  } else {
    u32 value = mem.Read<u32>(regs, paddr);
    if (RT(instr) != 0) [[likely]] {
      regs.gpr[RT(instr)] = value;
    }
  }
}

void Interpreter::sb(u32 instr) {
  u64 address = regs.gpr[RS(instr)] + (s16)instr;
  u32 paddr;
  if (!regs.cop0.MapVAddr(Cop0::STORE, address, paddr)) {
    regs.cop0.HandleTLBException(address);
    regs.cop0.FireException(regs.cop0.GetTLBExceptionCode(regs.cop0.tlbError, Cop0::STORE), 0, regs.oldPC);
  } else {
    mem.Write<u8>(regs, paddr, regs.gpr[RT(instr)]);
  }
}

void Interpreter::sc(u32 instr) {
  u64 address = regs.gpr[RS(instr)] + (s16)instr;

  if(regs.cop0.llbit) {
    regs.cop0.llbit = false;

    if (check_address_error(0b11, address)) {
      regs.gpr[RT(instr)] = 0;
      regs.cop0.HandleTLBException(address);
      regs.cop0.FireException(ExceptionCode::AddressErrorStore, 0, regs.oldPC);
      return;
    }

    u32 paddr = 0;
    if(!regs.cop0.MapVAddr(Cop0::STORE, address, paddr)) {
      regs.gpr[RT(instr)] = 0;
      regs.cop0.HandleTLBException(address);
      regs.cop0.FireException(regs.cop0.GetTLBExceptionCode(regs.cop0.tlbError, Cop0::STORE), 0, regs.oldPC);
    } else {
      mem.Write<u32>(regs, paddr, regs.gpr[RT(instr)]);
      if (RT(instr) != 0) [[likely]] {
        regs.gpr[RT(instr)] = 1;
      }
    }
  } else {
    if (RT(instr) != 0) [[likely]] {
      regs.gpr[RT(instr)] = 0;
    }
  }
}

void Interpreter::scd(u32 instr) {
  if (!regs.cop0.is64BitAddressing && !regs.cop0.kernelMode) {
    regs.cop0.FireException(ExceptionCode::ReservedInstruction, 0, regs.oldPC);
    return;
  }

  s64 address = regs.gpr[RS(instr)] + (s16)instr;

  if(regs.cop0.llbit) {
    regs.cop0.llbit = false;

    if (check_address_error(0b111, address)) {
      regs.gpr[RT(instr)] = 0;
      regs.cop0.HandleTLBException(address);
      regs.cop0.FireException(ExceptionCode::AddressErrorStore, 0, regs.oldPC);
      return;
    }

    u32 paddr = 0;
    if(!regs.cop0.MapVAddr(Cop0::STORE, address, paddr)) {
      regs.gpr[RT(instr)] = 0;
      regs.cop0.HandleTLBException(address);
      regs.cop0.FireException(regs.cop0.GetTLBExceptionCode(regs.cop0.tlbError, Cop0::STORE), 0, regs.oldPC);
    } else {
      mem.Write<u32>(regs, paddr, regs.gpr[RT(instr)]);
      if (RT(instr) != 0) [[likely]] {
        regs.gpr[RT(instr)] = 1;
      }
    }
  } else {
    if (RT(instr) != 0) [[likely]] {
      regs.gpr[RT(instr)] = 0;
    }
  }
}

void Interpreter::sh(u32 instr) {
  s64 address = regs.gpr[RS(instr)] + (s16)instr;

  u32 physical;
  if(!regs.cop0.MapVAddr(Cop0::STORE, address, physical)) {
    regs.cop0.HandleTLBException(address);
    regs.cop0.FireException(regs.cop0.GetTLBExceptionCode(regs.cop0.tlbError, Cop0::STORE), 0, regs.oldPC);
  } else {
    mem.Write<u16>(regs, physical, regs.gpr[RT(instr)]);
  }
}

void Interpreter::sw(u32 instr) {
  s16 offset = instr;
  u64 address = regs.gpr[RS(instr)] + offset;
  if (check_address_error(0b11, address)) {
    regs.cop0.HandleTLBException(address);
    regs.cop0.FireException(ExceptionCode::AddressErrorStore, 0, regs.oldPC);
    return;
  }

  u32 physical;
  if(!regs.cop0.MapVAddr(Cop0::STORE, address, physical)) {
    regs.cop0.HandleTLBException(address);
    regs.cop0.FireException(regs.cop0.GetTLBExceptionCode(regs.cop0.tlbError, Cop0::STORE), 0, regs.oldPC);
  } else {
    mem.Write<u32>(regs, physical, regs.gpr[RT(instr)]);
  }
}

void Interpreter::sd(u32 instr) {
  s64 address = regs.gpr[RS(instr)] + (s16)instr;
  if (check_address_error(0b111, address)) {
    regs.cop0.HandleTLBException(address);
    regs.cop0.FireException(ExceptionCode::AddressErrorStore, 0, regs.oldPC);
    return;
  }

  u32 physical;
  if(!regs.cop0.MapVAddr(Cop0::STORE, address, physical)) {
    regs.cop0.HandleTLBException(address);
    regs.cop0.FireException(regs.cop0.GetTLBExceptionCode(regs.cop0.tlbError, Cop0::STORE), 0, regs.oldPC);
  } else {
    mem.Write(regs, physical, regs.gpr[RT(instr)]);
  }
}

void Interpreter::sdl(u32 instr) {
  u64 address = regs.gpr[RS(instr)] + (s16)instr;
  u32 paddr;
  if (!regs.cop0.MapVAddr(Cop0::STORE, address, paddr)) {
    regs.cop0.HandleTLBException(address);
    regs.cop0.FireException(regs.cop0.GetTLBExceptionCode(regs.cop0.tlbError, Cop0::STORE), 0, regs.oldPC);
  } else {
    s32 shift = 8 * ((address ^ 0) & 7);
    u64 mask = 0xFFFFFFFFFFFFFFFF >> shift;
    u64 data = mem.Read<u64>(regs, paddr & ~7);
    u64 rt = regs.gpr[RT(instr)];
    mem.Write(regs, paddr & ~7, (data & ~mask) | (rt >> shift));
  }
}

void Interpreter::sdr(u32 instr) {
  u64 address = regs.gpr[RS(instr)] + (s16)instr;
  u32 paddr;
  if (!regs.cop0.MapVAddr(Cop0::STORE, address, paddr)) {
    regs.cop0.HandleTLBException(address);
    regs.cop0.FireException(regs.cop0.GetTLBExceptionCode(regs.cop0.tlbError, Cop0::STORE), 0, regs.oldPC);
  } else {
    s32 shift = 8 * ((address ^ 7) & 7);
    u64 mask = 0xFFFFFFFFFFFFFFFF << shift;
    u64 data = mem.Read<u64>(regs, paddr & ~7);
    u64 rt = regs.gpr[RT(instr)];
    mem.Write(regs, paddr & ~7, (data & ~mask) | (rt << shift));
  }
}

void Interpreter::swl(u32 instr) {
  u64 address = regs.gpr[RS(instr)] + (s16)instr;
  u32 paddr;
  if (!regs.cop0.MapVAddr(Cop0::STORE, address, paddr)) {
    regs.cop0.HandleTLBException(address);
    regs.cop0.FireException(regs.cop0.GetTLBExceptionCode(regs.cop0.tlbError, Cop0::STORE), 0, regs.oldPC);
  } else {
    u32 shift = 8 * ((address ^ 0) & 3);
    u32 mask = 0xFFFFFFFF >> shift;
    u32 data = mem.Read<u32>(regs, paddr & ~3);
    u32 rt = regs.gpr[RT(instr)];
    mem.Write<u32>(regs, paddr & ~3, (data & ~mask) | (rt >> shift));
  }
}

void Interpreter::swr(u32 instr) {
  u64 address = regs.gpr[RS(instr)] + (s16)instr;
  u32 paddr;
  if (!regs.cop0.MapVAddr(Cop0::STORE, address, paddr)) {
    regs.cop0.HandleTLBException(address);
    regs.cop0.FireException(regs.cop0.GetTLBExceptionCode(regs.cop0.tlbError, Cop0::STORE), 0, regs.oldPC);
  } else {
    u32 shift = 8 * ((address ^ 3) & 3);
    u32 mask = 0xFFFFFFFF << shift;
    u32 data = mem.Read<u32>(regs, paddr & ~3);
    u32 rt = regs.gpr[RT(instr)];
    mem.Write<u32>(regs, paddr & ~3, (data & ~mask) | (rt << shift));
  }
}

void Interpreter::ori(u32 instr) {
  s64 imm = (u16)instr;
  s64 result = imm | regs.gpr[RS(instr)];
  if (RT(instr) != 0) [[likely]] {
    regs.gpr[RT(instr)] = result;
  }
}

void Interpreter::or_(u32 instr) {
  if (RD(instr) != 0) [[likely]] {
    regs.gpr[RD(instr)] = regs.gpr[RS(instr)] | regs.gpr[RT(instr)];
  }
}

void Interpreter::nor(u32 instr) {
  if (RD(instr) != 0) [[likely]] {
    regs.gpr[RD(instr)] = ~(regs.gpr[RS(instr)] | regs.gpr[RT(instr)]);
  }
}

void Interpreter::j(u32 instr) {
  s32 target = (instr & 0x3ffffff) << 2;
  s64 address = (regs.oldPC & ~0xfffffff) | target;

  branch(true, address);
}

void Interpreter::jal(u32 instr) {
  regs.gpr[31] = regs.nextPC;
  j(instr);
}

void Interpreter::jalr(u32 instr) {
  u64 addr = regs.gpr[RS(instr)];
  branch(true, addr);
  if (RD(instr) != 0) [[likely]] {
    regs.gpr[RD(instr)] = regs.pc + 4;
  }
}

void Interpreter::jr(u32 instr) {
  u64 address = regs.gpr[RS(instr)];
  branch(true, address);
}

void Interpreter::slti(u32 instr) {
  s16 imm = instr;
  if (RT(instr) != 0) [[likely]] {
    regs.gpr[RT(instr)] = regs.gpr[RS(instr)] < imm;
  }
}

void Interpreter::sltiu(u32 instr) {
  s16 imm = instr;
  if (RT(instr) != 0) [[likely]] {
    regs.gpr[RT(instr)] = (u64) regs.gpr[RS(instr)] < imm;
  }
}

void Interpreter::slt(u32 instr) {
  if (RD(instr) != 0) [[likely]] {
    regs.gpr[RD(instr)] = regs.gpr[RS(instr)] < regs.gpr[RT(instr)];
  }
}

void Interpreter::sltu(u32 instr) {
  if (RD(instr) != 0) [[likely]] {
    regs.gpr[RD(instr)] = (u64) regs.gpr[RS(instr)] < (u64) regs.gpr[RT(instr)];
  }
}

void Interpreter::xori(u32 instr) {
  s64 imm = (u16)instr;
  if (RT(instr) != 0) [[likely]] {
    regs.gpr[RT(instr)] = regs.gpr[RS(instr)] ^ imm;
  }
}

void Interpreter::xor_(u32 instr) {
  if (RD(instr) != 0) [[likely]] {
    regs.gpr[RD(instr)] = regs.gpr[RT(instr)] ^ regs.gpr[RS(instr)];
  }
}

void Interpreter::andi(u32 instr) {
  s64 imm = (u16)instr;
  if (RT(instr) != 0) [[likely]] {
    regs.gpr[RT(instr)] = regs.gpr[RS(instr)] & imm;
  }
}

void Interpreter::and_(u32 instr) {
  if (RD(instr) != 0) [[likely]] {
    regs.gpr[RD(instr)] = regs.gpr[RS(instr)] & regs.gpr[RT(instr)];
  }
}

void Interpreter::sll(u32 instr) {
  if (RD(instr) != 0) [[likely]] {
    u8 sa = ((instr >> 6) & 0x1f);
    s32 result = regs.gpr[RT(instr)] << sa;
    regs.gpr[RD(instr)] = (s64) result;
  }
}

void Interpreter::sllv(u32 instr) {
  if (RD(instr) != 0) [[likely]] {
    u8 sa = (regs.gpr[RS(instr)]) & 0x1F;
    u32 rt = regs.gpr[RT(instr)];
    s32 result = rt << sa;
    regs.gpr[RD(instr)] = (s64) result;
  }
}

void Interpreter::dsll32(u32 instr) {
  if (RD(instr) != 0) [[likely]] {
    u8 sa = ((instr >> 6) & 0x1f);
    s64 result = regs.gpr[RT(instr)] << (sa + 32);
    regs.gpr[RD(instr)] = result;
  }
}

void Interpreter::dsll(u32 instr) {
  if (RD(instr) != 0) [[likely]] {
    u8 sa = ((instr >> 6) & 0x1f);
    s64 result = regs.gpr[RT(instr)] << sa;
    regs.gpr[RD(instr)] = result;
  }
}

void Interpreter::dsllv(u32 instr) {
  if (RD(instr) != 0) [[likely]] {
    s64 sa = regs.gpr[RS(instr)] & 63;
    s64 result = regs.gpr[RT(instr)] << sa;
    regs.gpr[RD(instr)] = result;
  }
}

void Interpreter::srl(u32 instr) {
  if (RD(instr) != 0) [[likely]] {
    u32 rt = regs.gpr[RT(instr)];
    u8 sa = ((instr >> 6) & 0x1f);
    u32 result = rt >> sa;
    regs.gpr[RD(instr)] = (s32) result;
  }
}

void Interpreter::srlv(u32 instr) {
  if (RD(instr) != 0) [[likely]] {
    u8 sa = (regs.gpr[RS(instr)] & 0x1F);
    u32 rt = regs.gpr[RT(instr)];
    s32 result = rt >> sa;
    regs.gpr[RD(instr)] = (s64) result;
  }
}

void Interpreter::dsrl(u32 instr) {
  if (RD(instr) != 0) [[likely]] {
    u64 rt = regs.gpr[RT(instr)];
    u8 sa = ((instr >> 6) & 0x1f);
    u64 result = rt >> sa;
    regs.gpr[RD(instr)] = s64(result);
  }
}

void Interpreter::dsrlv(u32 instr) {
  if (RD(instr) != 0) [[likely]] {
    u8 amount = (regs.gpr[RS(instr)] & 63);
    u64 rt = regs.gpr[RT(instr)];
    u64 result = rt >> amount;
    regs.gpr[RD(instr)] = s64(result);
  }
}

void Interpreter::dsrl32(u32 instr) {
  if (RD(instr) != 0) [[likely]] {
    u64 rt = regs.gpr[RT(instr)];
    u8 sa = ((instr >> 6) & 0x1f);
    u64 result = rt >> (sa + 32);
    regs.gpr[RD(instr)] = s64(result);
  }
}

void Interpreter::sra(u32 instr) {
  if (RD(instr) != 0) [[likely]] {
    s64 rt = regs.gpr[RT(instr)];
    u8 sa = ((instr >> 6) & 0x1f);
    s32 result = rt >> sa;
    regs.gpr[RD(instr)] = result;
  }
}

void Interpreter::srav(u32 instr) {
  if (RD(instr) != 0) [[likely]] {
    s64 rs = regs.gpr[RS(instr)];
    s64 rt = regs.gpr[RT(instr)];
    u8 sa = rs & 0x1f;
    s32 result = rt >> sa;
    regs.gpr[RD(instr)] = result;
  }
}

void Interpreter::dsra(u32 instr) {
  if (RD(instr) != 0) [[likely]] {
    s64 rt = regs.gpr[RT(instr)];
    u8 sa = ((instr >> 6) & 0x1f);
    s64 result = rt >> sa;
    regs.gpr[RD(instr)] = result;
  }
}

void Interpreter::dsrav(u32 instr) {
  if (RD(instr) != 0) [[likely]] {
    s64 rt = regs.gpr[RT(instr)];
    s64 rs = regs.gpr[RS(instr)];
    s64 sa = rs & 63;
    s64 result = rt >> sa;
    regs.gpr[RD(instr)] = result;
  }
}

void Interpreter::dsra32(u32 instr) {
  if (RD(instr) != 0) [[likely]] {
    s64 rt = regs.gpr[RT(instr)];
    u8 sa = ((instr >> 6) & 0x1f);
    s64 result = rt >> (sa + 32);
    regs.gpr[RD(instr)] = result;
  }
}

void Interpreter::dsub(u32 instr) {
  s64 rt = regs.gpr[RT(instr)];
  s64 rs = regs.gpr[RS(instr)];
  s64 result = rs - rt;
  if(check_signed_underflow(rs, rt, result)) {
    regs.cop0.FireException(ExceptionCode::Overflow, 0, regs.oldPC);
  } else {
    if (RD(instr) != 0) [[likely]] {
      regs.gpr[RD(instr)] = result;
    }
  }
}

void Interpreter::dsubu(u32 instr) {
  if (RD(instr) != 0) [[likely]] {
    u64 rt = regs.gpr[RT(instr)];
    u64 rs = regs.gpr[RS(instr)];
    u64 result = rs - rt;
    regs.gpr[RD(instr)] = s64(result);
  }
}

void Interpreter::sub(u32 instr) {
  s32 rt = regs.gpr[RT(instr)];
  s32 rs = regs.gpr[RS(instr)];
  s32 result = rs - rt;
  if(check_signed_underflow(rs, rt, result)) {
    regs.cop0.FireException(ExceptionCode::Overflow, 0, regs.oldPC);
  } else {
    if (RD(instr) != 0) [[likely]] {
      regs.gpr[RD(instr)] = result;
    }
  }
}

void Interpreter::subu(u32 instr) {
  if (RD(instr) != 0) [[likely]] {
    u32 rt = regs.gpr[RT(instr)];
    u32 rs = regs.gpr[RS(instr)];
    u32 result = rs - rt;
    regs.gpr[RD(instr)] = (s64) ((s32) result);
  }
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
  if (RD(instr) != 0) [[likely]] {
    regs.gpr[RD(instr)] = regs.lo;
  }
}

void Interpreter::mfhi(u32 instr) {
  if (RD(instr) != 0) [[likely]] {
    regs.gpr[RD(instr)] = regs.hi;
  }
}

void Interpreter::mtlo(u32 instr) {
  regs.lo = regs.gpr[RS(instr)];
}

void Interpreter::mthi(u32 instr) {
  regs.hi = regs.gpr[RS(instr)];
}

void Interpreter::trap(bool cond) {
  if(cond) {
    regs.cop0.FireException(ExceptionCode::Trap, 0, regs.oldPC);
  }
}

void Interpreter::mtc2(u32 instr) {
  cop2Latch = regs.gpr[RT(instr)];
}

void Interpreter::mfc2(u32 instr) {
  s32 value = cop2Latch;
  if (RT(instr) != 0) [[likely]] {
    regs.gpr[RT(instr)] = value;
  }
}

void Interpreter::dmtc2(u32 instr) {
  cop2Latch = regs.gpr[RT(instr)];
}

void Interpreter::dmfc2(u32 instr) {
  if (RT(instr) != 0) [[likely]] {
    regs.gpr[RT(instr)] = cop2Latch;
  }
}

void Interpreter::ctc2(u32) {

}

void Interpreter::cfc2(u32) {

}
}