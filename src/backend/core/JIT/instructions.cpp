#include <core/JIT.hpp>

#define check_address_error(mask, vaddr) (((!regs.cop0.is_64bit_addressing) && (s32)(vaddr) != (vaddr)) || (((vaddr) & (mask)) != 0))
#define check_signed_overflow(op1, op2, res) (((~((op1) ^ (op2)) & ((op1) ^ (res))) >> ((sizeof(res) * 8) - 1)) & 1)
#define check_signed_underflow(op1, op2, res) (((((op1) ^ (op2)) & ((op1) ^ (res))) >> ((sizeof(res) * 8) - 1)) & 1)

namespace n64 {
void JIT::add(u32 instr) {
  auto dst = Entry::Operand{ Entry::Operand::REG_S64, u8(RD(instr)) };
  auto op1 = Entry::Operand{ Entry::Operand::REG_U32, u8(RS(instr)) };
  auto op2 = Entry::Operand{ Entry::Operand::REG_U32, u8(RT(instr)) };
  Entry e(Entry::ADD, dst, op1, op2);
  ir.push(e);
}

void JIT::addu(u32 instr) {
  add(instr);
}

void JIT::addi(u32 instr) {
  auto dst = Entry::Operand{ Entry::Operand::REG_S64, u8(RT(instr)) };
  auto op1 = Entry::Operand{ Entry::Operand::REG_U32, u8(RS(instr)) };
  auto op2 = Entry::Operand{ Entry::Operand::IMM_S16, s16(instr) };
  Entry e(Entry::ADD, dst, op1, op2);
  ir.push(e);
}

void JIT::addiu(u32 instr) {
  addi(instr);
}

void JIT::dadd(u32 instr) {
  auto dst = Entry::Operand{ Entry::Operand::REG_S64, u8(RD(instr)) };
  auto op1 = Entry::Operand{ Entry::Operand::REG_U64, u8(RS(instr)) };
  auto op2 = Entry::Operand{ Entry::Operand::REG_U64, u8(RT(instr)) };
  Entry e(Entry::ADD, dst, op1, op2);
  ir.push(e);
}

void JIT::bltz(u32 instr) {
  auto op1 = Entry::Operand{ Entry::Operand::REG_S64, u8(RS(instr)) };
  auto op2 = Entry::Operand{ Entry::Operand::IMM_U64, 0 };
  Entry e(Entry::BRANCH, op1, Entry::BranchCond::LT, op2);
  ir.push(e);
}

void JIT::bgez(u32 instr) {
  auto op1 = Entry::Operand{ Entry::Operand::REG_S64, u8(RS(instr)) };
  auto op2 = Entry::Operand{ Entry::Operand::IMM_U64, 0 };
  Entry e(Entry::BRANCH, op1, Entry::BranchCond::GE, op2);
  ir.push(e);
}

void JIT::bltzl(u32 instr) {
  auto op1 = Entry::Operand{ Entry::Operand::REG_S64, u8(RS(instr)) };
  auto op2 = Entry::Operand{ Entry::Operand::IMM_U64, 0 };
  auto opc = Entry::Opcode(u16(Entry::BRANCH) | Entry::LIKELY);
  Entry e(opc, op1, Entry::BranchCond::LT, op2);
  ir.push(e);
}

void JIT::bgezl(u32 instr) {
  auto op1 = Entry::Operand{ Entry::Operand::REG_S64, u8(RS(instr)) };
  auto op2 = Entry::Operand{ Entry::Operand::IMM_U64, 0 };
  auto opc = Entry::Opcode(u16(Entry::BRANCH) | Entry::LIKELY);
  Entry e(opc, op1, Entry::BranchCond::GE, op2);
  ir.push(e);
}

void JIT::bltzal(u32 instr) {
  auto op1 = Entry::Operand{ Entry::Operand::REG_S64, u8(RS(instr)) };
  auto op2 = Entry::Operand{ Entry::Operand::IMM_U64, 0 };
  auto opc = Entry::Opcode(u16(Entry::BRANCH) | Entry::LINK);
  Entry e(opc, op1, Entry::BranchCond::LT, op2);
  ir.push(e);
}

void JIT::bgezal(u32 instr) {
  auto op1 = Entry::Operand{ Entry::Operand::REG_S64, u8(RS(instr)) };
  auto op2 = Entry::Operand{ Entry::Operand::IMM_U64, 0 };
  auto opc = Entry::Opcode(u16(Entry::BRANCH) | Entry::LINK);
  Entry e(opc, op1, Entry::BranchCond::GE, op2);
  ir.push(e);
}

void JIT::bltzall(u32 instr) {
  auto op1 = Entry::Operand{ Entry::Operand::REG_S64, u8(RS(instr)) };
  auto op2 = Entry::Operand{ Entry::Operand::IMM_U64, 0 };
  auto opc = Entry::Opcode(u16(Entry::BRANCH) | Entry::LINK | Entry::LIKELY);
  Entry e(opc, op1, Entry::BranchCond::LT, op2);
  ir.push(e);
}

void JIT::bgezall(u32 instr) {
  auto op1 = Entry::Operand{ Entry::Operand::REG_S64, u8(RS(instr)) };
  auto op2 = Entry::Operand{ Entry::Operand::IMM_U64, 0 };
  auto opc = Entry::Opcode(u16(Entry::BRANCH) | Entry::LINK | Entry::LIKELY);
  Entry e(opc, op1, Entry::BranchCond::GE, op2);
  ir.push(e);
}

void JIT::beq(u32 instr) {
  auto op1 = Entry::Operand{ Entry::Operand::REG_S64, u8(RS(instr)) };
  auto op2 = Entry::Operand{ Entry::Operand::REG_S64, u8(RT(instr)) };
  Entry e(Entry::BRANCH, op1, Entry::BranchCond::EQ, op2);
  ir.push(e);
}

void JIT::bne(u32 instr) {
  auto op1 = Entry::Operand{ Entry::Operand::REG_S64, u8(RS(instr)) };
  auto op2 = Entry::Operand{ Entry::Operand::REG_S64, u8(RT(instr)) };
  Entry e(Entry::BRANCH, op1, Entry::BranchCond::NE, op2);
  ir.push(e);
}

void JIT::blez(u32 instr) {
  auto op1 = Entry::Operand{ Entry::Operand::REG_S64, u8(RS(instr)) };
  auto op2 = Entry::Operand{ Entry::Operand::IMM_S64, 0 };
  Entry e(Entry::BRANCH, op1, Entry::BranchCond::LE, op2);
  ir.push(e);
}

void JIT::bgtz(u32 instr) {
  auto op1 = Entry::Operand{ Entry::Operand::REG_S64, u8(RS(instr)) };
  auto op2 = Entry::Operand{ Entry::Operand::IMM_S64, 0 };
  Entry e(Entry::BRANCH, op1, Entry::BranchCond::GT, op2);
  ir.push(e);
}

void JIT::beql(u32 instr) {
  auto op1 = Entry::Operand{ Entry::Operand::REG_S64, u8(RS(instr)) };
  auto op2 = Entry::Operand{ Entry::Operand::REG_S64, u8(RT(instr)) };
  auto opc = Entry::Opcode(u16(Entry::BRANCH) | Entry::LIKELY);
  Entry e(opc, op1, Entry::BranchCond::EQ, op2);
  ir.push(e);
}

void JIT::bnel(u32 instr) {
  auto op1 = Entry::Operand{ Entry::Operand::REG_S64, u8(RS(instr)) };
  auto op2 = Entry::Operand{ Entry::Operand::REG_S64, u8(RT(instr)) };
  auto opc = Entry::Opcode(u16(Entry::BRANCH) | Entry::LIKELY);
  Entry e(opc, op1, Entry::BranchCond::NE, op2);
  ir.push(e);
}

void JIT::blezl(u32 instr) {
  auto op1 = Entry::Operand{ Entry::Operand::REG_S64, u8(RS(instr)) };
  auto op2 = Entry::Operand{ Entry::Operand::IMM_S64, 0 };
  auto opc = Entry::Opcode(u16(Entry::BRANCH) | Entry::LIKELY);
  Entry e(opc, op1, Entry::BranchCond::LE, op2);
  ir.push(e);
}

void JIT::bgtzl(u32 instr) {
  auto op1 = Entry::Operand{ Entry::Operand::REG_S64, u8(RS(instr)) };
  auto op2 = Entry::Operand{ Entry::Operand::IMM_S64, 0 };
  auto opc = Entry::Opcode(u16(Entry::BRANCH) | Entry::LIKELY);
  Entry e(opc, op1, Entry::BranchCond::GT, op2);
  ir.push(e);
}

void JIT::daddu(u32 instr) {
  dadd(instr);
}

void JIT::daddi(u32 instr) {
  auto dst = Entry::Operand{ Entry::Operand::REG_S64, u8(RT(instr)) };
  auto op1 = Entry::Operand{ Entry::Operand::REG_S64, u8(RS(instr)) };
  auto op2 = Entry::Operand{ Entry::Operand::IMM_S16, s16(instr) };
  Entry e(Entry::ADD, dst, op1, op2);
  ir.push(e);
}

void JIT::daddiu(u32 instr) {
  daddi(instr);
}

void JIT::div(u32 instr) {
  
}

void JIT::divu(u32 instr) {
  movsxd(rax, GPR(dword, RS(instr))); // dividend
  movsxd(rcx, GPR(dword, RT(instr))); // divisor
  cmp(rcx, 0);
  je("divu_divisor==0");

  CodeGenerator::div(rcx);
  mov(qword[rdi + offsetof(Registers, lo)], eax);
  mov(qword[rdi + offsetof(Registers, hi)], edx);
  jmp("divu_exit");

  L("divu_divisor==0");
    mov(qword[rdi + offsetof(Registers, hi)], rax);
    mov(qword[rdi + offsetof(Registers, lo)], -1);

  L("divu_exit");
}

void JIT::ddiv(u32 instr) {
  mov(rax, GPR(qword, RS(instr)));
  mov(rcx, GPR(qword, RT(instr)));
  mov(r8, 0x8000000000000000);
  mov(r9, rax);
  CodeGenerator::xor_(r9, r8);
  mov(r8, 0xFFFFFFFFFFFFFFFF);
  mov(r10, rcx);
  CodeGenerator::xor_(r10, r8);
  CodeGenerator::xor_(r9, r10);
  cmp(rcx, 0);
  je("ddiv_else_if");
  cmp(r9, 1);
  jne("ddiv_else");
  mov(qword[rdi + offsetof(Registers, lo)], rax);
  mov(qword[rdi + offsetof(Registers, hi)], 0);
  jmp("ddiv_exit");
  L("ddiv_else_if");
    mov(qword[rdi + offsetof(Registers, hi)], rax);
    cmp(rax, 0);
    jge("ddiv_dividend>=0");
    mov(qword[rdi + offsetof(Registers, lo)], 1);
    L("ddiv_dividend>=0");
      mov(qword[rdi + offsetof(Registers, lo)], -1);
  L("ddiv_else");
    CodeGenerator::div(rcx);
    mov(qword[rdi + offsetof(Registers, lo)], rax);
    mov(qword[rdi + offsetof(Registers, hi)], rdx);
  L("ddiv_exit");
}

void JIT::ddivu(u32 instr) {
  mov(rax, GPR(qword, RS(instr)));
  mov(rcx, GPR(qword, RT(instr)));
  cmp(rcx, 0);
  je("ddivu_divisor==0");
  CodeGenerator::div(rcx);
  mov(qword[rdi + offsetof(Registers, lo)], rax);
  mov(qword[rdi + offsetof(Registers, hi)], rdx);
  jmp("ddivu_exit");
  L("ddivu_divisor==0");
    mov(qword[rdi + offsetof(Registers, lo)], -1);
    mov(qword[rdi + offsetof(Registers, hi)], rax);
  L("ddivu_exit");
}

void JIT::lui(u32 instr) {
  u64 val = s64(s16(instr));
  val <<= 16;
  mov(GPR(qword, RT(instr)), val);
}

void JIT::lb(u32 instr) {
  mov(rdx, GPR(qword, RS(instr)));
  CodeGenerator::add(rdx, s64(s16(instr)));
  mov(rsi, LOAD);
  push(rcx);
  lea(rcx, dword[rbp-4]);
  emitCall(MapVAddr);
  pop(rcx);
  cmp(rax, 0);
  je("lb_exception");
  mov(rsi, dword[rbp-4]);
  push(rcx);
  emitMemberCall(&Mem::Read<u8>, &mem);
  pop(rcx);
  mov(GPR(qword, RT(instr)), rax.cvt8());
  L("lb_exception");
  mov(rsi, rdx);
  emitCall(HandleTLBException);
  push(rsi);
  mov(rdi, REG(byte, cop0.tlbError));
  mov(rsi, LOAD);
  emitCall(GetTLBExceptionCode);
  pop(rsi);
  mov(rsi, rax);
  mov(rdx, 0);
  mov(rcx, 1);
  emitCall(FireException);
}

void JIT::lh(u32 instr) {
  u64 address = regs.gpr[RS(instr)] + (s16)instr;
  if ((address & 0b1) > 0) {
    HandleTLBException(regs, address);
    FireException(regs, ExceptionCode::AddressErrorLoad, 0, regs.oldPC);
    return;
  }

  u32 paddr = 0;
  if(!MapVAddr(regs, LOAD, address, paddr)) {
    HandleTLBException(regs, address);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, LOAD), 0, regs.oldPC);
  } else {
    regs.gpr[RT(instr)] = (s16)mem.Read<u16>(regs, paddr);
  }
}

