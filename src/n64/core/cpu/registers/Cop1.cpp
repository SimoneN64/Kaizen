#include <n64/core/cpu/registers/Cop1.hpp>
#include <n64/core/cpu/Registers.hpp>
#include <n64/core/Cpu.hpp>
#include <util.hpp>

namespace n64 {
void Cop1::decode(Cpu& cpu, u32 instr) {
  Registers& regs = cpu.regs;
  if(!regs.cop0.status.cu1) {
    FireException(regs, ExceptionCode::CoprocessorUnusable, 1, regs.oldPC);
    return;
  }

  u8 mask_sub = (instr >> 21) & 0x1F;
  u8 mask_fun = instr & 0x3F;
  u8 mask_branch = (instr >> 16) & 0x1F;
  switch(mask_sub) {
    // 000r_rccc
    case 0x00: mfc1(regs, instr); break;
    case 0x01: dmfc1(regs, instr); break;
    case 0x02: cfc1(regs, instr); break;
    case 0x04: mtc1(regs, instr); break;
    case 0x05: dmtc1(regs, instr); break;
    case 0x06: ctc1(regs, instr); break;
    case 0x08:
      switch(mask_branch) {
        case 0: cpu.b(instr, !regs.cop1.fcr31.compare); break;
        case 1: cpu.b(instr, regs.cop1.fcr31.compare); break;
        case 2: cpu.bl(instr, !regs.cop1.fcr31.compare); break;
        case 3: cpu.bl(instr, regs.cop1.fcr31.compare); break;
        default: util::panic("Undefined BC COP1 {:02X}\n", mask_branch);
      }
      break;
    case 0x10: // s
      switch(mask_fun) {
        case 0x00: adds(regs, instr); break;
        case 0x01: subs(regs, instr); break;
        case 0x02: muls(regs, instr); break;
        case 0x03: divs(regs, instr); break;
        case 0x04: sqrts(regs, instr); break;
        case 0x05: abss(regs, instr); break;
        case 0x06: movs(regs, instr); break;
        case 0x07: negs(regs, instr); break;
        case 0x08: roundls(regs, instr); break;
        case 0x09: truncls(regs, instr); break;
        case 0x0A: ceills(regs, instr); break;
        case 0x0B: floorls(regs, instr); break;
        case 0x0C: roundws(regs, instr); break;
        case 0x0D: truncws(regs, instr); break;
        case 0x0E: ceilws(regs, instr); break;
        case 0x0F: floorws(regs, instr); break;
        case 0x21: cvtds(regs, instr); break;
        case 0x24: cvtws(regs, instr); break;
        case 0x25: cvtls(regs, instr); break;
        case 0x30: cconds(regs, instr, F); break;
        case 0x31: cconds(regs, instr, UN); break;
        case 0x32: cconds(regs, instr, EQ); break;
        case 0x33: cconds(regs, instr, UEQ); break;
        case 0x34: cconds(regs, instr, OLT); break;
        case 0x35: cconds(regs, instr, ULT); break;
        case 0x36: cconds(regs, instr, OLE); break;
        case 0x37: cconds(regs, instr, ULE); break;
        case 0x38: cconds(regs, instr, SF); break;
        case 0x39: cconds(regs, instr, NGLE); break;
        case 0x3A: cconds(regs, instr, SEQ); break;
        case 0x3B: cconds(regs, instr, NGL); break;
        case 0x3C: cconds(regs, instr, LT); break;
        case 0x3D: cconds(regs, instr, NGE); break;
        case 0x3E: cconds(regs, instr, LE); break;
        case 0x3F: cconds(regs, instr, NGT); break;
        default: util::panic("Unimplemented COP1 function S[{} {}] ({:08X}) ({:016lX})", mask_fun >> 3, mask_fun & 7, instr, regs.oldPC);
      }
      break;
    case 0x11: // d
      switch(mask_fun) {
        case 0x00: addd(regs, instr); break;
        case 0x01: subd(regs, instr); break;
        case 0x02: muld(regs, instr); break;
        case 0x03: divd(regs, instr); break;
        case 0x04: sqrtd(regs, instr); break;
        case 0x05: absd(regs, instr); break;
        case 0x06: movd(regs, instr); break;
        case 0x07: negd(regs, instr); break;
        case 0x08: roundld(regs, instr); break;
        case 0x09: truncld(regs, instr); break;
        case 0x0A: ceilld(regs, instr); break;
        case 0x0B: floorld(regs, instr); break;
        case 0x0C: roundwd(regs, instr); break;
        case 0x0D: truncwd(regs, instr); break;
        case 0x0E: ceilwd(regs, instr); break;
        case 0x0F: floorwd(regs, instr); break;
        case 0x20: cvtsd(regs, instr); break;
        case 0x24: cvtwd(regs, instr); break;
        case 0x25: cvtld(regs, instr); break;
        case 0x30: ccondd(regs, instr, F); break;
        case 0x31: ccondd(regs, instr, UN); break;
        case 0x32: ccondd(regs, instr, EQ); break;
        case 0x33: ccondd(regs, instr, UEQ); break;
        case 0x34: ccondd(regs, instr, OLT); break;
        case 0x35: ccondd(regs, instr, ULT); break;
        case 0x36: ccondd(regs, instr, OLE); break;
        case 0x37: ccondd(regs, instr, ULE); break;
        case 0x38: ccondd(regs, instr, SF); break;
        case 0x39: ccondd(regs, instr, NGLE); break;
        case 0x3A: ccondd(regs, instr, SEQ); break;
        case 0x3B: ccondd(regs, instr, NGL); break;
        case 0x3C: ccondd(regs, instr, LT); break;
        case 0x3D: ccondd(regs, instr, NGE); break;
        case 0x3E: ccondd(regs, instr, LE); break;
        case 0x3F: ccondd(regs, instr, NGT); break;
        default: util::panic("Unimplemented COP1 function D[{} {}] ({:08X}) ({:016lX})", mask_fun >> 3, mask_fun & 7, instr, regs.oldPC);
      }
      break;
    case 0x14: // w
      switch(mask_fun) {
        case 0x01: subw(regs, instr); break;
        case 0x05: absw(regs, instr); break;
        case 0x02: mulw(regs, instr); break;
        case 0x06: movw(regs, instr); break;
        case 0x20: cvtsw(regs, instr); break;
        case 0x21: cvtdw(regs, instr); break;
        default: util::panic("Unimplemented COP1 function W[{} {}] ({:08X}) ({:016lX})", mask_fun >> 3, mask_fun & 7, instr, regs.oldPC);
      }
      break;
    case 0x15: // l
      switch(mask_fun) {
        case 0x01: subl(regs, instr); break;
        case 0x05: absl(regs, instr); break;
        case 0x02: mull(regs, instr); break;
        case 0x06: movl(regs, instr); break;
        case 0x20: cvtsl(regs, instr); break;
        case 0x21: cvtdl(regs, instr); break;
        default: util::panic("Unimplemented COP1 function L[{} {}] ({:08X}) ({:016lX})", mask_fun >> 3, mask_fun & 7, instr, regs.oldPC);
      }
      break;
    default: util::panic("Unimplemented COP1 instruction {} {}", mask_sub >> 3, mask_sub & 7);
  }
}
}