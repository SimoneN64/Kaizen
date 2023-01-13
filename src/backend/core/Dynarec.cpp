#include <Dynarec.hpp>
#include <Registers.hpp>
#include <filesystem>

namespace n64::JIT {
namespace fs = std::filesystem;

Dynarec::~Dynarec() {
  std::free(codeCache);
}

Dynarec::Dynarec() : code(CODECACHE_SIZE, codeCache) {
  codeCache = (u8*)Util::aligned_alloc(4096, CODECACHE_SIZE);
  CodeArray::protect(
    codeCache,
    CODECACHE_SIZE,
    CodeArray::PROTECT_RWE
  );
}

void Dynarec::Recompile(Registers& regs, Mem& mem, u32 pc) {
  bool branch = false, prevBranch = false;
  u32 startPC = pc;
  u32 loopPC = pc;
  Fn block = code.getCurr<Fn>();

  if(code.getSize() >= CODECACHE_SIZE) {
    Util::print("Code cache overflow!\n");
    code.setSize(code.getSize() & (CODECACHE_SIZE - 1));
    InvalidateCache();
  }

  Util::print("Start of block @ PC {:08X}\n", loopPC);
  code.sub(rsp, 8);

  while(!prevBranch) {
    instrInBlock++;
    prevBranch = branch;
    u32 instr = mem.Read32<false>(regs, loopPC, loopPC);

    code.mov(rdi, (u64)&regs);

    Util::print("\tInstr: {:08X}, PC: {:08X}\n", instr, loopPC);
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

    branch = Exec(regs, mem, instr);
  }

  code.add(rsp, 8);
  code.ret();
  Util::print("End of block @ PC {:08X}, len: {}\n", loopPC, instrInBlock);

  blockCache[startPC >> 20][startPC & 0xFFF] = block;
  blockCache[startPC >> 20][startPC & 0xFFF]();
}

void Dynarec::AllocateOuter(u32 pc) {
  blockCache[pc >> 20] = (Fn*)bumpAlloc(0x1000 * sizeof(Fn));
}

int Dynarec::Step(Mem &mem, Registers& regs) {
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
      Recompile(regs, mem, pc);
    }
  } else {
    AllocateOuter(pc);
    Recompile(regs, mem, pc);
  }

  return instrInBlock;
}
}