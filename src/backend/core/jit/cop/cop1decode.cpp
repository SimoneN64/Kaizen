#include <jit/cop/cop1decode.hpp>
#include <jit/instructions.hpp>
#include <Registers.hpp>

namespace n64 {
bool cop1IsEndBlock(u32 instr) {
  u8 mask_sub = (instr >> 21) & 0x1F;
  u8 mask_fun = instr & 0x3F;
  u8 mask_branch = (instr >> 16) & 0x1F;

  switch (mask_sub) {
    case 0x03:
    case 0x07:
      Util::panic("[RECOMPILER] FPU Reserved instruction exception! {:08X}", instr);
    case 0x08: return true;
    default: return false;
  }
}

void cop1Emit(JIT& cpu, u32 instr) {
  Xbyak::CodeGenerator& code = cpu.code;
  Registers& regs = cpu.regs;

  u8 mask_sub = (instr >> 21) & 0x1F;
  u8 mask_fun = instr & 0x3F;
  u8 mask_branch = (instr >> 16) & 0x1F;

  switch(mask_sub) {
    // 000r_rccc
    case 0x00:
      code.mov(rax, (u64)mfc1);
      code.call(rax);
      break;
    case 0x01:
      code.mov(rax, (u64)dmfc1);
      code.call(rax);
      break;
    case 0x02:
      code.mov(rax, (u64)cfc1);
      code.call(rax);
      break;
    case 0x03:
      Util::panic("[RECOMPILER] FPU Reserved instruction exception! {:08X}", instr);
    case 0x04:
      code.mov(rax, (u64)mtc1);
      code.call(rax);
      break;
    case 0x05:
      code.mov(rax, (u64)dmtc1);
      code.call(rax);
      break;
    case 0x06:
      code.mov(rax, (u64)ctc1);
      code.call(rax);
      break;
    case 0x07:
      Util::panic("[RECOMPILER] FPU Reserved instruction exception! {:08X}", instr);
    case 0x08:
      switch(mask_branch) {
        case 0:
          code.mov(regArg2, !regs.cop1.fcr31.compare);
          code.mov(rax, (u64)b);
          code.call(rax);
          break;
        case 1:
          code.mov(regArg2, regs.cop1.fcr31.compare);
          code.mov(rax, (u64)b);
          code.call(rax);
          break;
        case 2:
          code.mov(regArg2, !regs.cop1.fcr31.compare);
          code.mov(rax, (u64)bl);
          code.call(rax);
          break;
        case 3:
          code.mov(regArg2, regs.cop1.fcr31.compare);
          code.mov(rax, (u64)bl);
          code.call(rax);
          break;
        default: Util::panic("Undefined BC COP1 {:02X}", mask_branch);
      }
      break;
    case 0x10: // s
      switch(mask_fun) {
        case 0x00:
          code.mov(rax, (u64)adds);
          code.call(rax);
          break;
        case 0x01:
          code.mov(rax, (u64)subs);
          code.call(rax);
          break;
        case 0x02:
          code.mov(rax, (u64)muls);
          code.call(rax);
          break;
        case 0x03:
          code.mov(rax, (u64)divs);
          code.call(rax);
          break;
        case 0x04:
          code.mov(rax, (u64)sqrts);
          code.call(rax);
          break;
        case 0x05:
          code.mov(rax, (u64)abss);
          code.call(rax);
          break;
        case 0x06:
          code.mov(rax, (u64)movs);
          code.call(rax);
          break;
        case 0x07:
          code.mov(rax, (u64)negs);
          code.call(rax);
          break;
        case 0x08:
          code.mov(rax, (u64)roundls);
          code.call(rax);
          break;
        case 0x09:
          code.mov(rax, (u64)truncls);
          code.call(rax);
          break;
        case 0x0A:
          code.mov(rax, (u64)ceills);
          code.call(rax);
          break;
        case 0x0B:
          code.mov(rax, (u64)floorls);
          code.call(rax);
          break;
        case 0x0C:
          code.mov(rax, (u64)roundws);
          code.call(rax);
          break;
        case 0x0D:
          code.mov(rax, (u64)truncws);
          code.call(rax);
          break;
        case 0x0E:
          code.mov(rax, (u64)ceilws);
          code.call(rax);
          break;
        case 0x0F:
          code.mov(rax, (u64)floorws);
          code.call(rax);
          break;
        case 0x20:
          Util::panic("[RECOMPILER] FPU Reserved instruction exception! {:08X}", instr);
        case 0x21:
          code.mov(rax, (u64)cvtds);
          code.call(rax);
          break;
        case 0x24:
          code.mov(rax, (u64)cvtws);
          code.call(rax);
          break;
        case 0x25:
          code.mov(rax, (u64)cvtls);
          code.call(rax);
          break;
        case 0x30:
          code.mov(rax, (u64)ccond<float>);
          code.mov(regArg2, F);
          code.call(rax);
          break;
        case 0x31:
          code.mov(rax, (u64)ccond<float>);
          code.mov(regArg2, UN);
          code.call(rax);
          break;
        case 0x32:
          code.mov(rax, (u64)ccond<float>);
          code.mov(regArg2, EQ);
          code.call(rax);
          break;
        case 0x33:
          code.mov(rax, (u64)ccond<float>);
          code.mov(regArg2, UEQ);
          code.call(rax);
          break;
        case 0x34:
          code.mov(rax, (u64)ccond<float>);
          code.mov(regArg2, OLT);
          code.call(rax);
          break;
        case 0x35:
          code.mov(rax, (u64)ccond<float>);
          code.mov(regArg2, ULT);
          code.call(rax);
          break;
        case 0x36:
          code.mov(rax, (u64)ccond<float>);
          code.mov(regArg2, OLE);
          code.call(rax);
          break;
        case 0x37:
          code.mov(rax, (u64)ccond<float>);
          code.mov(regArg2, ULE);
          code.call(rax);
          break;
        case 0x38:
          code.mov(rax, (u64)ccond<float>);
          code.mov(regArg2, SF);
          code.call(rax);
          break;
        case 0x39:
          code.mov(rax, (u64)ccond<float>);
          code.mov(regArg2, NGLE);
          code.call(rax);
          break;
        case 0x3A:
          code.mov(rax, (u64)ccond<float>);
          code.mov(regArg2, SEQ);
          code.call(rax);
          break;
        case 0x3B:
          code.mov(rax, (u64)ccond<float>);
          code.mov(regArg2, NGL);
          code.call(rax);
          break;
        case 0x3C:
          code.mov(rax, (u64)ccond<float>);
          code.mov(regArg2, LT);
          code.call(rax);
          break;
        case 0x3D:
          code.mov(rax, (u64)ccond<float>);
          code.mov(regArg2, NGE);
          code.call(rax);
          break;
        case 0x3E:
          code.mov(rax, (u64)ccond<float>);
          code.mov(regArg2, LE);
          code.call(rax);
          break;
        case 0x3F:
          code.mov(rax, (u64)ccond<float>);
          code.mov(regArg2, NGT);
          code.call(rax);
          break;
        default: Util::panic("Unimplemented COP1 function S[{} {}] ({:08X}) ({:016X})", mask_fun >> 3, mask_fun & 7, instr, (u64)regs.oldPC);
      }
      break;
    case 0x11: // d
      switch(mask_fun) {
        case 0x00:
          code.mov(rax, (u64)addd);
          code.call(rax);
          break;
        case 0x01:
          code.mov(rax, (u64)subd);
          code.call(rax);
          break;
        case 0x02:
          code.mov(rax, (u64)muld);
          code.call(rax);
          break;
        case 0x03:
          code.mov(rax, (u64)divd);
          code.call(rax);
          break;
        case 0x04:
          code.mov(rax, (u64)sqrtd);
          code.call(rax);
          break;
        case 0x05:
          code.mov(rax, (u64)absd);
          code.call(rax);
          break;
        case 0x06:
          code.mov(rax, (u64)movd);
          code.call(rax);
          break;
        case 0x07:
          code.mov(rax, (u64)negd);
          code.call(rax);
          break;
        case 0x08:
          code.mov(rax, (u64)roundld);
          code.call(rax);
          break;
        case 0x09:
          code.mov(rax, (u64)truncld);
          code.call(rax);
          break;
        case 0x0A:
          code.mov(rax, (u64)ceilld);
          code.call(rax);
          break;
        case 0x0B:
          code.mov(rax, (u64)floorld);
          code.call(rax);
          break;
        case 0x0C:
          code.mov(rax, (u64)roundwd);
          code.call(rax);
          break;
        case 0x0D:
          code.mov(rax, (u64)truncwd);
          code.call(rax);
          break;
        case 0x0E:
          code.mov(rax, (u64)ceilwd);
          code.call(rax);
          break;
        case 0x0F:
          code.mov(rax, (u64)floorwd);
          code.call(rax);
          break;
        case 0x20:
          code.mov(rax, (u64)cvtsd);
          code.call(rax);
          break;
        case 0x21:
          Util::panic("[RECOMPILER] FPU Reserved instruction exception! {:08X}", instr);
        case 0x24:
          code.mov(rax, (u64)cvtwd);
          code.call(rax);
          break;
        case 0x25:
          code.mov(rax, (u64)cvtld);
          code.call(rax);
          break;
        case 0x30:
          code.mov(rax, (u64)ccond<double>);
          code.mov(regArg2, F);
          code.call(rax);
          break;
        case 0x31:
          code.mov(rax, (u64)ccond<double>);
          code.mov(regArg2, UN);
          code.call(rax);
          break;
        case 0x32:
          code.mov(rax, (u64)ccond<double>);
          code.mov(regArg2, EQ);
          code.call(rax);
          break;
        case 0x33:
          code.mov(rax, (u64)ccond<double>);
          code.mov(regArg2, UEQ);
          code.call(rax);
          break;
        case 0x34:
          code.mov(rax, (u64)ccond<double>);
          code.mov(regArg2, OLT);
          code.call(rax);
          break;
        case 0x35:
          code.mov(rax, (u64)ccond<double>);
          code.mov(regArg2, ULT);
          code.call(rax);
          break;
        case 0x36:
          code.mov(rax, (u64)ccond<double>);
          code.mov(regArg2, OLE);
          code.call(rax);
          break;
        case 0x37:
          code.mov(rax, (u64)ccond<double>);
          code.mov(regArg2, ULE);
          code.call(rax);
          break;
        case 0x38:
          code.mov(rax, (u64)ccond<double>);
          code.mov(regArg2, SF);
          code.call(rax);
          break;
        case 0x39:
          code.mov(rax, (u64)ccond<double>);
          code.mov(regArg2, NGLE);
          code.call(rax);
          break;
        case 0x3A:
          code.mov(rax, (u64)ccond<double>);
          code.mov(regArg2, SEQ);
          code.call(rax);
          break;
        case 0x3B:
          code.mov(rax, (u64)ccond<double>);
          code.mov(regArg2, NGL);
          code.call(rax);
          break;
        case 0x3C:
          code.mov(rax, (u64)ccond<double>);
          code.mov(regArg2, LT);
          code.call(rax);
          break;
        case 0x3D:
          code.mov(rax, (u64)ccond<double>);
          code.mov(regArg2, NGE);
          code.call(rax);
          break;
        case 0x3E:
          code.mov(rax, (u64)ccond<double>);
          code.mov(regArg2, LE);
          code.call(rax);
          break;
        case 0x3F:
          code.mov(rax, (u64)ccond<double>);
          code.mov(regArg2, NGT);
          code.call(rax);
          break;
        default: Util::panic("Unimplemented COP1 function D[{} {}] ({:08X}) ({:016X})", mask_fun >> 3, mask_fun & 7, instr, (u64)regs.oldPC);
      }
      break;
    case 0x14: // w
      switch(mask_fun) {
        case 0x01:
          code.mov(rax, (u64)subw);
          code.call(rax);
          break;
        case 0x05:
          code.mov(rax, (u64)absw);
          code.call(rax);
          break;
        case 0x02:
          code.mov(rax, (u64)mulw);
          code.call(rax);
          break;
        case 0x06:
          code.mov(rax, (u64)movw);
          code.call(rax);
          break;
        case 0x20:
          code.mov(rax, (u64)cvtsw);
          code.call(rax);
          break;
        case 0x21:
          code.mov(rax, (u64)cvtdw);
          code.call(rax);
          break;
        case 0x24:
          Util::panic("[RECOMPILER] FPU Reserved instruction exception! {:08X}", instr);
        default: Util::panic("Unimplemented COP1 function W[{} {}] ({:08X}) ({:016X})", mask_fun >> 3, mask_fun & 7, instr, (u64)regs.oldPC);
      }
      break;
    case 0x15: // l
      switch(mask_fun) {
        case 0x01:
          code.mov(rax, (u64)subl);
          code.call(rax);
          break;
        case 0x05:
          code.mov(rax, (u64)absl);
          code.call(rax);
          break;
        case 0x02:
          code.mov(rax, (u64)mull);
          code.call(rax);
          break;
        case 0x06:
          code.mov(rax, (u64)movl);
          code.call(rax);
          break;
        case 0x20:
          code.mov(rax, (u64)cvtsl);
          code.call(rax);
          break;
        case 0x21:
          code.mov(rax, (u64)cvtdl);
          code.call(rax);
          break;
        case 0x24:
          Util::panic("[RECOMPILER] FPU Reserved instruction exception! {:08X}", instr);
        case 0x25:
          Util::panic("[RECOMPILER] FPU Reserved instruction exception! {:08X}", instr);
        default: Util::panic("Unimplemented COP1 function L[{} {}] ({:08X}) ({:016X})", mask_fun >> 3, mask_fun & 7, instr, (u64)regs.oldPC);
      }
      break;
    default: Util::panic("Unimplemented COP1 instruction {} {}", mask_sub >> 3, mask_sub & 7);
  }
}
}