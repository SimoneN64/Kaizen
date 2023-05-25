#include <cachedinterpreter/cop/cop1decode.hpp>
#include <Registers.hpp>

namespace n64 {
auto cop1GetFunc(CachedInterpreter& cpu, u32 instr) {
  if(!regs.cop0.status.cu1) {
    FireException(regs, ExceptionCode::CoprocessorUnusable, 1, true);
    return;
  }

  u8 mask_sub = (instr >> 21) & 0x1F;
  u8 mask_fun = instr & 0x3F;
  u8 mask_branch = (instr >> 16) & 0x1F;
  switch(mask_sub) {
    // 000r_rccc
    case 0x00: return mfc1(regs, instr);
    case 0x01: return dmfc1(regs, instr);
    case 0x02: return cfc1(regs, instr);
    case 0x03: return FireException(regs, ExceptionCode::ReservedInstruction, 1, true);
    case 0x04: return mtc1(regs, instr);
    case 0x05: return dmtc1(regs, instr);
    case 0x06: return ctc1(regs, instr);
    case 0x07: return FireException(regs, ExceptionCode::ReservedInstruction, 1, true);
    case 0x08:
      switch(mask_branch) {
        case 0: return cpu.b(instr, !regs.cop1.fcr31.compare);
        case 1: return cpu.b(instr, regs.cop1.fcr31.compare);
        case 2: return cpu.bl(instr, !regs.cop1.fcr31.compare);
        case 3: return cpu.bl(instr, regs.cop1.fcr31.compare);
        default: Util::panic("Undefined BC COP1 {:02X}", mask_branch);
      }
      break;
    case 0x10: // s
      switch(mask_fun) {
        case 0x00: return adds(regs, instr);
        case 0x01: return subs(regs, instr);
        case 0x02: return muls(regs, instr);
        case 0x03: return divs(regs, instr);
        case 0x04: return sqrts(regs, instr);
        case 0x05: return abss(regs, instr);
        case 0x06: return movs(regs, instr);
        case 0x07: return negs(regs, instr);
        case 0x08: return roundls(regs, instr);
        case 0x09: return truncls(regs, instr);
        case 0x0A: return ceills(regs, instr);
        case 0x0B: return floorls(regs, instr);
        case 0x0C: return roundws(regs, instr);
        case 0x0D: return truncws(regs, instr);
        case 0x0E: return ceilws(regs, instr);
        case 0x0F: return floorws(regs, instr);
        case 0x20: return FireException(regs, ExceptionCode::ReservedInstruction, 1, true);
        case 0x21: return cvtds(regs, instr);
        case 0x24: return cvtws(regs, instr);
        case 0x25: return cvtls(regs, instr);
        case 0x30: return ccond<float>(regs, instr, F);
        case 0x31: return ccond<float>(regs, instr, UN);
        case 0x32: return ccond<float>(regs, instr, EQ);
        case 0x33: return ccond<float>(regs, instr, UEQ);
        case 0x34: return ccond<float>(regs, instr, OLT);
        case 0x35: return ccond<float>(regs, instr, ULT);
        case 0x36: return ccond<float>(regs, instr, OLE);
        case 0x37: return ccond<float>(regs, instr, ULE);
        case 0x38: return ccond<float>(regs, instr, SF);
        case 0x39: return ccond<float>(regs, instr, NGLE);
        case 0x3A: return ccond<float>(regs, instr, SEQ);
        case 0x3B: return ccond<float>(regs, instr, NGL);
        case 0x3C: return ccond<float>(regs, instr, LT);
        case 0x3D: return ccond<float>(regs, instr, NGE);
        case 0x3E: return ccond<float>(regs, instr, LE);
        case 0x3F: return ccond<float>(regs, instr, NGT);
        default: Util::panic("Unimplemented COP1 function S[{} {}] ({:08X}) ({:016X})", mask_fun >> 3, mask_fun & 7, instr, (u64)regs.oldPC);
      }
      break;
    case 0x11: // d
      switch(mask_fun) {
        case 0x00: return addd(regs, instr);
        case 0x01: return subd(regs, instr);
        case 0x02: return muld(regs, instr);
        case 0x03: return divd(regs, instr);
        case 0x04: return sqrtd(regs, instr);
        case 0x05: return absd(regs, instr);
        case 0x06: return movd(regs, instr);
        case 0x07: return negd(regs, instr);
        case 0x08: return roundld(regs, instr);
        case 0x09: return truncld(regs, instr);
        case 0x0A: return ceilld(regs, instr);
        case 0x0B: return floorld(regs, instr);
        case 0x0C: return roundwd(regs, instr);
        case 0x0D: return truncwd(regs, instr);
        case 0x0E: return ceilwd(regs, instr);
        case 0x0F: return floorwd(regs, instr);
        case 0x20: return cvtsd(regs, instr);
        case 0x21:
          return FireException(regs, ExceptionCode::ReservedInstruction, 1, true);
        case 0x24: return cvtwd(regs, instr);
        case 0x25: return cvtld(regs, instr);
        case 0x30: return ccond<double>(regs, instr, F);
        case 0x31: return ccond<double>(regs, instr, UN);
        case 0x32: return ccond<double>(regs, instr, EQ);
        case 0x33: return ccond<double>(regs, instr, UEQ);
        case 0x34: return ccond<double>(regs, instr, OLT);
        case 0x35: return ccond<double>(regs, instr, ULT);
        case 0x36: return ccond<double>(regs, instr, OLE);
        case 0x37: return ccond<double>(regs, instr, ULE);
        case 0x38: return ccond<double>(regs, instr, SF);
        case 0x39: return ccond<double>(regs, instr, NGLE);
        case 0x3A: return ccond<double>(regs, instr, SEQ);
        case 0x3B: return ccond<double>(regs, instr, NGL);
        case 0x3C: return ccond<double>(regs, instr, LT);
        case 0x3D: return ccond<double>(regs, instr, NGE);
        case 0x3E: return ccond<double>(regs, instr, LE);
        case 0x3F: return ccond<double>(regs, instr, NGT);
        default: Util::panic("Unimplemented COP1 function D[{} {}] ({:08X}) ({:016X})", mask_fun >> 3, mask_fun & 7, instr, (u64)regs.oldPC);
      }
      break;
    case 0x14: // w
      switch(mask_fun) {
        case 0x01: return subw(regs, instr);
        case 0x05: return absw(regs, instr);
        case 0x02: return mulw(regs, instr);
        case 0x06: return movw(regs, instr);
        case 0x20: return cvtsw(regs, instr);
        case 0x21: return cvtdw(regs, instr);
        case 0x24: return FireException(regs, ExceptionCode::ReservedInstruction, 1, true);
        default: Util::panic("Unimplemented COP1 function W[{} {}] ({:08X}) ({:016X})", mask_fun >> 3, mask_fun & 7, instr, (u64)regs.oldPC);
      }
      break;
    case 0x15: // l
      switch(mask_fun) {
        case 0x01: return subl(regs, instr);
        case 0x05: return absl(regs, instr);
        case 0x02: return mull(regs, instr);
        case 0x06: return movl(regs, instr);
        case 0x20: return cvtsl(regs, instr);
        case 0x21: return cvtdl(regs, instr);
        case 0x24: return FireException(regs, ExceptionCode::ReservedInstruction, 1, true);
        case 0x25: return FireException(regs, ExceptionCode::ReservedInstruction, 1, true);
        default: Util::panic("Unimplemented COP1 function L[{} {}] ({:08X}) ({:016X})", mask_fun >> 3, mask_fun & 7, instr, (u64)regs.oldPC);
      }
      break;
    default: Util::panic("Unimplemented COP1 instruction {} {}", mask_sub >> 3, mask_sub & 7);
  }
}
}