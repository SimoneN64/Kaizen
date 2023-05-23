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
  Xbyak::CodeGenerator& code = *cpu.code;
  Registers& regs = cpu.regs;

  u8 mask_sub = (instr >> 21) & 0x1F;
  u8 mask_fun = instr & 0x3F;
  u8 mask_branch = (instr >> 16) & 0x1F;

  switch(mask_sub) {
    // 000r_rccc
    case 0x00:
      code.mov(rax, (u64)mfc1);
      cpu.emitCall(rax);
      break;
    case 0x01:
      code.mov(rax, (u64)dmfc1);
      cpu.emitCall(rax);
      break;
    case 0x02:
      code.mov(rax, (u64)cfc1);
      cpu.emitCall(rax);
      break;
    case 0x03:
      Util::panic("[RECOMPILER] FPU Reserved instruction exception! {:08X}", instr);
    case 0x04:
      code.mov(rax, (u64)mtc1);
      cpu.emitCall(rax);
      break;
    case 0x05:
      code.mov(rax, (u64)dmtc1);
      cpu.emitCall(rax);
      break;
    case 0x06:
      code.mov(rax, (u64)ctc1);
      cpu.emitCall(rax);
      break;
    case 0x07:
      Util::panic("[RECOMPILER] FPU Reserved instruction exception! {:08X}", instr);
    case 0x08:
      switch(mask_branch) {
        case 0:
          code.mov(regArg2, !regs.cop1.fcr31.compare);
          code.mov(rax, (u64)b);
          cpu.emitCall(rax);
          break;
        case 1:
          code.mov(regArg2, regs.cop1.fcr31.compare);
          code.mov(rax, (u64)b);
          cpu.emitCall(rax);
          break;
        case 2:
          code.mov(regArg2, !regs.cop1.fcr31.compare);
          code.mov(rax, (u64)bl);
          cpu.emitCall(rax);
          break;
        case 3:
          code.mov(regArg2, regs.cop1.fcr31.compare);
          code.mov(rax, (u64)bl);
          cpu.emitCall(rax);
          break;
        default: Util::panic("Undefined BC COP1 {:02X}", mask_branch);
      }
      break;
    case 0x10: // s
      switch(mask_fun) {
        case 0x00:
          code.mov(rax, (u64)adds);
          cpu.emitCall(rax);
          break;
        case 0x01:
          code.mov(rax, (u64)subs);
          cpu.emitCall(rax);
          break;
        case 0x02:
          code.mov(rax, (u64)muls);
          cpu.emitCall(rax);
          break;
        case 0x03:
          code.mov(rax, (u64)divs);
          cpu.emitCall(rax);
          break;
        case 0x04:
          code.mov(rax, (u64)sqrts);
          cpu.emitCall(rax);
          break;
        case 0x05:
          code.mov(rax, (u64)abss);
          cpu.emitCall(rax);
          break;
        case 0x06:
          code.mov(rax, (u64)movs);
          cpu.emitCall(rax);
          break;
        case 0x07:
          code.mov(rax, (u64)negs);
          cpu.emitCall(rax);
          break;
        case 0x08:
          code.mov(rax, (u64)roundls);
          cpu.emitCall(rax);
          break;
        case 0x09:
          code.mov(rax, (u64)truncls);
          cpu.emitCall(rax);
          break;
        case 0x0A:
          code.mov(rax, (u64)ceills);
          cpu.emitCall(rax);
          break;
        case 0x0B:
          code.mov(rax, (u64)floorls);
          cpu.emitCall(rax);
          break;
        case 0x0C:
          code.mov(rax, (u64)roundws);
          cpu.emitCall(rax);
          break;
        case 0x0D:
          code.mov(rax, (u64)truncws);
          cpu.emitCall(rax);
          break;
        case 0x0E:
          code.mov(rax, (u64)ceilws);
          cpu.emitCall(rax);
          break;
        case 0x0F:
          code.mov(rax, (u64)floorws);
          cpu.emitCall(rax);
          break;
        case 0x20:
          Util::panic("[RECOMPILER] FPU Reserved instruction exception! {:08X}", instr);
        case 0x21:
          code.mov(rax, (u64)cvtds);
          cpu.emitCall(rax);
          break;
        case 0x24:
          code.mov(rax, (u64)cvtws);
          cpu.emitCall(rax);
          break;
        case 0x25:
          code.mov(rax, (u64)cvtls);
          cpu.emitCall(rax);
          break;
        case 0x30:
          code.mov(rax, (u64)ccond<float>);
          code.mov(regArg2, F);
          cpu.emitCall(rax);
          break;
        case 0x31:
          code.mov(rax, (u64)ccond<float>);
          code.mov(regArg2, UN);
          cpu.emitCall(rax);
          break;
        case 0x32:
          code.mov(rax, (u64)ccond<float>);
          code.mov(regArg2, EQ);
          cpu.emitCall(rax);
          break;
        case 0x33:
          code.mov(rax, (u64)ccond<float>);
          code.mov(regArg2, UEQ);
          cpu.emitCall(rax);
          break;
        case 0x34:
          code.mov(rax, (u64)ccond<float>);
          code.mov(regArg2, OLT);
          cpu.emitCall(rax);
          break;
        case 0x35:
          code.mov(rax, (u64)ccond<float>);
          code.mov(regArg2, ULT);
          cpu.emitCall(rax);
          break;
        case 0x36:
          code.mov(rax, (u64)ccond<float>);
          code.mov(regArg2, OLE);
          cpu.emitCall(rax);
          break;
        case 0x37:
          code.mov(rax, (u64)ccond<float>);
          code.mov(regArg2, ULE);
          cpu.emitCall(rax);
          break;
        case 0x38:
          code.mov(rax, (u64)ccond<float>);
          code.mov(regArg2, SF);
          cpu.emitCall(rax);
          break;
        case 0x39:
          code.mov(rax, (u64)ccond<float>);
          code.mov(regArg2, NGLE);
          cpu.emitCall(rax);
          break;
        case 0x3A:
          code.mov(rax, (u64)ccond<float>);
          code.mov(regArg2, SEQ);
          cpu.emitCall(rax);
          break;
        case 0x3B:
          code.mov(rax, (u64)ccond<float>);
          code.mov(regArg2, NGL);
          cpu.emitCall(rax);
          break;
        case 0x3C:
          code.mov(rax, (u64)ccond<float>);
          code.mov(regArg2, LT);
          cpu.emitCall(rax);
          break;
        case 0x3D:
          code.mov(rax, (u64)ccond<float>);
          code.mov(regArg2, NGE);
          cpu.emitCall(rax);
          break;
        case 0x3E:
          code.mov(rax, (u64)ccond<float>);
          code.mov(regArg2, LE);
          cpu.emitCall(rax);
          break;
        case 0x3F:
          code.mov(rax, (u64)ccond<float>);
          code.mov(regArg2, NGT);
          cpu.emitCall(rax);
          break;
        default: Util::panic("Unimplemented COP1 function S[{} {}] ({:08X}) ({:016X})", mask_fun >> 3, mask_fun & 7, instr, (u64)regs.oldPC);
      }
      break;
    case 0x11: // d
      switch(mask_fun) {
        case 0x00:
          code.mov(rax, (u64)addd);
          cpu.emitCall(rax);
          break;
        case 0x01:
          code.mov(rax, (u64)subd);
          cpu.emitCall(rax);
          break;
        case 0x02:
          code.mov(rax, (u64)muld);
          cpu.emitCall(rax);
          break;
        case 0x03:
          code.mov(rax, (u64)divd);
          cpu.emitCall(rax);
          break;
        case 0x04:
          code.mov(rax, (u64)sqrtd);
          cpu.emitCall(rax);
          break;
        case 0x05:
          code.mov(rax, (u64)absd);
          cpu.emitCall(rax);
          break;
        case 0x06:
          code.mov(rax, (u64)movd);
          cpu.emitCall(rax);
          break;
        case 0x07:
          code.mov(rax, (u64)negd);
          cpu.emitCall(rax);
          break;
        case 0x08:
          code.mov(rax, (u64)roundld);
          cpu.emitCall(rax);
          break;
        case 0x09:
          code.mov(rax, (u64)truncld);
          cpu.emitCall(rax);
          break;
        case 0x0A:
          code.mov(rax, (u64)ceilld);
          cpu.emitCall(rax);
          break;
        case 0x0B:
          code.mov(rax, (u64)floorld);
          cpu.emitCall(rax);
          break;
        case 0x0C:
          code.mov(rax, (u64)roundwd);
          cpu.emitCall(rax);
          break;
        case 0x0D:
          code.mov(rax, (u64)truncwd);
          cpu.emitCall(rax);
          break;
        case 0x0E:
          code.mov(rax, (u64)ceilwd);
          cpu.emitCall(rax);
          break;
        case 0x0F:
          code.mov(rax, (u64)floorwd);
          cpu.emitCall(rax);
          break;
        case 0x20:
          code.mov(rax, (u64)cvtsd);
          cpu.emitCall(rax);
          break;
        case 0x21:
          Util::panic("[RECOMPILER] FPU Reserved instruction exception! {:08X}", instr);
        case 0x24:
          code.mov(rax, (u64)cvtwd);
          cpu.emitCall(rax);
          break;
        case 0x25:
          code.mov(rax, (u64)cvtld);
          cpu.emitCall(rax);
          break;
        case 0x30:
          code.mov(rax, (u64)ccond<double>);
          code.mov(regArg2, F);
          cpu.emitCall(rax);
          break;
        case 0x31:
          code.mov(rax, (u64)ccond<double>);
          code.mov(regArg2, UN);
          cpu.emitCall(rax);
          break;
        case 0x32:
          code.mov(rax, (u64)ccond<double>);
          code.mov(regArg2, EQ);
          cpu.emitCall(rax);
          break;
        case 0x33:
          code.mov(rax, (u64)ccond<double>);
          code.mov(regArg2, UEQ);
          cpu.emitCall(rax);
          break;
        case 0x34:
          code.mov(rax, (u64)ccond<double>);
          code.mov(regArg2, OLT);
          cpu.emitCall(rax);
          break;
        case 0x35:
          code.mov(rax, (u64)ccond<double>);
          code.mov(regArg2, ULT);
          cpu.emitCall(rax);
          break;
        case 0x36:
          code.mov(rax, (u64)ccond<double>);
          code.mov(regArg2, OLE);
          cpu.emitCall(rax);
          break;
        case 0x37:
          code.mov(rax, (u64)ccond<double>);
          code.mov(regArg2, ULE);
          cpu.emitCall(rax);
          break;
        case 0x38:
          code.mov(rax, (u64)ccond<double>);
          code.mov(regArg2, SF);
          cpu.emitCall(rax);
          break;
        case 0x39:
          code.mov(rax, (u64)ccond<double>);
          code.mov(regArg2, NGLE);
          cpu.emitCall(rax);
          break;
        case 0x3A:
          code.mov(rax, (u64)ccond<double>);
          code.mov(regArg2, SEQ);
          cpu.emitCall(rax);
          break;
        case 0x3B:
          code.mov(rax, (u64)ccond<double>);
          code.mov(regArg2, NGL);
          cpu.emitCall(rax);
          break;
        case 0x3C:
          code.mov(rax, (u64)ccond<double>);
          code.mov(regArg2, LT);
          cpu.emitCall(rax);
          break;
        case 0x3D:
          code.mov(rax, (u64)ccond<double>);
          code.mov(regArg2, NGE);
          cpu.emitCall(rax);
          break;
        case 0x3E:
          code.mov(rax, (u64)ccond<double>);
          code.mov(regArg2, LE);
          cpu.emitCall(rax);
          break;
        case 0x3F:
          code.mov(rax, (u64)ccond<double>);
          code.mov(regArg2, NGT);
          cpu.emitCall(rax);
          break;
        default: Util::panic("Unimplemented COP1 function D[{} {}] ({:08X}) ({:016X})", mask_fun >> 3, mask_fun & 7, instr, (u64)regs.oldPC);
      }
      break;
    case 0x14: // w
      switch(mask_fun) {
        case 0x01:
          code.mov(rax, (u64)subw);
          cpu.emitCall(rax);
          break;
        case 0x05:
          code.mov(rax, (u64)absw);
          cpu.emitCall(rax);
          break;
        case 0x02:
          code.mov(rax, (u64)mulw);
          cpu.emitCall(rax);
          break;
        case 0x06:
          code.mov(rax, (u64)movw);
          cpu.emitCall(rax);
          break;
        case 0x20:
          code.mov(rax, (u64)cvtsw);
          cpu.emitCall(rax);
          break;
        case 0x21:
          code.mov(rax, (u64)cvtdw);
          cpu.emitCall(rax);
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
          cpu.emitCall(rax);
          break;
        case 0x05:
          code.mov(rax, (u64)absl);
          cpu.emitCall(rax);
          break;
        case 0x02:
          code.mov(rax, (u64)mull);
          cpu.emitCall(rax);
          break;
        case 0x06:
          code.mov(rax, (u64)movl);
          cpu.emitCall(rax);
          break;
        case 0x20:
          code.mov(rax, (u64)cvtsl);
          cpu.emitCall(rax);
          break;
        case 0x21:
          code.mov(rax, (u64)cvtdl);
          cpu.emitCall(rax);
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