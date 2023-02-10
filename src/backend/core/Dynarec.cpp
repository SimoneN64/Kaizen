#include <Dynarec.hpp>
#include <Registers.hpp>
#include <filesystem>

namespace n64::JIT {
namespace fs = std::filesystem;

Dynarec::~Dynarec() {
  Util::aligned_free(codeCache);
  dump.close();
}

Dynarec::Dynarec() : code(CODECACHE_SIZE, codeCache) {
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

void Dynarec::Recompile(Mem& mem, u32 pc) {
  bool branch = false, prevBranch = false;
  u32 startPC = pc;
  u32 loopPC = pc;
  Fn block = code.getCurr<Fn>();

  if(code.getSize() >= CODECACHE_OVERHEAD) {
    Util::debug("Code cache overflow!\n");
    code.setSize(0);
    InvalidateCache();
  }

  Util::debug("Start of block @ PC {:08X}\n", loopPC);
  code.sub(rsp, 8);

  while(!prevBranch) {
    instrInBlock++;
    prevBranch = branch;
    u32 instr = mem.Read32<false>(regs, loopPC, loopPC);

    emitBreakpoint();
    code.mov(rdi, (uintptr_t)&regs);

    Util::debug("\tInstr: {:08X}, PC: {:08X}\n", instr, loopPC);
    code.mov(r8, qword[rdi + REG_OFFSET(oldPC)]);
    code.mov(r9, qword[rdi + REG_OFFSET(pc)]);
    code.mov(r10, qword[rdi + REG_OFFSET(nextPC)]);
    code.mov(r8, r9);
    code.mov(r9, r10);
    code.add(r10, 4);
    code.mov(qword[rdi + REG_OFFSET(oldPC)], r8);
    code.mov(qword[rdi + REG_OFFSET(pc)], r9);
    code.mov(qword[rdi + REG_OFFSET(nextPC)], r10);

    loopPC += 4;

    branch = Exec(mem, instr);
  }

  code.add(rsp, 8);
  code.ret();
  Util::debug("End of block @ PC {:08X}, len: {}\n", loopPC, instrInBlock);
  dump.write(code.getCode<char*>(), code.getSize());

  blockCache[startPC >> 20][startPC & 0xFFF] = block;
  blockCache[startPC >> 20][startPC & 0xFFF]();
}

void Dynarec::AllocateOuter(u32 pc) {
  blockCache[pc >> 20] = (Fn*)bumpAlloc(0x1000 * sizeof(Fn));
}

int Dynarec::Step(Mem &mem) {
  instrInBlock = 0;
  regs.gpr[0] = 0;
  regs.prevDelaySlot = regs.delaySlot;
  regs.delaySlot = false;

  u32 pc{};
  if(!MapVAddr(regs, LOAD, regs.pc, pc)) {
    Util::panic("[RECOMPILER] Failed to translate PC to physical address (v: {:08X})\n", regs.pc);
  }

  if(blockCache[pc >> 20]) {
    if(blockCache[pc >> 20][pc & 0xfff]) {
      blockCache[pc >> 20][pc & 0xfff]();
    } else {
      Recompile(mem, pc);
    }
  } else {
    AllocateOuter(pc);
    Recompile(mem, pc);
  }

  CheckCompareInterrupt(mem.mmio.mi, regs);

  if(ShouldServiceInterrupt(regs)) {
    FireException(regs, ExceptionCode::Interrupt, 0, false);
  }

  return instrInBlock;
}
}