void JIT::lw(u32 instr) {
  s16 offset = instr;
  u64 address = regs.gpr[RS(instr)] + offset;
  if (check_address_error(0b11, address)) {
    HandleTLBException(regs, address);
    FireException(regs, ExceptionCode::AddressErrorLoad, 0, regs.oldPC);
    return;
  }

  u32 physical = 0;
  if (!MapVAddr(regs, LOAD, address, physical)) {
    HandleTLBException(regs, address);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, LOAD), 0, regs.oldPC);
  } else {
    regs.gpr[RT(instr)] = (s32)mem.Read<u32>(regs, physical);
  }
}

void JIT::ll(u32 instr) {
  u64 address = regs.gpr[RS(instr)] + (s16)instr;
  u32 physical;
  if (!MapVAddr(regs, LOAD, address, physical)) {
    HandleTLBException(regs, address);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, LOAD), 0, regs.oldPC);
  } else {
    if ((address & 0b11) > 0) {
      FireException(regs, ExceptionCode::AddressErrorLoad, 0, regs.oldPC);
      return;
    } else {
      regs.gpr[RT(instr)] = (s32)mem.Read<u32>(regs, physical);
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
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, LOAD), 0, regs.oldPC);
  } else {
    u32 shift = 8 * ((address ^ 0) & 3);
    u32 mask = 0xFFFFFFFF << shift;
    u32 data = mem.Read<u32>(regs, paddr & ~3);
    s32 result = s32((regs.gpr[RT(instr)] & ~mask) | (data << shift));
    regs.gpr[RT(instr)] = result;
  }
}

