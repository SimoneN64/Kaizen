#include <Cop0.hpp>
#include <JIT.hpp>

namespace n64 {
using namespace Xbyak;
JIT::JIT() : CodeGenerator(32*1024*1024, AutoGrow) { }

void JIT::Reset() {
  reset();
  regs.Reset();
  mem.Reset();
}

bool JIT::ShouldServiceInterrupt() {
  bool interrupts_pending = (regs.cop0.status.im & regs.cop0.cause.interruptPending) != 0;
  bool interrupts_enabled = regs.cop0.status.ie == 1;
  bool currently_handling_exception = regs.cop0.status.exl == 1;
  bool currently_handling_error = regs.cop0.status.erl == 1;

  return interrupts_pending && interrupts_enabled &&
         !currently_handling_exception && !currently_handling_error;
}

void JIT::CheckCompareInterrupt() {
  regs.cop0.count++;
  regs.cop0.count &= 0x1FFFFFFFF;
  if(regs.cop0.count == (u64)regs.cop0.compare << 1) {
    regs.cop0.cause.ip7 = 1;
    UpdateInterrupt(mem.mmio.mi, regs);
  }
}

Fn JIT::Recompile() {
  bool stable = true;
  cycles = 0;
  prologue();
  while(stable) {
    mov(rdi, u64(this) + offsetof(JIT, regs));
    cycles++;
    CheckCompareInterrupt();

    mov(rax, byte[rdi + offsetof(Registers, delaySlot)]);
    mov(byte[rdi + offsetof(Registers, prevDelaySlot)], rax);
    mov(byte[rdi + offsetof(Registers, delaySlot)], 0);

    u32 paddr = 0;
    if (!MapVAddr(regs, LOAD, regs.pc, paddr)) {
      mov(rsi, regs.pc);
      push(rax);
      call(HandleTLBException);
      mov(rsi, u64(GetTLBExceptionCode(regs.cop0.tlbError, LOAD)));
      CodeGenerator::xor_(rdx, rdx);
      CodeGenerator::xor_(rcx, rcx);
      push(rax);
      call(FireException);
      goto _epilogue;
    }

    u32 instr = mem.Read32(regs, paddr);
    stable = isStable(instr);
    Emit(instr);

    if (ShouldServiceInterrupt()) {
      mov(rsi, u64(ExceptionCode::Interrupt));
      CodeGenerator::xor_(rdx, rdx);
      CodeGenerator::xor_(rcx, rcx);
      push(rax);
      call(FireException);
      goto _epilogue;
    }

    mov(rax, qword[rdi + offsetof(Registers, pc)]);
    mov(qword[rdi + offsetof(Registers, oldPC)], rax);
    mov(rax, qword[rdi + offsetof(Registers, nextPC)]);
    mov(qword[rdi + offsetof(Registers, pc)], rax);
    CodeGenerator::add(qword[rdi + offsetof(Registers, nextPC)], 4);
  }
_epilogue:
  epilogue();
  ready();
  return getCode<Fn>();
}

int JIT::Step() {
  if(!blocks[regs.pc >> 20]) {
    blocks[regs.pc >> 20] = (Fn*)calloc(4096, sizeof(Fn));
    blocks[regs.pc >> 20][regs.pc & 0xfff] = Recompile();
  }

  if (!blocks[regs.pc >> 20][regs.pc & 0xfff]) {
    blocks[regs.pc >> 20][regs.pc & 0xfff] = Recompile();
  }

  return blocks[regs.pc >> 20][regs.pc & 0xfff]();
}
}
