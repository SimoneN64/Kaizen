#include <cachedinterpreter/cop/cop0decode.hpp>
#include <Registers.hpp>

namespace n64 {
CachedFn cop0GetFunc(CachedInterpreter& cpu, u32 instr) {
  u8 mask_cop = (instr >> 21) & 0x1F;
  u8 mask_cop2 = instr & 0x3F;
  switch(mask_cop) {
    case 0x00: return [] (CachedInterpreter& cpu, u32 instr) { cpu.regs.cop0.mfc0(cpu.regs, instr); };
    case 0x01: return [] (CachedInterpreter& cpu, u32 instr) { cpu.regs.cop0.dmfc0(cpu.regs, instr); };
    case 0x04: return [] (CachedInterpreter& cpu, u32 instr) { cpu.regs.cop0.mtc0(cpu.regs, instr); };
    case 0x05: return [] (CachedInterpreter& cpu, u32 instr) { cpu.regs.cop0.dmtc0(cpu.regs, instr); };
    case 0x10 ... 0x1F:
      switch(mask_cop2) {
        case 0x01: return [](CachedInterpreter& cpu, u32) { cpu.regs.cop0.tlbr(); };
        case 0x02: return [](CachedInterpreter& cpu, u32 instr) { cpu.regs.cop0.tlbw(cpu.regs.cop0.index & 0x3F); };
        case 0x06: return [](CachedInterpreter& cpu, u32 instr) { cpu.regs.cop0.tlbw(cpu.regs.cop0.GetRandom()); };
        case 0x08: return [](CachedInterpreter& cpu, u32 instr) { cpu.regs.cop0.tlbp(cpu.regs); };
        case 0x18: return [](CachedInterpreter& cpu, u32 instr) { cpu.regs.cop0.eret(cpu.regs); };
        default: Util::panic("Unimplemented COP0 function {} {} ({:08X}) ({:016lX})", mask_cop2 >> 3, mask_cop2 & 7, instr, cpu.regs.oldPC);
      }
      break;
    default: Util::panic("Unimplemented COP0 instruction {} {}", mask_cop >> 4, mask_cop & 7);
  }
}
}