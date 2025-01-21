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

void JIT::InvalidateBlock(const u32 paddr) {
  if (const u32 index = paddr >> kUpperShift; !blockCache[index].empty())
    blockCache[index].erase(blockCache[index].begin(), blockCache[index].end());
}

int JIT::Step() {
  blockPC = regs.pc;
  u32 paddr = 0;

  if (!regs.cop0.MapVAddr(Cop0::LOAD, blockPC, paddr)) {
    /*regs.cop0.HandleTLBException(blockPC);
    regs.cop0.FireException(Cop0::GetTLBExceptionCode(regs.cop0.tlbError, Cop0::LOAD), 0, blockPC);
    return 1;*/
    Util::panic(
      "[JIT]: Unhandled exception TLB exception {} when retrieving PC physical address! (virtual: 0x{:016lX})",
      static_cast<int>(Cop0::GetTLBExceptionCode(regs.cop0.tlbError, Cop0::LOAD)), static_cast<u64>(blockPC));
  }

  u32 upperIndex = paddr >> kUpperShift;
  u32 lowerIndex = paddr & 0xff;

  if (!blockCache[upperIndex].empty()) {
    if (blockCache[upperIndex][lowerIndex]) {
      return blockCache[upperIndex][lowerIndex]();
    }
  } else {
    blockCache[upperIndex].resize(kLowerSize);
  }

  const auto block = code.getCurr<BlockFn>();
  blockCache[upperIndex][lowerIndex] = block;

  code.setProtectModeRW();

  u32 instructionsInBlock = 0;

  bool instrEndsBlock = false;
  bool instrInDelaySlot = false;
  bool branchWasLikely = false;
  bool blockEndsOnBranch = false;

  // code.int3();
  code.sub(code.rsp, 8);
  code.push(code.rbp);
  code.mov(code.rbp, reinterpret_cast<uintptr_t>(this)); // Load context pointer

  while (!instrInDelaySlot) {
    // CheckCompareInterrupt();

    if (check_address_error(0b11, u64(blockPC))) [[unlikely]] {
      /*regs.cop0.HandleTLBException(blockPC);
      regs.cop0.FireException(ExceptionCode::AddressErrorLoad, 0, blockPC);
      return 1;*/

      Util::panic("[JIT]: Unhandled exception ADL due to unaligned PC virtual value! (0x{:016lX})", blockPC);
    }

    if (!regs.cop0.MapVAddr(Cop0::LOAD, blockPC, paddr)) {
      /*regs.cop0.HandleTLBException(blockPC);
      regs.cop0.FireException(Cop0::GetTLBExceptionCode(regs.cop0.tlbError, Cop0::LOAD), 0, blockPC);
      return 1;*/
      Util::panic(
        "[JIT]: Unhandled exception TLB exception {} when retrieving PC physical address! (virtual: 0x{:016lX})",
        static_cast<int>(Cop0::GetTLBExceptionCode(regs.cop0.tlbError, Cop0::LOAD)), static_cast<u64>(blockPC));
    }

    const u32 instruction = mem.Read<u32>(regs, paddr);

    /*if(ShouldServiceInterrupt()) {
      regs.cop0.FireException(ExceptionCode::Interrupt, 0, blockPC);
      return 1;
    }*/

    blockPC += 4;
    instructionsInBlock++;
    Emit(instruction);

    instrInDelaySlot = instrEndsBlock;
    instrEndsBlock = InstrEndsBlock(instruction);
    if (instrEndsBlock) {
      branchWasLikely = IsBranchLikely(instruction);
    }

    if (instrInDelaySlot) {
      blockEndsOnBranch = true;
    }

    if (instrInDelaySlot && branchWasLikely) {
      branchWasLikely = false;
      code.L(branch_likely_not_taken);
      code.mov(code.rax, blockPC);
      code.mov(REG(qword, pc), code.rax);
    }
  }

  // emit code to store the value of pc
  if (!blockEndsOnBranch) {
    code.mov(code.rax, blockPC);
    code.mov(REG(qword, pc), code.rax);
  }
  code.mov(code.rax, instructionsInBlock);
  code.pop(code.rbp);
  code.add(code.rsp, 8);
  code.ret();
  code.setProtectModeRE();
  const auto dump = code.getCode();
  Util::WriteFileBinary(dump, code.getSize(), "jit.dump");
  // Util::panic("");
  return block();
}

std::vector<u8> JIT::Serialize() {
  std::vector<u8> res{};

  res.resize(sizeof(Registers));

  memcpy(res.data(), &regs, sizeof(Registers));

  return res;
}

void JIT::Deserialize(const std::vector<u8> &data) { memcpy(&regs, data.data(), sizeof(Registers)); }
} // namespace n64
