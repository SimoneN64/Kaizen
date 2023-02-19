#include <dynarec/cop/cop1decode.hpp>
#include <dynarec/instructions.hpp>
#include <Registers.hpp>

namespace n64 {
bool cop1Decode(Registers& regs, Dynarec& cpu, u32 instr) {
  Xbyak::CodeGenerator& code = cpu.code;

  u8 mask_sub = (instr >> 21) & 0x1F;
  u8 mask_fun = instr & 0x3F;
  u8 mask_branch = (instr >> 16) & 0x1F;

  code.mov(code.rdi, (u64)&regs);
  code.mov(code.esi, instr);

  switch(mask_sub) {
    // 000r_rccc
    case 0x00:
      code.mov(code.rax, (u64)mfc1);
      code.call(code.rax);
      break;
    case 0x01:
      code.mov(code.rax, (u64)dmfc1);
      code.call(code.rax);
      break;
    case 0x02:
      code.mov(code.rax, (u64)cfc1);
      code.call(code.rax);
      break;
    case 0x03:
      Util::panic("[RECOMPILER] FPU Reserved instruction exception!\n");
    case 0x04:
      code.mov(code.rax, (u64)mtc1);
      code.call(code.rax);
      break;
    case 0x05:
      code.mov(code.rax, (u64)dmtc1);
      code.call(code.rax);
      break;
    case 0x06:
      code.mov(code.rax, (u64)ctc1);
      code.call(code.rax);
      break;
    case 0x07:
      Util::panic("[RECOMPILER] FPU Reserved instruction exception!\n");
    case 0x08:
      switch(mask_branch) {
        case 0:
          code.mov(code.rdi, !regs.cop1.fcr31.compare);
          code.mov(code.rax, (u64)b);
          code.call(code.rax);
          return true;
        case 1:
          code.mov(code.rdi, regs.cop1.fcr31.compare);
          code.mov(code.rax, (u64)b);
          code.call(code.rax);
          return true;
        case 2:
          code.mov(code.rdi, !regs.cop1.fcr31.compare);
          code.mov(code.rax, (u64)bl);
          code.call(code.rax);
          return true;
        case 3:
          code.mov(code.rdi, regs.cop1.fcr31.compare);
          code.mov(code.rax, (u64)bl);
          code.call(code.rax);
          return true;
        default: Util::panic("Undefined BC COP1 {:02X}\n", mask_branch);
      }
      break;
    case 0x10: // s
      switch(mask_fun) {
        case 0x00:
          code.mov(code.rax, (u64)adds);
          code.call(code.rax);
          break;
        case 0x01:
          code.mov(code.rax, (u64)subs);
          code.call(code.rax);
          break;
        case 0x02:
          code.mov(code.rax, (u64)muls);
          code.call(code.rax);
          break;
        case 0x03:
          code.mov(code.rax, (u64)divs);
          code.call(code.rax);
          break;
        case 0x04:
          code.mov(code.rax, (u64)sqrts);
          code.call(code.rax);
          break;
        case 0x05:
          code.mov(code.rax, (u64)abss);
          code.call(code.rax);
          break;
        case 0x06:
          code.mov(code.rax, (u64)movs);
          code.call(code.rax);
          break;
        case 0x07:
          code.mov(code.rax, (u64)negs);
          code.call(code.rax);
          break;
        case 0x08:
          code.mov(code.rax, (u64)roundls);
          code.call(code.rax);
          break;
        case 0x09:
          code.mov(code.rax, (u64)truncls);
          code.call(code.rax);
          break;
        case 0x0A:
          code.mov(code.rax, (u64)ceills);
          code.call(code.rax);
          break;
        case 0x0B:
          code.mov(code.rax, (u64)floorls);
          code.call(code.rax);
          break;
        case 0x0C:
          code.mov(code.rax, (u64)roundws);
          code.call(code.rax);
          break;
        case 0x0D:
          code.mov(code.rax, (u64)truncws);
          code.call(code.rax);
          break;
        case 0x0E:
          code.mov(code.rax, (u64)ceilws);
          code.call(code.rax);
          break;
        case 0x0F:
          code.mov(code.rax, (u64)floorws);
          code.call(code.rax);
          break;
        case 0x20:
          Util::panic("[RECOMPILER] FPU Reserved instruction exception!\n");
        case 0x21:
          code.mov(code.rax, (u64)cvtds);
          code.call(code.rax);
          break;
        case 0x24:
          code.mov(code.rax, (u64)cvtws);
          code.call(code.rax);
          break;
        case 0x25:
          code.mov(code.rax, (u64)cvtls);
          code.call(code.rax);
          break;
        case 0x30:
          code.mov(code.rax, (u64)ccond<float>);
          code.mov(code.rdx, F);
          code.call(code.rax);
          break;
        case 0x31:
          code.mov(code.rax, (u64)ccond<float>);
          code.mov(code.rdx, UN);
          code.call(code.rax);
          break;
        case 0x32:
          code.mov(code.rax, (u64)ccond<float>);
          code.mov(code.rdx, EQ);
          code.call(code.rax);
          break;
        case 0x33:
          code.mov(code.rax, (u64)ccond<float>);
          code.mov(code.rdx, UEQ);
          code.call(code.rax);
          break;
        case 0x34:
          code.mov(code.rax, (u64)ccond<float>);
          code.mov(code.rdx, OLT);
          code.call(code.rax);
          break;
        case 0x35:
          code.mov(code.rax, (u64)ccond<float>);
          code.mov(code.rdx, ULT);
          code.call(code.rax);
          break;
        case 0x36:
          code.mov(code.rax, (u64)ccond<float>);
          code.mov(code.rdx, OLE);
          code.call(code.rax);
          break;
        case 0x37:
          code.mov(code.rax, (u64)ccond<float>);
          code.mov(code.rdx, ULE);
          code.call(code.rax);
          break;
        case 0x38:
          code.mov(code.rax, (u64)ccond<float>);
          code.mov(code.rdx, SF);
          code.call(code.rax);
          break;
        case 0x39:
          code.mov(code.rax, (u64)ccond<float>);
          code.mov(code.rdx, NGLE);
          code.call(code.rax);
          break;
        case 0x3A:
          code.mov(code.rax, (u64)ccond<float>);
          code.mov(code.rdx, SEQ);
          code.call(code.rax);
          break;
        case 0x3B:
          code.mov(code.rax, (u64)ccond<float>);
          code.mov(code.rdx, NGL);
          code.call(code.rax);
          break;
        case 0x3C:
          code.mov(code.rax, (u64)ccond<float>);
          code.mov(code.rdx, LT);
          code.call(code.rax);
          break;
        case 0x3D:
          code.mov(code.rax, (u64)ccond<float>);
          code.mov(code.rdx, NGE);
          code.call(code.rax);
          break;
        case 0x3E:
          code.mov(code.rax, (u64)ccond<float>);
          code.mov(code.rdx, LE);
          code.call(code.rax);
          break;
        case 0x3F:
          code.mov(code.rax, (u64)ccond<float>);
          code.mov(code.rdx, NGT);
          code.call(code.rax);
          break;
        default: Util::panic("Unimplemented COP1 function S[{} {}] ({:08X}) ({:016X})", mask_fun >> 3, mask_fun & 7, instr, (u64)regs.oldPC);
      }
      break;
    case 0x11: // d
      switch(mask_fun) {
        case 0x00:
          code.mov(code.rax, (u64)addd);
          code.call(code.rax);
          break;
        case 0x01:
          code.mov(code.rax, (u64)subd);
          code.call(code.rax);
          break;
        case 0x02:
          code.mov(code.rax, (u64)muld);
          code.call(code.rax);
          break;
        case 0x03:
          code.mov(code.rax, (u64)divd);
          code.call(code.rax);
          break;
        case 0x04:
          code.mov(code.rax, (u64)sqrtd);
          code.call(code.rax);
          break;
        case 0x05:
          code.mov(code.rax, (u64)absd);
          code.call(code.rax);
          break;
        case 0x06:
          code.mov(code.rax, (u64)movd);
          code.call(code.rax);
          break;
        case 0x07:
          code.mov(code.rax, (u64)negd);
          code.call(code.rax);
          break;
        case 0x08:
          code.mov(code.rax, (u64)roundld);
          code.call(code.rax);
          break;
        case 0x09:
          code.mov(code.rax, (u64)truncld);
          code.call(code.rax);
          break;
        case 0x0A:
          code.mov(code.rax, (u64)ceilld);
          code.call(code.rax);
          break;
        case 0x0B:
          code.mov(code.rax, (u64)floorld);
          code.call(code.rax);
          break;
        case 0x0C:
          code.mov(code.rax, (u64)roundwd);
          code.call(code.rax);
          break;
        case 0x0D:
          code.mov(code.rax, (u64)truncwd);
          code.call(code.rax);
          break;
        case 0x0E:
          code.mov(code.rax, (u64)ceilwd);
          code.call(code.rax);
          break;
        case 0x0F:
          code.mov(code.rax, (u64)floorwd);
          code.call(code.rax);
          break;
        case 0x20:
          code.mov(code.rax, (u64)cvtsd);
          code.call(code.rax);
          break;
        case 0x21:
          Util::panic("[RECOMPILER] FPU Reserved instruction exception!\n");
        case 0x24:
          code.mov(code.rax, (u64)cvtwd);
          code.call(code.rax);
          break;
        case 0x25:
          code.mov(code.rax, (u64)cvtld);
          code.call(code.rax);
          break;
        case 0x30:
          code.mov(code.rax, (u64)ccond<double>);
          code.mov(code.rdx, F);
          code.call(code.rax);
          break;
        case 0x31:
          code.mov(code.rax, (u64)ccond<double>);
          code.mov(code.rdx, UN);
          code.call(code.rax);
          break;
        case 0x32:
          code.mov(code.rax, (u64)ccond<double>);
          code.mov(code.rdx, EQ);
          code.call(code.rax);
          break;
        case 0x33:
          code.mov(code.rax, (u64)ccond<double>);
          code.mov(code.rdx, UEQ);
          code.call(code.rax);
          break;
        case 0x34:
          code.mov(code.rax, (u64)ccond<double>);
          code.mov(code.rdx, OLT);
          code.call(code.rax);
          break;
        case 0x35:
          code.mov(code.rax, (u64)ccond<double>);
          code.mov(code.rdx, ULT);
          code.call(code.rax);
          break;
        case 0x36:
          code.mov(code.rax, (u64)ccond<double>);
          code.mov(code.rdx, OLE);
          code.call(code.rax);
          break;
        case 0x37:
          code.mov(code.rax, (u64)ccond<double>);
          code.mov(code.rdx, ULE);
          code.call(code.rax);
          break;
        case 0x38:
          code.mov(code.rax, (u64)ccond<double>);
          code.mov(code.rdx, SF);
          code.call(code.rax);
          break;
        case 0x39:
          code.mov(code.rax, (u64)ccond<double>);
          code.mov(code.rdx, NGLE);
          code.call(code.rax);
          break;
        case 0x3A:
          code.mov(code.rax, (u64)ccond<double>);
          code.mov(code.rdx, SEQ);
          code.call(code.rax);
          break;
        case 0x3B:
          code.mov(code.rax, (u64)ccond<double>);
          code.mov(code.rdx, NGL);
          code.call(code.rax);
          break;
        case 0x3C:
          code.mov(code.rax, (u64)ccond<double>);
          code.mov(code.rdx, LT);
          code.call(code.rax);
          break;
        case 0x3D:
          code.mov(code.rax, (u64)ccond<double>);
          code.mov(code.rdx, NGE);
          code.call(code.rax);
          break;
        case 0x3E:
          code.mov(code.rax, (u64)ccond<double>);
          code.mov(code.rdx, LE);
          code.call(code.rax);
          break;
        case 0x3F:
          code.mov(code.rax, (u64)ccond<double>);
          code.mov(code.rdx, NGT);
          code.call(code.rax);
          break;
        default: Util::panic("Unimplemented COP1 function D[{} {}] ({:08X}) ({:016X})", mask_fun >> 3, mask_fun & 7, instr, (u64)regs.oldPC);
      }
      break;
    case 0x14: // w
      switch(mask_fun) {
        case 0x01:
          code.mov(code.rax, (u64)subw);
          code.call(code.rax);
          break;
        case 0x05:
          code.mov(code.rax, (u64)absw);
          code.call(code.rax);
          break;
        case 0x02:
          code.mov(code.rax, (u64)mulw);
          code.call(code.rax);
          break;
        case 0x06:
          code.mov(code.rax, (u64)movw);
          code.call(code.rax);
          break;
        case 0x20:
          code.mov(code.rax, (u64)cvtsw);
          code.call(code.rax);
          break;
        case 0x21:
          code.mov(code.rax, (u64)cvtdw);
          code.call(code.rax);
          break;
        case 0x24:
          Util::panic("[RECOMPILER] FPU reserved instruction exception!\n");
        default: Util::panic("Unimplemented COP1 function W[{} {}] ({:08X}) ({:016X})", mask_fun >> 3, mask_fun & 7, instr, (u64)regs.oldPC);
      }
      break;
    case 0x15: // l
      switch(mask_fun) {
        case 0x01:
          code.mov(code.rax, (u64)subl);
          code.call(code.rax);
          break;
        case 0x05:
          code.mov(code.rax, (u64)absl);
          code.call(code.rax);
          break;
        case 0x02:
          code.mov(code.rax, (u64)mull);
          code.call(code.rax);
          break;
        case 0x06:
          code.mov(code.rax, (u64)movl);
          code.call(code.rax);
          break;
        case 0x20:
          code.mov(code.rax, (u64)cvtsl);
          code.call(code.rax);
          break;
        case 0x21:
          code.mov(code.rax, (u64)cvtdl);
          code.call(code.rax);
          break;
        case 0x24:
          Util::panic("[RECOMPILER] FPU reserved instruction exception!\n");
        case 0x25:
          Util::panic("[RECOMPILER] FPU reserved instruction exception!\n");
        default: Util::panic("Unimplemented COP1 function L[{} {}] ({:08X}) ({:016X})", mask_fun >> 3, mask_fun & 7, instr, (u64)regs.oldPC);
      }
      break;
    default: Util::panic("Unimplemented COP1 instruction {} {}", mask_sub >> 3, mask_sub & 7);
  }

  return false;
}
}