void JIT::lwr(u32 instr) {
  u64 address = regs.gpr[RS(instr)] + (s16)instr;
  u32 paddr = 0;
  if(!MapVAddr(regs, LOAD, address, paddr)) {
    HandleTLBException(regs, address);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, LOAD), 0, regs.oldPC);
  } else {
    u32 shift = 8 * ((address ^ 3) & 3);
    u32 mask = 0xFFFFFFFF >> shift;
    u32 data = mem.Read<u32>(regs, paddr & ~3);
    s32 result = s32((regs.gpr[RT(instr)] & ~mask) | (data >> shift));
    regs.gpr[RT(instr)] = result;
  }
}

void JIT::ld(u32 instr) {
  s64 address = regs.gpr[RS(instr)] + (s16)instr;
  if (check_address_error(0b111, address)) {
    HandleTLBException(regs, address);
    FireException(regs, ExceptionCode::AddressErrorLoad, 0, regs.oldPC);
    return;
  }

  u32 paddr = 0;
  if(!MapVAddr(regs, LOAD, address, paddr)) {
    HandleTLBException(regs, address);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, LOAD), 0, regs.oldPC);
  } else {
    s64 value = mem.Read<u64>(regs, paddr);
    regs.gpr[RT(instr)] = value;
  }
}

