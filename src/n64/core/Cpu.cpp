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
    if(regs.delaySlot) { // TODO: cached value of delay_slot should be used, but Namco Museum breaks!
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
      case Interrupt ... TLBModification:
      case AddressErrorLoad ... Trap:
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

void Cpu::Step(Mem& mem) {
  regs.gpr[0] = 0;

  CheckCompareInterrupt(mem.mmio.mi, regs);
  HandleInterrupt(regs);

  regs.prevDelaySlot = regs.delaySlot;
  regs.delaySlot = false;

  u32 instruction = mem.Read<u32>(regs, regs.pc, regs.pc);

  /*cs_insn* insn;
  u8 code[4]{};
  memcpy(code, &instruction, 4);

  u32 pc = regs.pc;

  size_t count = cs_disasm(handle, code, 4, (u64)pc, 0, &insn);
  if(count > 0) {
    for(int i = 0; i < count; i++) {
      fprintf(log, "%s", fmt::format("0x{:016X}\t{}\t{}\n", insn[i].address, insn[i].mnemonic, insn[i].op_str).c_str());
    }

    cs_free(insn, count);
  }*/

  regs.oldPC = regs.pc;
  regs.pc = regs.nextPC;
  regs.nextPC += 4;

  Exec(mem, instruction);
}
}