#include <Registers.hpp>
#include <filesystem>
#include <JIT.hpp>

namespace n64 {
namespace fs = std::filesystem;

JIT::~JIT() {
  Util::aligned_free(codeCache);
  dump.close();
}

JIT::JIT() : code(CODECACHE_SIZE, codeCache) {
  codeCache = (u8*)Util::aligned_alloc(4096, CODECACHE_SIZE);
  CodeArray::protect(
    codeCache,
    CODECACHE_SIZE,
    CodeArray::PROTECT_RWE
  );

  if(fs::exists("jit.dump")) {
    fs::remove("jit.dump");
  }

  dump.open("jit.dump", std::ios::ate | std::ios::binary);
  dump.unsetf(std::ios::skipws);
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
  Fn block = code.getCurr<Fn>();

  if(code.getSize() >= CODECACHE_OVERHEAD) {
    Util::debug("Code cache overflow!");
    code.setSize(0);
    InvalidateCache();
  }

  code.push(rbx);
  code.push(rbp);
  code.push(r12);
  code.push(r13);
  code.push(r14);
  code.push(r15);
#ifdef ABI_MSVC
  code.push(rsi);
  code.push(rdi);
#endif
  code.sub(rsp, 8);
  code.mov(rbp, rsp);

  while(!prevBranch) {
    instrInBlock++;
    prevBranch = branch;
    u32 instr = mem.Read32(regs, loopPC);
    Util::trace("{:08X}", instr);
    loopPC += 4;

    code.mov(regArg0, (uintptr_t)this);
    code.mov(qword[rdi + GPR_OFFSET(0, this)], 0);
    code.mov(r8, qword[rdi + REG_OFFSET(oldPC, this)]);
    code.mov(r9, qword[rdi + REG_OFFSET(pc, this)]);
    code.mov(r10, qword[rdi + REG_OFFSET(nextPC, this)]);
    code.mov(r8, r9);
    code.mov(r9, r10);
    code.add(r10, 4);
    code.mov(qword[rdi + REG_OFFSET(oldPC, this)], r8);
    code.mov(qword[rdi + REG_OFFSET(pc, this)], r9);
    code.mov(qword[rdi + REG_OFFSET(nextPC, this)], r10);

    code.mov(regArg1, instr);
    branch = Exec(mem, instr);
  }

  code.add(rsp, 8);
#ifdef ABI_MSVC
  code.pop(rdi);
  code.pop(rsi);
#endif
  code.pop(r15);
  code.pop(r14);
  code.pop(r13);
  code.pop(r12);
  code.pop(rbp);
  code.pop(rbx);
  code.ret();
  dump.write(code.getCode<char*>(), code.getSize());

  blockCache[startPC >> 20][startPC & 0xFFF] = block;
}

void JIT::AllocateOuter(u32 pc) {
  blockCache[pc >> 20] = (Fn*)bumpAlloc(0x1000 * sizeof(Fn));
}

int JIT::Run() {
  /*instrInBlock = 0;
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
  */
  Util::panic("JIT is unimplemented!");
}

void JIT::Reset() {
  code.reset();
  regs.Reset();
  InvalidateCache();
}
}