void JIT::lld(u32 instr) {
  if (!regs.cop0.is_64bit_addressing && !regs.cop0.kernel_mode) {
    FireException(regs, ExceptionCode::ReservedInstruction, 0, regs.oldPC);
    return;
  }

  u64 address = regs.gpr[RS(instr)] + (s16)instr;
  u32 paddr;
  if (!MapVAddr(regs, LOAD, address, paddr)) {
    HandleTLBException(regs, address);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, LOAD), 0, regs.oldPC);
  } else {
    if ((address & 0b111) > 0) {
      FireException(regs, ExceptionCode::AddressErrorLoad, 0, regs.oldPC);
    } else {
      regs.gpr[RT(instr)] = mem.Read<u64>(regs, paddr);
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
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, LOAD), 0, regs.oldPC);
  } else {
    s32 shift = 8 * ((address ^ 0) & 7);
    u64 mask = 0xFFFFFFFFFFFFFFFF << shift;
    u64 data = mem.Read<u64>(regs, paddr & ~7);
    s64 result = (s64) ((regs.gpr[RT(instr)] & ~mask) | (data << shift));
    regs.gpr[RT(instr)] = result;
  }
}

void JIT::ldr(u32 instr) {
  u64 address = regs.gpr[RS(instr)] + (s16)instr;
  u32 paddr;
  if (!MapVAddr(regs, LOAD, address, paddr)) {
    HandleTLBException(regs, address);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, LOAD), 0, regs.oldPC);
  } else {
    s32 shift = 8 * ((address ^ 7) & 7);
    u64 mask = 0xFFFFFFFFFFFFFFFF >> shift;
    u64 data = mem.Read<u64>(regs, paddr & ~7);
    s64 result = (s64) ((regs.gpr[RT(instr)] & ~mask) | (data >> shift));
    regs.gpr[RT(instr)] = result;
  }
}

void JIT::lbu(u32 instr) {
  u64 address = regs.gpr[RS(instr)] + (s16)instr;
  u32 paddr;
  if (!MapVAddr(regs, LOAD, address, paddr)) {
    HandleTLBException(regs, address);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, LOAD), 0, regs.oldPC);
  } else {
    u8 value = mem.Read<u8>(regs, paddr);
    regs.gpr[RT(instr)] = value;
  }
}

