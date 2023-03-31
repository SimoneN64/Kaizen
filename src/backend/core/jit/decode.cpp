#include <jit/instructions.hpp>
#include <jit/cop/cop1decode.hpp>
#include <jit/cop/cop0decode.hpp>
#include <Registers.hpp>

namespace n64 {
void JIT::cop2Decode(u32 instr) {
  switch(RS(instr)) {
    case 0x00:
      code.mov(rax, (u64)mfc2);
      code.call(rax);
      break;
    case 0x01:
      code.mov(rax, (u64)dmfc2);
      code.call(rax);
      break;
    case 0x02: case 0x06: break;
    case 0x04:
      code.mov(rax, (u64)mtc2);
      code.call(rax);
      break;
    case 0x05:
      code.mov(rax, (u64)dmtc2);
      code.call(rax);
      break;
    default:
      Util::panic("[RECOMPILER] Unhandled reserved instruction exception {:016X}", (u64)regs.pc);
  }
}

void JIT::special(u32 instr) {
  u8 mask = (instr & 0x3F);

  // 00rr_rccc
  switch (mask) { // TODO: named constants for clearer code
    case 0:
      if (instr != 0) {
        code.mov(rax, (u64)sll);
        code.call(rax);
      }
      break;
    case 0x02:
      code.mov(rax, (u64)srl);
      code.call(rax);
      break;
    case 0x03:
      code.mov(rax, (u64)sra);
      code.call(rax);
      break;
    case 0x04:
      code.mov(rax, (u64)sllv);
      code.call(rax);
      break;
    case 0x06:
      code.mov(rax, (u64)srlv);
      code.call(rax);
      break;
    case 0x07:
      code.mov(rax, (u64)srav);
      code.call(rax);
      break;
    case 0x08:
      code.mov(rax, (u64)jr);
      code.call(rax);
      break;
    case 0x09:
      code.mov(rax, (u64)jalr);
      code.call(rax);
      break;
    case 0x0C: Util::panic("[RECOMPILER] Unhandled syscall instruction {:016X}", (u64)regs.pc);
    case 0x0D: Util::panic("[RECOMPILER] Unhandled break instruction {:016X}", (u64)regs.pc);
    case 0x0F: break; // SYNC
    case 0x10:
      code.mov(rax, (u64)mfhi);
      code.call(rax);
      break;
    case 0x11:
      code.mov(rax, (u64)mthi);
      code.call(rax);
      break;
    case 0x12:
      code.mov(rax, (u64)mflo);
      code.call(rax);
      break;
    case 0x13:
      code.mov(rax, (u64)mtlo);
      code.call(rax);
      break;
    case 0x14:
      code.mov(rax, (u64)dsllv);
      code.call(rax);
      break;
    case 0x16:
      code.mov(rax, (u64)dsrlv);
      code.call(rax);
      break;
    case 0x17:
      code.mov(rax, (u64)dsrav);
      code.call(rax);
      break;
    case 0x18:
      code.mov(rax, (u64)mult);
      code.call(rax);
      break;
    case 0x19:
      code.mov(rax, (u64)multu);
      code.call(rax);
      break;
    case 0x1A:
      code.mov(rax, (u64)div);
      code.call(rax);
      break;
    case 0x1B:
      code.mov(rax, (u64)divu);
      code.call(rax);
      break;
    case 0x1C:
      code.mov(rax, (u64)dmult);
      code.call(rax);
      break;
    case 0x1D:
      code.mov(rax, (u64)dmultu);
      code.call(rax);
      break;
    case 0x1E:
      code.mov(rax, (u64)ddiv);
      code.call(rax);
      break;
    case 0x1F:
      code.mov(rax, (u64)ddivu);
      code.call(rax);
      break;
    case 0x20:
      code.mov(rax, (u64)add);
      code.call(rax);
      break;
    case 0x21:
      code.mov(rax, (u64)addu);
      code.call(rax);
      break;
    case 0x22:
      code.mov(rax, (u64)sub);
      code.call(rax);
      break;
    case 0x23:
      code.mov(rax, (u64)subu);
      code.call(rax);
      break;
    case 0x24:
      code.mov(rax, (u64)and_);
      code.call(rax);
      break;
    case 0x25:
      code.mov(rax, (u64)or_);
      code.call(rax);
      break;
    case 0x26:
      code.mov(rax, (u64)xor_);
      code.call(rax);
      break;
    case 0x27:
      code.mov(rax, (u64)nor);
      code.call(rax);
      break;
    case 0x2A:
      code.mov(rax, (u64)slt);
      code.call(rax);
      break;
    case 0x2B:
      code.mov(rax, (u64)sltu);
      code.call(rax);
      break;
    case 0x2C:
      code.mov(rax, (u64)dadd);
      code.call(rax);
      break;
    case 0x2D:
      code.mov(rax, (u64)daddu);
      code.call(rax);
      break;
    case 0x2E:
      code.mov(rax, (u64)dsub);
      code.call(rax);
      break;
    case 0x2F:
      code.mov(rax, (u64)dsubu);
      code.call(rax);
      break;
    case 0x30:
      code.mov(r8, qword[rdi + GPR_OFFSET(RS(instr), this)]);
      code.mov(rcx, qword[rdi + GPR_OFFSET(RT(instr), this)]);
      code.xor_(regArg1, regArg1);
      code.cmp(r8, rcx);
      code.setge(regArg1.cvt8());
      code.mov(rax, (u64)trap);
      code.call(rax);
      break;
    case 0x31:
      code.mov(r8, qword[rdi + GPR_OFFSET(RS(instr), this)]);
      code.mov(rcx, qword[rdi + GPR_OFFSET(RT(instr), this)]);
      code.xor_(regArg1, regArg1);
      code.cmp(r8, rcx);
      code.setae(regArg1.cvt8());
      code.mov(rax, (u64)trap);
      code.call(rax);
      break;
    case 0x32:
      code.mov(r8, qword[rdi + GPR_OFFSET(RS(instr), this)]);
      code.mov(rcx, qword[rdi + GPR_OFFSET(RT(instr), this)]);
      code.xor_(regArg1, regArg1);
      code.cmp(r8, rcx);
      code.setl(regArg1.cvt8());
      code.mov(rax, (u64)trap);
      code.call(rax);
      break;
    case 0x33:
      code.mov(r8, qword[rdi + GPR_OFFSET(RS(instr), this)]);
      code.mov(rcx, qword[rdi + GPR_OFFSET(RT(instr), this)]);
      code.xor_(regArg1, regArg1);
      code.cmp(r8, rcx);
      code.setb(regArg1.cvt8());
      code.mov(rax, (u64)trap);
      code.call(rax);
      break;
    case 0x34:
      code.mov(r8, qword[rdi + GPR_OFFSET(RS(instr), this)]);
      code.mov(rcx, qword[rdi + GPR_OFFSET(RT(instr), this)]);
      code.xor_(regArg1, regArg1);
      code.cmp(r8, rcx);
      code.sete(regArg1.cvt8());
      code.mov(rax, (u64)trap);
      code.call(rax);
      break;
    case 0x36:
      code.mov(r8, qword[rdi + GPR_OFFSET(RS(instr), this)]);
      code.mov(rcx, qword[rdi + GPR_OFFSET(RT(instr), this)]);
      code.xor_(regArg1, regArg1);
      code.cmp(r8, rcx);
      code.setne(regArg1.cvt8());
      code.mov(rax, (u64)trap);
      code.call(rax);
      break;
    case 0x38:
      code.mov(rax, (u64)dsll);
      code.call(rax);
      break;
    case 0x3A:
      code.mov(rax, (u64)dsrl);
      code.call(rax);
      break;
    case 0x3B:
      code.mov(rax, (u64)dsra);
      code.call(rax);
      break;
    case 0x3C:
      code.mov(rax, (u64)dsll32);
      code.call(rax);
      break;
    case 0x3E:
      code.mov(rax, (u64)dsrl32);
      code.call(rax);
      break;
    case 0x3F:
      code.mov(rax, (u64)dsra32);
      code.call(rax);
      break;
    default:
      Util::panic("Unimplemented special {} {} ({:08X}) (pc: {:016X})", (mask >> 3) & 7, mask & 7, instr, (u64)regs.oldPC);
  }
}

void JIT::regimm(u32 instr) {
  u8 mask = ((instr >> 16) & 0x1F);
  // 000r_rccc
  switch (mask) { // TODO: named constants for clearer code
    case 0x00:
      code.mov(r8, qword[rdi + GPR_OFFSET(RS(instr), this)]);
      code.xor_(regArg2, regArg2);
      code.cmp(r8, 0);
      code.setl(regArg2.cvt8());
      code.mov(rax, (u64)b);
      code.call(rax);
      break;
    case 0x01:
      code.mov(r8, qword[rdi + GPR_OFFSET(RS(instr), this)]);
      code.xor_(regArg2, regArg2);
      code.cmp(r8, 0);
      code.setge(regArg2.cvt8());
      code.mov(rax, (u64)b);
      code.call(rax);
      break;
    case 0x02:
      code.mov(r8, qword[rdi + GPR_OFFSET(RS(instr), this)]);
      code.xor_(regArg2, regArg2);
      code.cmp(r8, 0);
      code.setl(dl);
      code.mov(rax, (u64)bl);
      code.call(rax);
      break;
    case 0x03:
      code.mov(r8, qword[rdi + GPR_OFFSET(RS(instr), this)]);
      code.xor_(regArg2, regArg2);
      code.cmp(r8, 0);
      code.setge(regArg2.cvt8());
      code.mov(rax, (u64)bl);
      code.call(rax);
      break;
    case 0x08:
      code.mov(r8, qword[rdi + GPR_OFFSET(RS(instr), this)]);
      code.xor_(regArg1, regArg1);
      code.cmp(r8, s64(s16(instr)));
      code.setge(regArg1.cvt8());
      code.mov(rax, (u64)trap);
      code.call(rax);
      break;
    case 0x09:
      code.mov(r8, qword[rdi + GPR_OFFSET(RS(instr), this)]);
      code.xor_(regArg1, regArg1);
      code.cmp(r8, u64(s64(s16(instr))));
      code.setae(regArg1.cvt8());
      code.mov(rax, (u64)trap);
      code.call(rax);
      break;
    case 0x0A:
      code.mov(r8, qword[rdi + GPR_OFFSET(RS(instr), this)]);
      code.xor_(regArg1, regArg1);
      code.cmp(r8, s64(s16(instr)));
      code.setl(regArg1.cvt8());
      code.mov(rax, (u64)trap);
      code.call(rax);
      break;
    case 0x0B:
      code.mov(r8, qword[rdi + GPR_OFFSET(RS(instr), this)]);
      code.xor_(regArg1, regArg1);
      code.cmp(r8, u64(s64(s16(instr))));
      code.setb(regArg1.cvt8());
      code.mov(rax, (u64)trap);
      code.call(rax);
      break;
    case 0x0C:
      code.mov(r8, qword[rdi + GPR_OFFSET(RS(instr), this)]);
      code.xor_(regArg1, regArg1);
      code.cmp(r8, s64(s16(instr)));
      code.sete(regArg1.cvt8());
      code.mov(rax, (u64)trap);
      code.call(rax);
      break;
    case 0x0E:
      code.mov(r8, qword[rdi + GPR_OFFSET(RS(instr), this)]);
      code.xor_(regArg1, regArg1);
      code.cmp(r8, s64(s16(instr)));
      code.setne(regArg1.cvt8());
      code.mov(rax, (u64)trap);
      code.call(rax);
      break;
    case 0x10:
      code.mov(rcx, qword[rdi + GPR_OFFSET(RS(instr), this)]);
      code.xor_(regArg2, regArg2);
      code.cmp(rcx, 0);
      code.setl(regArg2.cvt8());
      code.mov(rax, (u64)blink);
      code.call(rax);
      break;
    case 0x11:
      code.mov(rcx, qword[rdi + GPR_OFFSET(RS(instr), this)]);
      code.xor_(regArg2, regArg2);
      code.cmp(rcx, 0);
      code.setge(regArg2.cvt8());
      code.mov(rax, (u64)blink);
      code.call(rax);
      break;
    case 0x12:
      code.mov(rcx, qword[rdi + GPR_OFFSET(RS(instr), this)]);
      code.xor_(regArg2, regArg2);
      code.cmp(rcx, 0);
      code.setl(regArg2.cvt8());
      code.mov(rax, (u64)bllink);
      code.call(rax);
      break;
    case 0x13:
      code.mov(rcx, qword[rdi + GPR_OFFSET(RS(instr), this)]);
      code.xor_(regArg2, regArg2);
      code.cmp(rcx, 0);
      code.setge(regArg2.cvt8());
      code.mov(rax, (u64)bllink);
      code.call(rax);
      break;
    default:
      Util::panic("Unimplemented regimm {} {} ({:08X}) (pc: {:016X})", (mask >> 3) & 3, mask & 7, instr, (u64)regs.oldPC);
  }
}

bool JIT::isEndBlock(u32 instr) {
  u8 mask = (instr >> 26) & 0x3f;

  // 00rr_rccc
  switch (mask) { // TODO: named constants for clearer code
    case 0x00: {
      u8 special_mask = (instr & 0x3F);

      // 00rr_rccc
      switch (special_mask) { // TODO: named constants for clearer code
        case 0:
        case 0x02: case 0x03: case 0x04: case 0x06:
        case 0x07:
          return false;
        case 0x08: case 0x09:
          return true;
        case 0x0C: Util::panic("[RECOMPILER] Unhandled syscall instruction {:016X}", (u64)regs.pc);
        case 0x0D: Util::panic("[RECOMPILER] Unhandled break instruction {:016X}", (u64)regs.pc);
        case 0x0F: break; // SYNC
        case 0x10: case 0x11: case 0x12: case 0x13:
        case 0x14: case 0x16: case 0x17: case 0x18:
        case 0x19: case 0x1A: case 0x1B: case 0x1C:
        case 0x1D: case 0x1E: case 0x1F: case 0x20:
        case 0x21: case 0x22: case 0x23: case 0x24:
        case 0x25: case 0x26: case 0x27: case 0x2A:
        case 0x2B: case 0x2C: case 0x2D: case 0x2E:
        case 0x2F:
          return false;
        case 0x30: case 0x31: case 0x32: case 0x33:
        case 0x34: case 0x36:
          return true;
        case 0x38: case 0x3A: case 0x3B: case 0x3C:
        case 0x3E: case 0x3F:
          return false;
        default:
          Util::panic("Unimplemented special {} {} ({:08X}) (pc: {:016X})", (mask >> 3) & 7, mask & 7, instr, (u64)regs.oldPC);
      }
    }
    break;
  case 0x01: case 0x02: case 0x03:
  case 0x04: case 0x05: case 0x06:
  case 0x07:
    return true;
  case 0x08: case 0x09: case 0x0A: case 0x0B:
  case 0x0C: case 0x0D: case 0x0E: case 0x0F:
    return false;
  case 0x10: 
    return cop0IsEndBlock(instr);
  case 0x11: 
    return cop1IsEndBlock(instr);
  case 0x18: case 0x19: case 0x1A: case 0x1B:
  case 0x12: 
    return false;
  case 0x14: case 0x15: case 0x16: case 0x17:
    return true;
  case 0x1F: Util::panic("[RECOMPILER] Unhandled reserved instruction exception {:016X}", regs.oldPC); break;
  case 0x20: case 0x21: case 0x22: case 0x23:
  case 0x24: case 0x25: case 0x26: case 0x27:
  case 0x28: case 0x29: case 0x2A: case 0x2B:
  case 0x2C: case 0x2D: case 0x2E: case 0x2F:
  case 0x30: case 0x31: case 0x34: case 0x35:
  case 0x37: case 0x38: case 0x39: case 0x3C:
  case 0x3D: case 0x3F:
    return false;
  default:
    Util::panic("Unimplemented instruction {:02X} ({:08X}) (pc: {:016X})", mask, instr, (u64)regs.oldPC);
  }

  return false;
}

void JIT::Emit(Mem& mem, u32 instr) {
  u8 mask = (instr >> 26) & 0x3f;

  // 00rr_rccc
  switch(mask) { // TODO: named constants for clearer code
    case 0x00: special(instr); break;
    case 0x01: regimm(instr); break;
    case 0x02:
      code.mov(rax, (u64)j);
      code.call(rax);
      break;
    case 0x03:
      code.mov(rax, (u64)jal);
      code.call(rax);
      break;
    case 0x04:
      code.mov(r8, qword[rdi + GPR_OFFSET(RS(instr), this)]);
      code.mov(r9, qword[rdi + GPR_OFFSET(RT(instr), this)]);
      code.xor_(regArg2, regArg2);
      code.cmp(r8, r9);
      code.sete(regArg2.cvt8());
      code.mov(rax, (u64)b);
      code.call(rax);
      break;
    case 0x05:
      code.mov(r8, qword[rdi + GPR_OFFSET(RS(instr), this)]);
      code.mov(r9, qword[rdi + GPR_OFFSET(RT(instr), this)]);
      code.xor_(regArg2, regArg2);
      code.cmp(r8, r9);
      code.setne(regArg2.cvt8());
      code.mov(rax, (u64)b);
      code.call(rax);
      break;
    case 0x06:
      code.mov(r8, qword[rdi + GPR_OFFSET(RS(instr), this)]);
      code.xor_(regArg2, regArg2);
      code.test(r8, r8);
      code.setnz(regArg2.cvt8());
      code.mov(rax, (u64)b);
      code.call(rax);
      break;
    case 0x07:
      code.mov(r8, qword[rdi + GPR_OFFSET(RS(instr), this)]);
      code.xor_(regArg2, regArg2);
      code.test(r8, r8);
      code.setg(regArg2.cvt8());
      code.mov(rax, (u64)b);
      code.call(rax);
      break;
    case 0x08:
      code.mov(rax, (u64)addi);
      code.call(rax);
      break;
    case 0x09:
      code.mov(rax, (u64)addiu);
      code.call(rax);
      break;
    case 0x0A:
      code.mov(rax, (u64)slti);
      code.call(rax);
      break;
    case 0x0B:
      code.mov(rax, (u64)sltiu);
      code.call(rax);
      break;
    case 0x0C:
      code.mov(rax, (u64)andi);
      code.call(rax);
      break;
    case 0x0D:
      code.mov(rax, (u64)ori);
      code.call(rax);
      break;
    case 0x0E:
      code.mov(rax, (u64)xori);
      code.call(rax);
      break;
    case 0x0F:
      code.mov(rax, (u64)lui);
      code.call(rax);
      break;
    case 0x10: cop0Emit(*this, instr); break;
    case 0x11: cop1Emit(*this, instr); break;
    case 0x12: cop2Decode(instr); break;
    case 0x14:
      code.mov(r8, qword[rdi + GPR_OFFSET(RS(instr), this)]);
      code.mov(rcx, qword[rdi + GPR_OFFSET(RT(instr), this)]);
      code.xor_(regArg2, regArg2);
      code.cmp(r8, rcx);
      code.sete(regArg2.cvt8());
      code.mov(rax, (u64)bl);
      code.call(rax);
      break;
    case 0x15:
      code.mov(r8, qword[rdi + GPR_OFFSET(RS(instr), this)]);
      code.mov(rcx, qword[rdi + GPR_OFFSET(RT(instr), this)]);
      code.xor_(regArg2, regArg2);
      code.cmp(r8, rcx);
      code.setne(regArg2.cvt8());
      code.mov(rax, (u64)bl);
      code.call(rax);
      break;
    case 0x16:
      code.mov(r8, qword[rdi + GPR_OFFSET(RS(instr), this)]);
      code.xor_(regArg2, regArg2);
      code.cmp(r8, 0);
      code.setle(regArg2.cvt8());
      code.mov(rax, (u64)bl);
      code.call(rax);
      break;
    case 0x17:
      code.mov(r8, qword[rdi + GPR_OFFSET(RS(instr), this)]);
      code.xor_(regArg2, regArg2);
      code.cmp(r8, 0);
      code.setg(regArg2.cvt8());
      code.mov(rax, (u64)b);
      code.call(rax);
      break;
    case 0x18:
      code.mov(rax, (u64)daddi);
      code.call(rax);
      break;
    case 0x19:
      code.mov(rax, (u64)daddiu);
      code.call(rax);
      break;
    case 0x1A:
      code.mov(rax, (u64)ldl);
      code.call(rax);
      break;
    case 0x1B:
      code.mov(rax, (u64)ldr);
      code.call(rax);
      break;
    case 0x1F: Util::panic("[RECOMPILER] Unhandled reserved instruction exception {:016X}", regs.oldPC); break;
    case 0x20:
      code.mov(rax, (u64)lb);
      code.call(rax);
      break;
    case 0x21:
      code.mov(rax, (u64)lh);
      code.call(rax);
      break;
    case 0x22:
      code.mov(rax, (u64)lwl);
      code.call(rax);
      break;
    case 0x23:
      code.mov(rax, (u64)lw);
      code.call(rax);
      break;
    case 0x24:
      code.mov(rax, (u64)lbu);
      code.call(rax);
      break;
    case 0x25:
      code.mov(rax, (u64)lhu);
      code.call(rax);
      break;
    case 0x26:
      code.mov(rax, (u64)lwr);
      code.call(rax);
      break;
    case 0x27:
      code.mov(rax, (u64)lwu);
      code.call(rax);
      break;
    case 0x28:
      code.mov(rax, (u64)sb);
      code.call(rax);
      break;
    case 0x29:
      code.mov(rax, (u64)sh);
      code.call(rax);
      break;
    case 0x2A:
      code.mov(rax, (u64)swl);
      code.call(rax);
      break;
    case 0x2B:
      code.mov(rax, (u64)sw);
      code.call(rax);
      break;
    case 0x2C:
      code.mov(rax, (u64)sdl);
      code.call(rax);
      break;
    case 0x2D:
      code.mov(rax, (u64)sdr);
      code.call(rax);
      break;
    case 0x2E:
      code.mov(rax, (u64)swr);
      code.call(rax);
      break;
    case 0x2F: break; // CACHE
    case 0x30:
      code.mov(rax, (u64)ll);
      code.call(rax);
      break;
    case 0x31:
      code.mov(rax, (u64)lwc1);
      code.call(rax);
      break;
    case 0x34:
      code.mov(rax, (u64)lld);
      code.call(rax);
      break;
    case 0x35:
      code.mov(rax, (u64)ldc1);
      code.call(rax);
      break;
    case 0x37:
      code.mov(rax, (u64)ld);
      code.call(rax);
      break;
    case 0x38:
      code.mov(rax, (u64)sc);
      code.call(rax);
      break;
    case 0x39:
      code.mov(rax, (u64)swc1);
      code.call(rax);
      break;
    case 0x3C:
      code.mov(rax, (u64)scd);
      code.call(rax);
      break;
    case 0x3D:
      code.mov(rax, (u64)sdc1);
      code.call(rax);
      break;
    case 0x3F:
      code.mov(rax, (u64)sd);
      code.call(rax);
      break;
    default:
      Util::panic("Unimplemented instruction {:02X} ({:08X}) (pc: {:016X})", mask, instr, (u64)regs.oldPC);
  }
}
}
