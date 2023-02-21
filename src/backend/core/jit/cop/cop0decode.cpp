#include <jit/cop/cop0decode.hpp>
#include <jit/cop/cop0instructions.hpp>
#include <Registers.hpp>

namespace n64 {
void cop0Decode(Registers& regs, JIT& cpu, u32 instr) {
  u8 mask_cop = (instr >> 21) & 0x1F;
  u8 mask_cop2 = instr & 0x3F;
  Xbyak::CodeGenerator& code = cpu.code;

  switch(mask_cop) {
    case 0x00:
      code.mov(code.rsi, (u64)instr);
      code.mov(code.rax, (u64)mfc0);
      code.call(code.rax);
      break;
    case 0x01:
      code.mov(code.rsi, (u64)instr);
      code.mov(code.rax, (u64)dmfc0);
      code.call(code.rax);
      break;
    case 0x04:
      code.mov(code.rsi, (u64)instr);
      code.mov(code.rax, (uintptr_t)mtc0);
      code.call(code.rax);
      break;
    case 0x05:
      code.mov(code.rsi, (u64)instr);
      code.mov(code.rax, (u64)dmtc0);
      code.call(code.rax);
      break;
    case 0x10 ... 0x1F:
      switch(mask_cop2) {
        case 0x01:
          code.mov(code.rax, (u64)tlbr);
          code.call(code.rax);
          break;
        case 0x02:
          code.and_(code.dword[code.rdi + offsetof(Registers, cop0.index)], 0x3F);
          code.mov(code.rsi, code.dword[code.rdi]);
          code.mov(code.rax, (u64)tlbw);
          code.call(code.rax);
          break;
        case 0x06:
          code.mov(code.rax, (u64)regs.cop0.GetRandom());
          code.mov(code.rsi, code.rax);
          code.mov(code.rax, (u64)tlbw);
          code.call(code.rax);
          break;
        case 0x08:
          code.mov(code.rax, (u64)tlbp);
          code.call(code.rax);
          break;
        case 0x18:
          code.mov(code.rax, (u64)eret);
          code.call(code.rax);
          break;
        default: Util::panic("Unimplemented COP0 function {} {} ({:08X}) ({:016lX})", mask_cop2 >> 3, mask_cop2 & 7, instr, regs.oldPC);
      }
      break;
    default: Util::panic("Unimplemented COP0 instruction {} {}", mask_cop >> 4, mask_cop & 7);
  }
}
}