#include <dynarec/cop/cop0decode.hpp>
#include <Registers.hpp>
#include <dynarec/cop/cop0instructions.hpp>

namespace n64::JIT {
void cop0Decode(n64::Registers& regs, u32 instr) {
  u8 mask_cop = (instr >> 21) & 0x1F;
  u8 mask_cop2 = instr & 0x3F;
  switch(mask_cop) {
    case 0x00: mfc0(regs, instr); break;
    case 0x01: dmfc0(regs, instr); break;
    case 0x04: mtc0(regs, instr); break;
    case 0x05: dmtc0(regs, instr); break;
    case 0x10 ... 0x1F:
      switch(mask_cop2) {
        case 0x01: tlbr(regs); break;
        case 0x02: tlbw(regs.cop0.index & 0x3F, regs); break;
        case 0x06: tlbw(regs.cop0.GetRandom(), regs); break;
        case 0x08: tlbp(regs); break;
        case 0x18: eret(regs); break;
        default: util::panic("Unimplemented COP0 function {} {} ({:08X}) ({:016lX})", mask_cop2 >> 3, mask_cop2 & 7, instr, regs.oldPC);
      }
      break;
    default: util::panic("Unimplemented COP0 instruction {} {}", mask_cop >> 4, mask_cop & 7);
  }
}
}