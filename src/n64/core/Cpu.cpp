#include <n64/core/Cpu.hpp>
#include <util.hpp>

namespace n64 {
void Cpu::Reset() {
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

inline void Cpu::disassembly(u32 instr) {
  size_t count;
  cs_insn *insn;

  u8 code[4];
  memcpy(code, &instr, 4);

  count = cs_disasm(handle, code, 4, regs.pc, 0, &insn);

  if (count > 0) {
    size_t j;
    for (j = 0; j < count; j++) {
      fmt::print("0x{:016X}:\t{}\t\t{}\n", insn[j].address, insn[j].mnemonic, insn[j].op_str);
    }

    cs_free(insn, count);
  } else
    printf("ERROR: Failed to disassemble given code!\n");
}

void Cpu::Step(Mem& mem) {
  regs.gpr[0] = 0;

  CheckCompareInterrupt(mem.mmio.mi, regs);

  regs.prevDelaySlot = regs.delaySlot;
  regs.delaySlot = false;

  u32 instruction = mem.Read32(regs, regs.pc, regs.pc);

  if(ShouldServiceInterrupt(regs)) {
    FireException(regs, ExceptionCode::Interrupt, 0, regs.pc);
    return;
  }

  regs.oldPC = regs.pc;
  regs.pc = regs.nextPC;
  regs.nextPC += 4;

  Exec(mem, instruction);
}
}