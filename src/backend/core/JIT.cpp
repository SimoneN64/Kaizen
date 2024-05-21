#include <Core.hpp>
#include <jit/helpers.hpp>

namespace n64 {
JIT::JIT(ParallelRDP& parallel) : mem(regs, parallel) {
  regs.gpr[0] = 0;
  regs.gprIsConstant[0] = true;
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
    mem.mmio.mi.UpdateInterrupt();
  }
}

int JIT::Step() {
  u32 instruction;
  s64 pc = regs.pc;

  do {
    //CheckCompareInterrupt();

    //regs.prevDelaySlot = regs.delaySlot;
    //regs.delaySlot = false;

    /*if (check_address_error(0b11, u64(pc))) [[unlikely]] {
      regs.cop0.HandleTLBException(pc);
      regs.cop0.FireException(ExceptionCode::AddressErrorLoad, 0, pc);
      return 1;
    }*/

    u32 paddr = 0;
    if (!regs.cop0.MapVAddr(Cop0::LOAD, pc, paddr)) {
      /*regs.cop0.HandleTLBException(pc);
      regs.cop0.FireException(regs.cop0.GetTLBExceptionCode(regs.cop0.tlbError, Cop0::LOAD), 0, pc);
      return 1;*/
      Util::panic("[JIT]: Unhandled exception TLB exception {} when retrieving PC physical address!", regs.cop0.GetTLBExceptionCode(regs.cop0.tlbError, Cop0::LOAD));
    }

    instruction = mem.Read<u32>(regs, paddr);

    /*if(ShouldServiceInterrupt()) {
      regs.cop0.FireException(ExceptionCode::Interrupt, 0, regs.pc);
      return 1;
    }*/

    pc += 4;

    //Exec(instruction);
  } while (!InstrEndsBlock(instruction));

  return 1;
}

std::vector<u8> JIT::Serialize() {
  std::vector<u8> res{};

  res.resize(sizeof(Registers));

  memcpy(res.data(), &regs, sizeof(Registers));

  return res;
}

void JIT::Deserialize(const std::vector<u8> &data) {
  memcpy(&regs, data.data(), sizeof(Registers));
}
}