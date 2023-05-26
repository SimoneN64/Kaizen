#include <cachedinterpreter/cop/cop1decode.hpp>
#include <cachedinterpreter/instructions.hpp>
#include <Registers.hpp>

namespace n64 {
CachedFn cop1GetFunc(CachedInterpreter& cpu, u32 instr) {
  if(!cpu.regs.cop0.status.cu1) {
    return [](CachedInterpreter& cpu, u32) { FireException(cpu.regs, ExceptionCode::CoprocessorUnusable, 1, true); };
  }

  u8 mask_sub = (instr >> 21) & 0x1F;
  u8 mask_fun = instr & 0x3F;
  u8 mask_branch = (instr >> 16) & 0x1F;
  switch(mask_sub) {
    // 000r_rccc
    case 0x00: return [](CachedInterpreter&cpu, u32 instr) { cpu.regs.cop1.mfc1(cpu.regs, instr); };
    case 0x01: return [](CachedInterpreter&cpu, u32 instr) { cpu.regs.cop1.dmfc1(cpu.regs, instr); };
    case 0x02: return [](CachedInterpreter&cpu, u32 instr) { cpu.regs.cop1.cfc1(cpu.regs, instr); };
    case 0x03: return [](CachedInterpreter&cpu, u32) { FireException(cpu.regs, ExceptionCode::ReservedInstruction, 1, true); };
    case 0x04: return [](CachedInterpreter&cpu, u32 instr) { cpu.regs.cop1.mtc1(cpu.regs, instr); };
    case 0x05: return [](CachedInterpreter&cpu, u32 instr) { cpu.regs.cop1.dmtc1(cpu.regs, instr); };
    case 0x06: return [](CachedInterpreter&cpu, u32 instr) { cpu.regs.cop1.ctc1(cpu.regs, instr); };
    case 0x07: return [](CachedInterpreter&cpu, u32) { FireException(cpu.regs, ExceptionCode::ReservedInstruction, 1, true); };
    case 0x08:
      switch(mask_branch) {
        case 0: return [](CachedInterpreter& cpu, u32 instr) { b(cpu, instr, !cpu.regs.cop1.fcr31.compare); };
        case 1: return [](CachedInterpreter& cpu, u32 instr) { b(cpu, instr, cpu.regs.cop1.fcr31.compare); };
        case 2: return [](CachedInterpreter& cpu, u32 instr) { bl(cpu, instr, !cpu.regs.cop1.fcr31.compare); };
        case 3: return [](CachedInterpreter& cpu, u32 instr) { bl(cpu, instr, cpu.regs.cop1.fcr31.compare); };
        default: Util::panic("Undefined BC COP1 {:02X}", mask_branch);
      }
      break;
    case 0x10: // s
      switch(mask_fun) {
        case 0x00: return [](CachedInterpreter& cpu, u32 instr) { cpu.regs.cop1.adds(cpu.regs, instr); };
        case 0x01: return [](CachedInterpreter& cpu, u32 instr) { cpu.regs.cop1.subs(cpu.regs, instr); };
        case 0x02: return [](CachedInterpreter& cpu, u32 instr) { cpu.regs.cop1.muls(cpu.regs, instr); };
        case 0x03: return [](CachedInterpreter& cpu, u32 instr) { cpu.regs.cop1.divs(cpu.regs, instr); };
        case 0x04: return [](CachedInterpreter& cpu, u32 instr) { cpu.regs.cop1.sqrts(cpu.regs, instr); };
        case 0x05: return [](CachedInterpreter& cpu, u32 instr) { cpu.regs.cop1.abss(cpu.regs, instr); };
        case 0x06: return [](CachedInterpreter& cpu, u32 instr) { cpu.regs.cop1.movs(cpu.regs, instr); };
        case 0x07: return [](CachedInterpreter& cpu, u32 instr) { cpu.regs.cop1.negs(cpu.regs, instr); };
        case 0x08: return [](CachedInterpreter& cpu, u32 instr) { cpu.regs.cop1.roundls(cpu.regs, instr); };
        case 0x09: return [](CachedInterpreter& cpu, u32 instr) { cpu.regs.cop1.truncls(cpu.regs, instr); };
        case 0x0A: return [](CachedInterpreter& cpu, u32 instr) { cpu.regs.cop1.ceills(cpu.regs, instr); };
        case 0x0B: return [](CachedInterpreter& cpu, u32 instr) { cpu.regs.cop1.floorls(cpu.regs, instr); };
        case 0x0C: return [](CachedInterpreter& cpu, u32 instr) { cpu.regs.cop1.roundws(cpu.regs, instr); };
        case 0x0D: return [](CachedInterpreter& cpu, u32 instr) { cpu.regs.cop1.truncws(cpu.regs, instr); };
        case 0x0E: return [](CachedInterpreter& cpu, u32 instr) { cpu.regs.cop1.ceilws(cpu.regs, instr); };
        case 0x0F: return [](CachedInterpreter& cpu, u32 instr) { cpu.regs.cop1.floorws(cpu.regs, instr); };
        case 0x20: return [](CachedInterpreter& cpu, u32) { FireException(cpu.regs, ExceptionCode::ReservedInstruction, 1, true); };
        case 0x21: return [](CachedInterpreter& cpu, u32 instr) { cpu.regs.cop1.cvtds(cpu.regs, instr); };
        case 0x24: return [](CachedInterpreter& cpu, u32 instr) { cpu.regs.cop1.cvtws(cpu.regs, instr); };
        case 0x25: return [](CachedInterpreter& cpu, u32 instr) { cpu.regs.cop1.cvtls(cpu.regs, instr); };
        case 0x30: return [](CachedInterpreter& cpu, u32 instr) { cpu.regs.cop1.ccond<float>(cpu.regs, instr, F); };
        case 0x31: return [](CachedInterpreter& cpu, u32 instr) { cpu.regs.cop1.ccond<float>(cpu.regs, instr, UN); };
        case 0x32: return [](CachedInterpreter& cpu, u32 instr) { cpu.regs.cop1.ccond<float>(cpu.regs, instr, EQ); };
        case 0x33: return [](CachedInterpreter& cpu, u32 instr) { cpu.regs.cop1.ccond<float>(cpu.regs, instr, UEQ); };
        case 0x34: return [](CachedInterpreter& cpu, u32 instr) { cpu.regs.cop1.ccond<float>(cpu.regs, instr, OLT); };
        case 0x35: return [](CachedInterpreter& cpu, u32 instr) { cpu.regs.cop1.ccond<float>(cpu.regs, instr, ULT); };
        case 0x36: return [](CachedInterpreter& cpu, u32 instr) { cpu.regs.cop1.ccond<float>(cpu.regs, instr, OLE); };
        case 0x37: return [](CachedInterpreter& cpu, u32 instr) { cpu.regs.cop1.ccond<float>(cpu.regs, instr, ULE); };
        case 0x38: return [](CachedInterpreter& cpu, u32 instr) { cpu.regs.cop1.ccond<float>(cpu.regs, instr, SF); };
        case 0x39: return [](CachedInterpreter& cpu, u32 instr) { cpu.regs.cop1.ccond<float>(cpu.regs, instr, NGLE); };
        case 0x3A: return [](CachedInterpreter& cpu, u32 instr) { cpu.regs.cop1.ccond<float>(cpu.regs, instr, SEQ); };
        case 0x3B: return [](CachedInterpreter& cpu, u32 instr) { cpu.regs.cop1.ccond<float>(cpu.regs, instr, NGL); };
        case 0x3C: return [](CachedInterpreter& cpu, u32 instr) { cpu.regs.cop1.ccond<float>(cpu.regs, instr, LT); };
        case 0x3D: return [](CachedInterpreter& cpu, u32 instr) { cpu.regs.cop1.ccond<float>(cpu.regs, instr, NGE); };
        case 0x3E: return [](CachedInterpreter& cpu, u32 instr) { cpu.regs.cop1.ccond<float>(cpu.regs, instr, LE); };
        case 0x3F: return [](CachedInterpreter& cpu, u32 instr) { cpu.regs.cop1.ccond<float>(cpu.regs, instr, NGT); };
        default: Util::panic("Unimplemented COP1 function S[{} {}] ({:08X}) ({:016X})", mask_fun >> 3, mask_fun & 7, instr, (u64)cpu.regs.oldPC);
      }
      break;
    case 0x11: // d
      switch(mask_fun) {
        case 0x00: return [](CachedInterpreter& cpu, u32 instr) { cpu.regs.cop1.addd(cpu.regs, instr); };
        case 0x01: return [](CachedInterpreter& cpu, u32 instr) { cpu.regs.cop1.subd(cpu.regs, instr); };
        case 0x02: return [](CachedInterpreter& cpu, u32 instr) { cpu.regs.cop1.muld(cpu.regs, instr); };
        case 0x03: return [](CachedInterpreter& cpu, u32 instr) { cpu.regs.cop1.divd(cpu.regs, instr); };
        case 0x04: return [](CachedInterpreter& cpu, u32 instr) { cpu.regs.cop1.sqrtd(cpu.regs, instr); };
        case 0x05: return [](CachedInterpreter& cpu, u32 instr) { cpu.regs.cop1.absd(cpu.regs, instr); };
        case 0x06: return [](CachedInterpreter& cpu, u32 instr) { cpu.regs.cop1.movd(cpu.regs, instr); };
        case 0x07: return [](CachedInterpreter& cpu, u32 instr) { cpu.regs.cop1.negd(cpu.regs, instr); };
        case 0x08: return [](CachedInterpreter& cpu, u32 instr) { cpu.regs.cop1.roundld(cpu.regs, instr); };
        case 0x09: return [](CachedInterpreter& cpu, u32 instr) { cpu.regs.cop1.truncld(cpu.regs, instr); };
        case 0x0A: return [](CachedInterpreter& cpu, u32 instr) { cpu.regs.cop1.ceilld(cpu.regs, instr); };
        case 0x0B: return [](CachedInterpreter& cpu, u32 instr) { cpu.regs.cop1.floorld(cpu.regs, instr); };
        case 0x0C: return [](CachedInterpreter& cpu, u32 instr) { cpu.regs.cop1.roundwd(cpu.regs, instr); };
        case 0x0D: return [](CachedInterpreter& cpu, u32 instr) { cpu.regs.cop1.truncwd(cpu.regs, instr); };
        case 0x0E: return [](CachedInterpreter& cpu, u32 instr) { cpu.regs.cop1.ceilwd(cpu.regs, instr); };
        case 0x0F: return [](CachedInterpreter& cpu, u32 instr) { cpu.regs.cop1.floorwd(cpu.regs, instr); };
        case 0x20: return [](CachedInterpreter& cpu, u32 instr) { cpu.regs.cop1.cvtsd(cpu.regs, instr); };
        case 0x21: return [](CachedInterpreter&cpu, u32) { FireException(cpu.regs, ExceptionCode::ReservedInstruction, 1, true); };
        case 0x24: return [](CachedInterpreter& cpu, u32 instr) { cpu.regs.cop1.cvtwd(cpu.regs, instr); };
        case 0x25: return [](CachedInterpreter& cpu, u32 instr) { cpu.regs.cop1.cvtld(cpu.regs, instr); };
        case 0x30: return [](CachedInterpreter& cpu, u32 instr) { cpu.regs.cop1.ccond<double>(cpu.regs, instr, F); };
        case 0x31: return [](CachedInterpreter& cpu, u32 instr) { cpu.regs.cop1.ccond<double>(cpu.regs, instr, UN); };
        case 0x32: return [](CachedInterpreter& cpu, u32 instr) { cpu.regs.cop1.ccond<double>(cpu.regs, instr, EQ); };
        case 0x33: return [](CachedInterpreter& cpu, u32 instr) { cpu.regs.cop1.ccond<double>(cpu.regs, instr, UEQ); };
        case 0x34: return [](CachedInterpreter& cpu, u32 instr) { cpu.regs.cop1.ccond<double>(cpu.regs, instr, OLT); };
        case 0x35: return [](CachedInterpreter& cpu, u32 instr) { cpu.regs.cop1.ccond<double>(cpu.regs, instr, ULT); };
        case 0x36: return [](CachedInterpreter& cpu, u32 instr) { cpu.regs.cop1.ccond<double>(cpu.regs, instr, OLE); };
        case 0x37: return [](CachedInterpreter& cpu, u32 instr) { cpu.regs.cop1.ccond<double>(cpu.regs, instr, ULE); };
        case 0x38: return [](CachedInterpreter& cpu, u32 instr) { cpu.regs.cop1.ccond<double>(cpu.regs, instr, SF); };
        case 0x39: return [](CachedInterpreter& cpu, u32 instr) { cpu.regs.cop1.ccond<double>(cpu.regs, instr, NGLE); };
        case 0x3A: return [](CachedInterpreter& cpu, u32 instr) { cpu.regs.cop1.ccond<double>(cpu.regs, instr, SEQ); };
        case 0x3B: return [](CachedInterpreter& cpu, u32 instr) { cpu.regs.cop1.ccond<double>(cpu.regs, instr, NGL); };
        case 0x3C: return [](CachedInterpreter& cpu, u32 instr) { cpu.regs.cop1.ccond<double>(cpu.regs, instr, LT); };
        case 0x3D: return [](CachedInterpreter& cpu, u32 instr) { cpu.regs.cop1.ccond<double>(cpu.regs, instr, NGE); };
        case 0x3E: return [](CachedInterpreter& cpu, u32 instr) { cpu.regs.cop1.ccond<double>(cpu.regs, instr, LE); };
        case 0x3F: return [](CachedInterpreter& cpu, u32 instr) { cpu.regs.cop1.ccond<double>(cpu.regs, instr, NGT); };
        default: Util::panic("Unimplemented COP1 function D[{} {}] ({:08X}) ({:016X})", mask_fun >> 3, mask_fun & 7, instr, (u64)cpu.regs.oldPC);
      }
      break;
    case 0x14: // w
      switch(mask_fun) {
        case 0x01: return [](CachedInterpreter& cpu, u32 instr) { cpu.regs.cop1.subw(cpu.regs, instr); };
        case 0x05: return [](CachedInterpreter& cpu, u32 instr) { cpu.regs.cop1.absw(cpu.regs, instr); };
        case 0x02: return [](CachedInterpreter& cpu, u32 instr) { cpu.regs.cop1.mulw(cpu.regs, instr); };
        case 0x06: return [](CachedInterpreter& cpu, u32 instr) { cpu.regs.cop1.movw(cpu.regs, instr); };
        case 0x20: return [](CachedInterpreter& cpu, u32 instr) { cpu.regs.cop1.cvtsw(cpu.regs, instr); };
        case 0x21: return [](CachedInterpreter& cpu, u32 instr) { cpu.regs.cop1.cvtdw(cpu.regs, instr); };
        case 0x24: return [](CachedInterpreter&cpu, u32) { FireException(cpu.regs, ExceptionCode::ReservedInstruction, 1, true); };
        default: Util::panic("Unimplemented COP1 function W[{} {}] ({:08X}) ({:016X})", mask_fun >> 3, mask_fun & 7, instr, (u64)cpu.regs.oldPC);
      }
      break;
    case 0x15: // l
      switch(mask_fun) {
        case 0x01: return [](CachedInterpreter& cpu, u32 instr) { cpu.regs.cop1.subl(cpu.regs, instr); };
        case 0x05: return [](CachedInterpreter& cpu, u32 instr) { cpu.regs.cop1.absl(cpu.regs, instr); };
        case 0x02: return [](CachedInterpreter& cpu, u32 instr) { cpu.regs.cop1.mull(cpu.regs, instr); };
        case 0x06: return [](CachedInterpreter& cpu, u32 instr) { cpu.regs.cop1.movl(cpu.regs, instr); };
        case 0x20: return [](CachedInterpreter& cpu, u32 instr) { cpu.regs.cop1.cvtsl(cpu.regs, instr); };
        case 0x21: return [](CachedInterpreter& cpu, u32 instr) { cpu.regs.cop1.cvtdl(cpu.regs, instr); };
        case 0x24: return [](CachedInterpreter&cpu, u32) { FireException(cpu.regs, ExceptionCode::ReservedInstruction, 1, true); };
        case 0x25: return [](CachedInterpreter&cpu, u32) { FireException(cpu.regs, ExceptionCode::ReservedInstruction, 1, true); };
        default: Util::panic("Unimplemented COP1 function L[{} {}] ({:08X}) ({:016X})", mask_fun >> 3, mask_fun & 7, instr, (u64)cpu.regs.oldPC);
      }
      break;
    default: Util::panic("Unimplemented COP1 instruction {} {}", mask_sub >> 3, mask_sub & 7);
  }
}
}