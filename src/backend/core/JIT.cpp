#include <Registers.hpp>
#include <JIT.hpp>

namespace n64 {
JIT::~JIT() {
  Util::aligned_free(codeCache);
}

JIT::JIT() {
  codeCache = (u8*)Util::aligned_alloc(4096, CODECACHE_SIZE);
  code = std::make_unique<CodeGenerator>(CODECACHE_SIZE, codeCache);
  CodeArray::protect(
    codeCache,
    CODECACHE_SIZE,
    CodeArray::PROTECT_RWE
  );
  regs.Reset();
}

inline bool ShouldServiceInterrupt(Registers& regs) {
  bool interrupts_pending = (regs.cop0.status.im & regs.cop0.cause.interruptPending) != 0;
  bool interrupts_enabled = regs.cop0.status.ie == 1;
  bool currently_handling_exception = regs.cop0.status.exl == 1;
  bool currently_handling_error = regs.cop0.status.erl == 1;

  return interrupts_pending && interrupts_enabled &&
         !currently_handling_exception && !currently_handling_error;
}

inline void CheckCompareInterrupt(MI& mi, Registers& regs) {
  regs.cop0.count++;
  regs.cop0.count &= 0x1FFFFFFFF;
  if(regs.cop0.count == (u64)regs.cop0.compare << 1) {
    regs.cop0.cause.ip7 = 1;
    UpdateInterrupt(mi, regs);
  }
}

void JIT::Recompile(Mem& mem, u32 pc) {
  bool branch = false, prevBranch = false;
  u32 startPC = pc;
  u32 loopPC = pc;
  Fn block = code->getCurr<Fn>();

  if(code->getSize() >= CODECACHE_OVERHEAD) {
    Util::panic("Code cache overflow!");
    code->setSize(0);
    InvalidateCache();
  }

  code->push(rbp);
  code->mov(rbp, rsp);
#ifdef ABI_MSVC
  code->sub(rsp, 8);
#else
  code->sub(rsp, 8);
#endif

  while (!prevBranch) {
    code->mov(regArg0, (uintptr_t)this);
    code->mov(regScratch0, qword[regArg0 + THIS_OFFSET(instrInBlock, this)]);
    code->inc(regScratch0);
    code->mov(qword[regArg0 + THIS_OFFSET(instrInBlock, this)], regScratch0);

    prevBranch = branch;
    u32 instr = mem.Read32(regs, loopPC);
    loopPC += 4;
    branch = isEndBlock(instr);
    Util::trace("{:08X}", instr);

    code->mov(qword[regArg0 + GPR_OFFSET(0, this)], 0);
    code->mov(regScratch0, qword[regArg0 + REG_OFFSET(oldPC, this)]);
    code->mov(regScratch1, qword[regArg0 + REG_OFFSET(pc, this)]);
    code->mov(regScratch2, qword[regArg0 + REG_OFFSET(nextPC, this)]);
    code->mov(regScratch0, regScratch1);
    code->mov(regScratch1, regScratch2);
    code->add(regScratch2, 4);
    code->mov(qword[regArg0 + REG_OFFSET(oldPC, this)], regScratch0);
    code->mov(qword[regArg0 + REG_OFFSET(pc, this)], regScratch1);
    code->mov(qword[regArg0 + REG_OFFSET(nextPC, this)], regScratch2);

    code->mov(regArg1, instr);
    Emit(instr);
  }

  code->mov(rsp, rbp);
  code->pop(rbp);
  code->ret();

  blockCache[startPC >> 20][startPC & 0xFFF] = block;
}

void JIT::AllocateOuter(u32 pc) {
  blockCache[pc >> 20] = (Fn*)bumpAlloc(0x1000 * sizeof(Fn));
}

int JIT::Run() {
  instrInBlock = 0;
  regs.prevDelaySlot = regs.delaySlot;
  regs.delaySlot = false;

  u32 pc{};

  if(!MapVAddr(regs, LOAD, regs.pc, pc)) {
    HandleTLBException(regs, regs.pc);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, LOAD), 0, false);
    return 0;
  }

  if(!blockCache[pc >> 20]) {
    AllocateOuter(pc);
  }

  if(!blockCache[pc >> 20][pc & 0xfff]) {
    Recompile(mem, pc);
  }

  CheckCompareInterrupt(mem.mmio.mi, regs);
  if(ShouldServiceInterrupt(regs)) {
    FireException(regs, ExceptionCode::Interrupt, 0, false);
    return 0;
  }

  blockCache[pc >> 20][pc & 0xfff]();
  return instrInBlock;
}

void JIT::Reset() {
  code->reset();
  regs.Reset();
  InvalidateCache();
}
}