void JIT::lhu(u32 instr) {
  s64 address = regs.gpr[RS(instr)] + (s16)instr;
  if ((address & 0b1) > 0) {
    HandleTLBException(regs, address);
    FireException(regs, ExceptionCode::AddressErrorLoad, 0, regs.oldPC);
    return;
  }
  u32 paddr;
  if (!MapVAddr(regs, LOAD, address, paddr)) {
    HandleTLBException(regs, address);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, LOAD), 0, regs.oldPC);
  } else {
    u16 value = mem.Read<u16>(regs, paddr);
    regs.gpr[RT(instr)] = value;
  }
}

void JIT::lwu(u32 instr) {
  s64 address = regs.gpr[RS(instr)] + (s16)instr;
  if ((address & 0b11) > 0) {
    HandleTLBException(regs, address);
    FireException(regs, ExceptionCode::AddressErrorLoad, 0, regs.oldPC);
    return;
  }

  u32 paddr;
  if (!MapVAddr(regs, LOAD, address, paddr)) {
    HandleTLBException(regs, address);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, LOAD), 0, regs.oldPC);
  } else {
    u32 value = mem.Read<u32>(regs, paddr);
    regs.gpr[RT(instr)] = value;
  }
}

void JIT::sb(u32 instr) {
  u64 address = regs.gpr[RS(instr)] + (s16)instr;
  u32 paddr;
  if (!MapVAddr(regs, STORE, address, paddr)) {
    HandleTLBException(regs, address);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, STORE), 0, regs.oldPC);
  } else {
    mem.Write<u8>(regs, paddr, regs.gpr[RT(instr)]);
  }
}

void JIT::sc(u32 instr) {
  u64 address = regs.gpr[RS(instr)] + (s16)instr;

  if ((address & 0b11) > 0) {
    FireException(regs, ExceptionCode::AddressErrorStore, 0, regs.oldPC);
    return;
  }

  if(regs.cop0.llbit) {
    regs.cop0.llbit = false;
    u32 paddr = 0;
    if(!MapVAddr(regs, STORE, address, paddr)) {
      HandleTLBException(regs, address);
      FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, STORE), 0, regs.oldPC);
    } else {
      mem.Write<u32>(regs, paddr, regs.gpr[RT(instr)]);
      regs.gpr[RT(instr)] = 1;
    }
  } else {
    regs.gpr[RT(instr)] = 0;
  }
}

void JIT::scd(u32 instr) {
  if (!regs.cop0.is_64bit_addressing && !regs.cop0.kernel_mode) {
    FireException(regs, ExceptionCode::ReservedInstruction, 0, regs.oldPC);
    return;
  }

  s64 address = regs.gpr[RS(instr)] + (s16)instr;
  if ((address & 0b111) > 0) {
    HandleTLBException(regs, address);
    FireException(regs, ExceptionCode::AddressErrorStore, 0, regs.oldPC);
    return;
  }

  if(regs.cop0.llbit) {
    regs.cop0.llbit = false;
    u32 paddr = 0;
    if(!MapVAddr(regs, STORE, address, paddr)) {
      HandleTLBException(regs, address);
      FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, STORE), 0, regs.oldPC);
    } else {
      mem.Write<u32>(regs, paddr, regs.gpr[RT(instr)]);
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
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, STORE), 0, regs.oldPC);
  } else {
    mem.Write<u16>(regs, physical, regs.gpr[RT(instr)]);
  }
}

void JIT::sw(u32 instr) {
  s16 offset = instr;
  u64 address = regs.gpr[RS(instr)] + offset;
  if (check_address_error(0b11, address)) {
    HandleTLBException(regs, address);
    FireException(regs, ExceptionCode::AddressErrorStore, 0, regs.oldPC);
    return;
  }

  u32 physical;
  if(!MapVAddr(regs, STORE, address, physical)) {
    HandleTLBException(regs, address);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, STORE), 0, regs.oldPC);
  } else {
    mem.Write<u32>(regs, physical, regs.gpr[RT(instr)]);
  }
}

void JIT::sd(u32 instr) {
  s64 address = regs.gpr[RS(instr)] + (s16)instr;
  if (check_address_error(0b111, address)) {
    HandleTLBException(regs, address);
    FireException(regs, ExceptionCode::AddressErrorStore, 0, regs.oldPC);
    return;
  }

  u32 physical;
  if(!MapVAddr(regs, STORE, address, physical)) {
    HandleTLBException(regs, address);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, STORE), 0, regs.oldPC);
  } else {
    mem.Write(regs, physical, regs.gpr[RT(instr)]);
  }
}

