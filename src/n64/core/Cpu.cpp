#include <n64/core/Cpu.hpp>
#include <n64/core/mmio/MI.hpp>
#include <n64/core/mmio/Interrupt.hpp>
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

inline bool Is64BitAddressing(Cop0& cp0, u64 addr) {
  u8 region = (addr >> 62) & 3;
  switch(region) {
    case 0b00: return cp0.status.ux;
    case 0b01: return cp0.status.sx;
    case 0b11: return cp0.status.kx;
    default: return false;
  }
}

void FireException(Registers& regs, ExceptionCode code, int cop, s64 pc) {
  bool old_exl = regs.cop0.status.exl;

  if(!regs.cop0.status.exl) {
    if(regs.prevDelaySlot) {
      regs.cop0.cause.branchDelay = true;
      pc -= 4;
    } else {
      regs.cop0.cause.branchDelay = false;
    }

    regs.cop0.status.exl = true;
    regs.cop0.EPC = pc;
  }

  regs.cop0.cause.copError = cop;
  regs.cop0.cause.exceptionCode = code;

  if(regs.cop0.status.bev) {
    util::panic("BEV bit set!\n");
  } else {
    switch(code) {
      case Interrupt: case TLBModification:
      case AddressErrorLoad: case AddressErrorStore:
      case InstructionBusError: case DataBusError:
      case Syscall: case Breakpoint:
      case ReservedInstruction: case CoprocessorUnusable:
      case Overflow: case Trap:
      case FloatingPointError: case Watch:
        regs.SetPC(s64(s32(0x80000180)));
        break;
      case TLBLoad: case TLBStore:
        if(old_exl || regs.cop0.tlbError == INVALID) {
          regs.SetPC(s64(s32(0x80000180)));
        } else if(Is64BitAddressing(regs.cop0, regs.cop0.badVaddr)) {
          regs.SetPC(s64(s32(0x80000080)));
        } else {
          regs.SetPC(s64(s32(0x80000000)));
        }
        break;
      default: util::panic("Unhandled exception! {}\n", code);
    }
  }
}

inline void HandleInterrupt(Registers& regs) {
  if(ShouldServiceInterrupt(regs)) {
    FireException(regs, ExceptionCode::Interrupt, 0, regs.pc);
  }
}

inline void Cpu::disassembly(u32 instr) const {
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
  HandleInterrupt(regs);

  u32 instruction = mem.Read32(regs, regs.pc, regs.pc);

  //disassembly(instruction);

  regs.oldPC = regs.pc;
  regs.pc = regs.nextPC;
  regs.nextPC += 4;

  Exec(mem, instruction);

  regs.prevDelaySlot = regs.delaySlot;
  regs.delaySlot = false;
}
}