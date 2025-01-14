#include <Core.hpp>
#include <jit/helpers.hpp>

namespace n64 {
JIT::JIT(ParallelRDP &parallel) : regs(this), mem(regs, parallel, this) { blockCache.resize(kUpperSize); }

bool JIT::ShouldServiceInterrupt() const {
  const bool interrupts_pending = (regs.cop0.status.im & regs.cop0.cause.interruptPending) != 0;
  const bool interrupts_enabled = regs.cop0.status.ie == 1;
  const bool currently_handling_exception = regs.cop0.status.exl == 1;
  const bool currently_handling_error = regs.cop0.status.erl == 1;

  return interrupts_pending && interrupts_enabled && !currently_handling_exception && !currently_handling_error;
}

void JIT::CheckCompareInterrupt() {
  regs.cop0.count++;
  regs.cop0.count &= 0x1FFFFFFFF;
  if (regs.cop0.count == static_cast<u64>(regs.cop0.compare) << 1) {
    regs.cop0.cause.ip7 = 1;
    mem.mmio.mi.UpdateInterrupt();
  }
}

int JIT::Step() {
  blockPC = regs.pc;

  if (!blockCache[blockPC >> 8].empty()) {
    if (blockCache[blockPC >> 8][blockPC >> 20]) {
      return blockCache[blockPC >> 8][blockPC >> 20]();
    }
  } else {
    blockCache[blockPC >> 8].resize(kLowerSize);
  }

  regs.block_delaySlot = false;

  u32 instruction;

  do {
    // CheckCompareInterrupt();

    if (check_address_error(0b11, u64(blockPC))) [[unlikely]] {
      /*regs.cop0.HandleTLBException(blockPC);
      regs.cop0.FireException(ExceptionCode::AddressErrorLoad, 0, blockPC);
      return 1;*/

      Util::panic("[JIT]: Unhandled exception ADL due to unaligned PC virtual value! (0x{:016lX})",
                  static_cast<u64>(regs.pc));
    }

    u32 paddr = 0;

    if (!regs.cop0.MapVAddr(Cop0::LOAD, blockPC, paddr)) {
      /*regs.cop0.HandleTLBException(blockPC);
      regs.cop0.FireException(Cop0::GetTLBExceptionCode(regs.cop0.tlbError, Cop0::LOAD), 0, blockPC);
      return 1;*/
      Util::panic(
        "[JIT]: Unhandled exception TLB exception {} when retrieving PC physical address! (virtual: 0x{:016lX})",
        static_cast<int>(Cop0::GetTLBExceptionCode(regs.cop0.tlbError, Cop0::LOAD)), static_cast<u64>(regs.pc));
    }

    instruction = mem.Read<u32>(regs, paddr);

    /*if(ShouldServiceInterrupt()) {
      regs.cop0.FireException(ExceptionCode::Interrupt, 0, regs.pc);
      return 1;
    }*/

    blockPC += 4;

    Emit(instruction);
  }
  while (!InstrEndsBlock(instruction));

  // emit code to store the value of pc
  blockCache[regs.pc >> 8][regs.pc >> 20] = code.getCurr<BlockFn>();
  return blockCache[regs.pc >> 8][regs.pc >> 20]();
}

std::vector<u8> JIT::Serialize() {
  std::vector<u8> res{};

  res.resize(sizeof(Registers));

  memcpy(res.data(), &regs, sizeof(Registers));

  return res;
}

void JIT::Deserialize(const std::vector<u8> &data) { memcpy(&regs, data.data(), sizeof(Registers)); }
} // namespace n64