void JIT::sdl(u32 instr) {
  u64 address = regs.gpr[RS(instr)] + (s16)instr;
  u32 paddr;
  if (!MapVAddr(regs, STORE, address, paddr)) {
    HandleTLBException(regs, address);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, STORE), 0, regs.oldPC);
  } else {
    s32 shift = 8 * ((address ^ 0) & 7);
    u64 mask = 0xFFFFFFFFFFFFFFFF >> shift;
    u64 data = mem.Read<u64>(regs, paddr & ~7);
    u64 rt = regs.gpr[RT(instr)];
    mem.Write(regs, paddr & ~7, (data & ~mask) | (rt >> shift));
  }
}

void JIT::sdr(u32 instr) {
  u64 address = regs.gpr[RS(instr)] + (s16)instr;
  u32 paddr;
  if (!MapVAddr(regs, STORE, address, paddr)) {
    HandleTLBException(regs, address);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, STORE), 0, regs.oldPC);
  } else {
    s32 shift = 8 * ((address ^ 7) & 7);
    u64 mask = 0xFFFFFFFFFFFFFFFF << shift;
    u64 data = mem.Read<u64>(regs, paddr & ~7);
    u64 rt = regs.gpr[RT(instr)];
    mem.Write(regs, paddr & ~7, (data & ~mask) | (rt << shift));
  }
}

void JIT::swl(u32 instr) {
  u64 address = regs.gpr[RS(instr)] + (s16)instr;
  u32 paddr;
  if (!MapVAddr(regs, STORE, address, paddr)) {
    HandleTLBException(regs, address);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, STORE), 0, regs.oldPC);
  } else {
    u32 shift = 8 * ((address ^ 0) & 3);
    u32 mask = 0xFFFFFFFF >> shift;
    u32 data = mem.Read<u32>(regs, paddr & ~3);
    u32 rt = regs.gpr[RT(instr)];
    mem.Write<u32>(regs, paddr & ~3, (data & ~mask) | (rt >> shift));
  }
}

void JIT::swr(u32 instr) {
  u64 address = regs.gpr[RS(instr)] + (s16)instr;
  u32 paddr;
  if (!MapVAddr(regs, STORE, address, paddr)) {
    HandleTLBException(regs, address);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, STORE), 0, regs.oldPC);
  } else {
    u32 shift = 8 * ((address ^ 3) & 3);
    u32 mask = 0xFFFFFFFF << shift;
    u32 data = mem.Read<u32>(regs, paddr & ~3);
    u32 rt = regs.gpr[RT(instr)];
    mem.Write<u32>(regs, paddr & ~3, (data & ~mask) | (rt << shift));
  }
}

void JIT::ori(u32 instr) {
  s64 imm = (u16)instr;
  mov(rax, GPR(qword, RS(instr)));
  CodeGenerator::or_(rax, imm);
  mov(GPR(qword, RT(instr)), rax);
}

void JIT::or_(u32 instr) {
  if (RD(instr) != 0) [[likely]] {
    mov(rax, GPR(qword, RS(instr)));
    mov(rcx, GPR(qword, RT(instr)));
    CodeGenerator::or_(rax, rcx);
    mov(GPR(qword, RD(instr)), rax);
  }
}

void JIT::nor(u32 instr) {
  if (RD(instr) != 0) [[likely]] {
    mov(rax, GPR(qword, RS(instr)));
    mov(rcx, GPR(qword, RT(instr)));
    CodeGenerator::or_(rax, rcx);
    not_(rax);
    mov(GPR(qword, RD(instr)), rax);
  }
}

void JIT::j(u32 instr) {
  s32 target = (instr & 0x3ffffff) << 2;
  s64 address = (regs.oldPC & ~0xfffffff) | target;

  mov(byte[rdi + offsetof(Registers, delaySlot)], 1);
  mov(qword[rdi + offsetof(Registers, nextPC)], address);
}

void JIT::jal(u32 instr) {
  mov(rax, qword[rdi + offsetof(Registers, nextPC)]);
  mov(GPR(qword, 31), rax);
  j(instr);
}

void JIT::jalr(u32 instr) {
  mov(byte[rdi + offsetof(Registers, delaySlot)], 1);
  mov(rax, GPR(qword, RS(instr)));
  mov(qword[rdi + offsetof(Registers, nextPC)], rax);

  if (RD(instr) != 0) [[likely]] {
    mov(rax, qword[rdi + offsetof(Registers, pc)]);
    CodeGenerator::add(rax, 4);
    mov(GPR(qword, RD(instr)), rax);
  }
}

void JIT::slti(u32 instr) {
  mov(rax, s64(s16(instr)));
  mov(rcx, GPR(qword, RS(instr)));
  cmp(rcx, rax);
  setl(GPR(qword, RT(instr)));
}

