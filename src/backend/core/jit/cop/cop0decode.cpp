#include <jit/cop/cop0decode.hpp>
#include <jit/cop/cop0instructions.hpp>
#include <Registers.hpp>

namespace n64 {
void cop0Decode(JIT& cpu, u32 instr) {
  u8 mask_cop = (instr >> 21) & 0x1F;
  u8 mask_cop2 = instr & 0x3F;
  Xbyak::CodeGenerator& code = cpu.code;
  Registers& regs = cpu.regs;

  switch(mask_cop) {
    case 0x00:
      code.mov(rax, (u64)mfc0);
      code.call(rax);
      break;
    case 0x01:
      code.mov(rax, (u64)dmfc0);
      code.call(rax);
      break;
    case 0x04:
      code.mov(rax, (uintptr_t)mtc0);
      code.call(rax);
      break;
    case 0x05:
      code.mov(rax, (u64)dmtc0);
      code.call(rax);
      break;
    case 0x10 ... 0x1F:
      switch(mask_cop2) {
        case 0x01:
          code.mov(rax, (u64)tlbr);
          code.call(rax);
          break;
        case 0x02:
          code.mov(regArg1, dword[rdi + REG_OFFSET(cop0.index, &cpu)]);
          code.and_(regArg1, 0x3F);
          code.mov(rax, (u64)tlbw);
          code.call(rax);
          break;
        case 0x06:
          code.mov(regArg1, (u64)regs.cop0.GetRandom());
          code.mov(rax, (u64)tlbw);
          code.call(rax);
          break;
        case 0x08:
          code.mov(rax, (u64)tlbp);
          code.call(rax);
          break;
        case 0x18:
          code.mov(rax, (u64)eret);
          code.call(rax);
          break;
        default: Util::panic("Unimplemented COP0 function {} {} ({:08X}) ({:016lX})", mask_cop2 >> 3, mask_cop2 & 7, instr, regs.oldPC);
      }
      break;
    default: Util::panic("Unimplemented COP0 instruction {} {}", mask_cop >> 4, mask_cop & 7);
  }
}
}