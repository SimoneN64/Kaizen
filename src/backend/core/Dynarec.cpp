#include <Dynarec.hpp>
#include <Registers.hpp>
#include <filesystem>

namespace n64::JIT {
namespace fs = std::filesystem;

Dynarec::~Dynarec() {
  dumpCode.close();
}

Dynarec::Dynarec() : code(DEFAULT_MAX_CODE_SIZE, AutoGrow) {
  if(fs::exists("jit.dump")) {
    fs::remove("jit.dump");
  }

  dumpCode.open("jit.dump", std::ios::app | std::ios::binary);
  code.ready();
}

void Dynarec::Recompile(Registers& regs, Mem& mem) {
  bool branch = false, prevBranch = false;
  u32 start_addr = regs.pc;
  Fn block = code.getCurr<Fn>();

  code.sub(rsp, 8);

  while(!prevBranch) {
    instrInBlock++;
    prevBranch = branch;
    u32 instr = mem.Read32(regs, start_addr, start_addr);

    start_addr += 4;

    code.mov(rdi, (u64)&regs);
    branch = Exec(regs, mem, instr);
  }

  code.add(rsp, 8);
  code.ret();
  dumpCode.write(code.getCode<char*>(), code.getSize());
  u32 pc = regs.pc & 0xffffffff;
  codeCache[pc >> 20][pc & 0xFFF] = block;
  block();
}

void Dynarec::AllocateOuter(Registers& regs) {
  u32 pc = regs.pc & 0xffffffff;
  codeCache[pc >> 20] = (Fn*)calloc(0xFFF, sizeof(Fn));
}

int Dynarec::Step(Mem &mem, Registers& regs) {
  instrInBlock = 0;
  regs.gpr[0] = 0;
  regs.prevDelaySlot = regs.delaySlot;
  regs.delaySlot = false;

  u32 pc = regs.pc & 0xffffffff;

  if(codeCache[pc >> 20]) {
    if(codeCache[pc >> 20][pc & 0xfff]) {
      codeCache[pc >> 20][pc & 0xfff]();
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