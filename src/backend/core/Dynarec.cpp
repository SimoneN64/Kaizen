#include <Dynarec.hpp>
#include <Registers.hpp>

namespace n64::JIT {
Dynarec::Dynarec() : code(Xbyak::DEFAULT_MAX_CODE_SIZE, Xbyak::AutoGrow) {}

void Dynarec::Recompile(Registers& regs, Mem& mem) {
  bool branch = false, prevBranch = false;
  u16 start_addr = regs.pc;
  Fn block = code.getCurr<Fn>();

  while(!prevBranch) {
    instrInBlock++;
    prevBranch = branch;
    u32 instr = mem.Read32(regs, regs.pc, regs.pc);

    regs.oldPC = regs.pc;
    regs.pc = regs.nextPC;
    regs.nextPC += 4;

    branch = Exec(regs, mem, instr);
  }

  code.ret();
  codeCache[start_addr >> 12][start_addr & 15] = block;
  block();
}

void Dynarec::AllocateOuter(Registers& regs) {
  codeCache[regs.pc >> 20] = (Fn*)calloc(0xFFF, sizeof(Fn));
}

int Dynarec::Step(Mem &mem, Registers& regs) {
  instrInBlock = 0;
  regs.gpr[0] = 0;
  regs.prevDelaySlot = regs.delaySlot;
  regs.delaySlot = false;

  if(codeCache[regs.pc >> 20]) {
    if(codeCache[regs.pc >> 20][regs.pc & 0xfff]) {
      codeCache[regs.pc >> 20][regs.pc & 0xfff]();
    } else {
      Recompile(regs, mem);
    }
  } else {
    AllocateOuter(regs);
    Recompile(regs, mem);
  }

  return instrInBlock;
}
}