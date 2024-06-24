#include <Core.hpp>
#include "Scheduler.hpp"

namespace n64 {
Interpreter::Interpreter(ParallelRDP& parallel) : mem(regs, parallel) { }

bool Interpreter::ShouldServiceInterrupt() {
  bool interrupts_pending = (regs.cop0.status.im & regs.cop0.cause.interruptPending) != 0;
  bool interrupts_enabled = regs.cop0.status.ie == 1;
  bool currently_handling_exception = regs.cop0.status.exl == 1;
  bool currently_handling_error = regs.cop0.status.erl == 1;

  return interrupts_pending && interrupts_enabled &&
         !currently_handling_exception && !currently_handling_error;
}

void Interpreter::CheckCompareInterrupt() {
  regs.cop0.count++;
  regs.cop0.count &= 0x1FFFFFFFF;
  if(regs.cop0.count == (u64)regs.cop0.compare << 1) {
    regs.cop0.cause.ip7 = 1;
    mem.mmio.mi.UpdateInterrupt();
  }
}

int Interpreter::Step() {
  CheckCompareInterrupt();

  regs.prevDelaySlot = regs.delaySlot;
  regs.delaySlot = false;

  if(check_address_error(0b11, u64(regs.pc))) [[unlikely]] {
    regs.cop0.HandleTLBException(regs.pc);
    regs.cop0.FireException(ExceptionCode::AddressErrorLoad, 0, regs.pc);
    return 1;
  }

  u32 paddr = 0;
  if(!regs.cop0.MapVAddr(Cop0::LOAD, regs.pc, paddr)) {
    regs.cop0.HandleTLBException(regs.pc);
    regs.cop0.FireException(regs.cop0.GetTLBExceptionCode(regs.cop0.tlbError, Cop0::LOAD), 0, regs.pc);
    return 1;
  }

  u32 instruction = mem.Read<u32>(regs, paddr);

  if(ShouldServiceInterrupt()) {
    regs.cop0.FireException(ExceptionCode::Interrupt, 0, regs.pc);
    return 1;
  }

  regs.oldPC = regs.pc;
  regs.pc = regs.nextPC;
  regs.nextPC += 4;

  Exec(instruction);

  return 1;
}

int Interpreter::RunCached() {
  const InstructionCache& cache = **currentCache;
  for(int i = 0; i < cache.instructions.size(); i++) {
    const auto& instr = cache.instructions[i];
    if(scheduler.ShouldRun()) [[unlikely]] {
      scheduler.Tick(1, mem);
      return i;
    }

    regs.oldPC = regs.pc;
    regs.pc = regs.nextPC;
    regs.nextPC += 4;

    (this->*instr.funcPointer)(instr.instruction);
    if(!currentCache) [[unlikely]] return i;
  }

  return cache.instructions.size();
}

std::vector<u8> Interpreter::Serialize() {
  std::vector<u8> res{};

  res.resize(sizeof(Registers));

  memcpy(res.data(), &regs, sizeof(Registers));

  return res;
}

void Interpreter::Deserialize(const std::vector<u8> &data) {
  memcpy(&regs, data.data(), sizeof(Registers));
}

constexpr std::unique_ptr<InstructionCache> *Interpreter::GetCache(const u32 address) {
  switch(address) {
    case RDRAM_REGION: {
      const u32 index = (address & RDRAM_DSIZE) >> 1;
      if ((address & RDRAM_DSIZE) < RDRAM_SIZE) {
        if (rdramCache[index]) {
          return &rdramCache[index];
        }
        // mark index as filled for faster page clearing
        // rdramCacheFilled[(address & RDRAM_DSIZE) / CACHE_BLOCK_SIZE].push_back(index);
        return &rdramCache[index];
      }
      return nullptr;
    }
    default: return nullptr;
  }
}
}