void JIT::sltiu(u32 instr) {
  mov(rax, s64(s16(instr)));
  mov(rcx, GPR(qword, RS(instr)));
  cmp(rcx, rax);
  setb(GPR(qword, RT(instr)));
}

void JIT::slt(u32 instr) {
  if (RD(instr) != 0) [[likely]] {
    mov(rax, GPR(qword, RS(instr)));
    mov(rcx, GPR(qword, RT(instr)));
    cmp(rax, rcx);
    setl(GPR(qword, RD(instr)));
  }
}

void JIT::sltu(u32 instr) {
  if (RD(instr) != 0) [[likely]] {
    mov(rax, GPR(qword, RS(instr)));
    mov(rcx, GPR(qword, RT(instr)));
    cmp(rax, rcx);
    setb(GPR(qword, RD(instr)));
  }
}

void JIT::xori(u32 instr) {
  s64 imm = (u16)instr;
  mov(rax, imm);
  mov(rcx, GPR(qword, RS(instr)));
  CodeGenerator::xor_(rcx, rax);
  mov(GPR(qword, RT(instr)), rcx);
}

void JIT::xor_(u32 instr) {
  if (RD(instr) != 0) [[likely]] {
    mov(rax, GPR(qword, RT(instr)));
    mov(rcx, GPR(qword, RS(instr)));
    CodeGenerator::xor_(rax, rcx);
    mov(GPR(qword, RD(instr)), rax);
  }
}

void JIT::andi(u32 instr) {
  s64 imm = (u16)instr;
  mov(rax, (u16)instr);
  mov(rcx, GPR(qword, RS(instr)));
  CodeGenerator::and_(rcx, rax);
  mov(GPR(qword, RT(instr)), rcx);
}

void JIT::and_(u32 instr) {
  if (RD(instr) != 0) [[likely]] {
    mov(rax, GPR(qword, RT(instr)));
    mov(rcx, GPR(qword, RS(instr)));
    CodeGenerator::and_(rax, rcx);
    mov(GPR(qword, RD(instr)), rax);
  }
}

void JIT::sll(u32 instr) {
  auto dst = Entry::Operand{ Entry::Operand::REG_S64, u8(RD(instr)) };
  auto op1 = Entry::Operand{ Entry::Operand::REG_S64, u8(RT(instr)) };
  auto op2 = Entry::Operand{ Entry::Operand::IMM_U5, std::nullopt };
  Entry e(Entry::SLL, dst, op1, op2);
  ir.push(e);
}

void JIT::sllv(u32 instr) {
  if (RD(instr) != 0) [[likely]] {
    mov(rcx, GPR(qword, RS(instr)));
    CodeGenerator::and_(cl, 0x1F);
    mov(rax, GPR(qword, RT(instr)));
    sal(rax, cl);
    movsxd(rcx, eax);
    mov(GPR(qword, RD(instr)), rcx);
  }
}

void JIT::dsll32(u32 instr) {
  if (RD(instr) != 0) [[likely]] {
    u8 sa = ((instr >> 6) & 0x1f) + 32;
    mov(rax, GPR(qword, RT(instr)));
    sal(rax, sa);
    mov(GPR(qword, RT(instr)), rax);
  }
}

void JIT::dsll(u32 instr) {
  if (RD(instr) != 0) [[likely]] {
    u8 sa = ((instr >> 6) & 0x1f);
    mov(rax, GPR(qword, RT(instr)));
    sal(rax, sa);
    mov(GPR(qword, RT(instr)), rax);
  }
}

void JIT::dsllv(u32 instr) {
  if (RD(instr) != 0) [[likely]] {
    mov(rcx, GPR(qword, RS(instr)));
    CodeGenerator::and_(cl, 63);
    mov(rax, GPR(qword, RT(instr)));
    sal(rax, cl);
    mov(GPR(qword, RD(instr)), rax);
  }
}

void JIT::srl(u32 instr) {
  if (RD(instr) != 0) [[likely]] {
    u8 sa = ((instr >> 6) & 0x1f);
    mov(rax, GPR(qword, RT(instr)));
    CodeGenerator::shr(rax, sa);
    movsxd(rcx, eax);
    mov(GPR(qword, RD(instr)), rcx);
  }
}

void JIT::srlv(u32 instr) {
  if (RD(instr) != 0) [[likely]] {
    mov(rcx, GPR(qword, RS(instr)));
    CodeGenerator::and_(cl, 0x1F);
    mov(rax, GPR(qword, RT(instr)));
    shr(rax, cl);
    movsxd(rcx, eax);
    mov(GPR(qword, RD(instr)), rcx);
  }
}

void JIT::tgei(u32) {

}

void JIT::tgeiu(u32) {

}

void JIT::tlti(u32) {

}

void JIT::tltiu(u32) {

}

void JIT::teqi(u32) {

}

void JIT::tnei(u32) {

}

void JIT::tge(u32) {

}

void JIT::tgeu(u32) {

}

void JIT::tlt(u32) {

}

void JIT::tltu(u32) {

}

void JIT::teq(u32) {

}

void JIT::tne(u32) {

}

void JIT::dsrl(u32 instr) {
  if (RD(instr) != 0) [[likely]] {
    u8 sa = ((instr >> 6) & 0x1f);
    mov(rax, GPR(qword, RT(instr)));
    CodeGenerator::shr(rax, sa);
    mov(GPR(qword, RD(instr)), rax);
  }
}

void JIT::dsrlv(u32 instr) {
  if (RD(instr) != 0) [[likely]] {
    mov(rcx, GPR(qword, RS(instr)));
    CodeGenerator::and_(cl, 63);
    mov(rax, GPR(qword, RT(instr)));
    shr(rax, cl);
    mov(GPR(qword, RD(instr)), rax);
  }
}

void JIT::dsrl32(u32 instr) {
  if (RD(instr) != 0) [[likely]] {
    u8 sa = ((instr >> 6) & 0x1f) + 32;
    mov(rax, GPR(qword, RT(instr)));
    CodeGenerator::shr(rax, sa);
    mov(GPR(qword, RD(instr)), rax);
  }
}

void JIT::sra(u32 instr) {
  if (RD(instr) != 0) [[likely]] {
    u8 sa = ((instr >> 6) & 0x1f);
    mov(rax, GPR(qword, RT(instr)));
    sar(rax, sa);
    movsxd(rcx, eax);
    mov(GPR(qword, RD(instr)), rcx);
  }
}

void JIT::srav(u32 instr) {
  if (RD(instr) != 0) [[likely]] {
    mov(rcx, GPR(qword, RS(instr)));
    CodeGenerator::and_(cl, 0x1F);
    mov(rax, GPR(qword, RT(instr)));
    sar(rax, cl);
    movsxd(rcx, eax);
    mov(GPR(qword, RD(instr)), rcx);
  }
}

void JIT::dsra(u32 instr) {
  if (RD(instr) != 0) [[likely]] {
    u8 sa = ((instr >> 6) & 0x1f);
    mov(rax, GPR(qword, RT(instr)));
    sar(rax, sa);
    mov(GPR(qword, RD(instr)), rax);
  }
}

void JIT::dsrav(u32 instr) {
  if (RD(instr) != 0) [[likely]] {
    mov(rcx, GPR(qword, RS(instr)));
    CodeGenerator::and_(cl, 63);
    mov(rax, GPR(qword, RT(instr)));
    sar(rax, cl);
    mov(GPR(qword, RD(instr)), rax);
  }
}

void JIT::dsra32(u32 instr) {
  if (RD(instr) != 0) [[likely]] {
    u8 sa = ((instr >> 6) & 0x1f) + 32;
    mov(rax, GPR(qword, RT(instr)));
    sar(rax, sa);
    mov(GPR(qword, RD(instr)), rax);
  }
}

void JIT::jr(u32 instr) {
  mov(rax, GPR(qword, RS(instr)));
  mov(REG(byte, delaySlot), 1);
  mov(REG(qword, nextPC), rax);
}

void JIT::dsub(u32 instr) {
  if (RD(instr) != 0) [[likely]] {
    mov(rax, GPR(qword, RT(instr)));
    mov(rcx, GPR(qword, RS(instr)));
    CodeGenerator::sub(rcx, rax);
    mov(GPR(qword, RD(instr)), rcx);
  }
}

void JIT::dsubu(u32 instr) {
  dsub(instr);
}

void JIT::sub(u32 instr) {
  if (RD(instr) != 0) [[likely]] {
    mov(eax, GPR(qword, RT(instr)));
    mov(ecx, GPR(qword, RS(instr)));
    CodeGenerator::sub(ecx, eax);
    movsxd(rax, ecx);
    mov(GPR(qword, RD(instr)), rax);
  }
}

void JIT::subu(u32 instr) {
  sub(instr);
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
  if (RD(instr) != 0) [[likely]] {
    regs.gpr[RD(instr)] = regs.lo;
  }
}

void JIT::mfhi(u32 instr) {
  if (RD(instr) != 0) [[likely]] {
    regs.gpr[RD(instr)] = regs.hi;
  }
}

void JIT::mtlo(u32 instr) {
  regs.lo = regs.gpr[RS(instr)];
}

void JIT::mthi(u32 instr) {
  regs.hi = regs.gpr[RS(instr)];
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