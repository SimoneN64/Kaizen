#include <jit/instructions.hpp>
#include <jit/cop/cop1decode.hpp>
#include <jit/cop/cop0decode.hpp>
#include <Registers.hpp>

namespace n64 {
void JIT::cop2Decode(u32 instr) {
  switch(RS(instr)) {
    case 0x00:
      code->mov(rax, (u64)mfc2);
      emitCall(rax);
      break;
    case 0x01:
      code->mov(rax, (u64)dmfc2);
      emitCall(rax);
      break;
    case 0x02: case 0x06: break;
    case 0x04:
      code->mov(rax, (u64)mtc2);
      emitCall(rax);
      break;
    case 0x05:
      code->mov(rax, (u64)dmtc2);
      emitCall(rax);
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
        code->mov(rax, (u64)sll);
        emitCall(rax);
      }
      break;
    case 0x02:
      code->mov(rax, (u64)srl);
      emitCall(rax);
      break;
    case 0x03:
      code->mov(rax, (u64)sra);
      emitCall(rax);
      break;
    case 0x04:
      code->mov(rax, (u64)sllv);
      emitCall(rax);
      break;
    case 0x06:
      code->mov(rax, (u64)srlv);
      emitCall(rax);
      break;
    case 0x07:
      code->mov(rax, (u64)srav);
      emitCall(rax);
      break;
    case 0x08:
      code->mov(rax, (u64)jr);
      emitCall(rax);
      break;
    case 0x09:
      code->mov(rax, (u64)jalr);
      emitCall(rax);
      break;
    case 0x0C: Util::panic("[RECOMPILER] Unhandled syscall instruction {:016X}", (u64)regs.pc);
    case 0x0D: Util::panic("[RECOMPILER] Unhandled break instruction {:016X}", (u64)regs.pc);
    case 0x0F: break; // SYNC
    case 0x10:
      code->mov(rax, (u64)mfhi);
      emitCall(rax);
      break;
    case 0x11:
      code->mov(rax, (u64)mthi);
      emitCall(rax);
      break;
    case 0x12:
      code->mov(rax, (u64)mflo);
      emitCall(rax);
      break;
    case 0x13:
      code->mov(rax, (u64)mtlo);
      emitCall(rax);
      break;
    case 0x14:
      code->mov(rax, (u64)dsllv);
      emitCall(rax);
      break;
    case 0x16:
      code->mov(rax, (u64)dsrlv);
      emitCall(rax);
      break;
    case 0x17:
      code->mov(rax, (u64)dsrav);
      emitCall(rax);
      break;
    case 0x18:
      code->mov(rax, (u64)mult);
      emitCall(rax);
      break;
    case 0x19:
      code->mov(rax, (u64)multu);
      emitCall(rax);
      break;
    case 0x1A:
      code->mov(rax, (u64)div);
      emitCall(rax);
      break;
    case 0x1B:
      code->mov(rax, (u64)divu);
      emitCall(rax);
      break;
    case 0x1C:
      code->mov(rax, (u64)dmult);
      emitCall(rax);
      break;
    case 0x1D:
      code->mov(rax, (u64)dmultu);
      emitCall(rax);
      break;
    case 0x1E:
      code->mov(rax, (u64)ddiv);
      emitCall(rax);
      break;
    case 0x1F:
      code->mov(rax, (u64)ddivu);
      emitCall(rax);
      break;
    case 0x20:
      code->mov(rax, (u64)add);
      emitCall(rax);
      break;
    case 0x21:
      code->mov(rax, (u64)addu);
      emitCall(rax);
      break;
    case 0x22:
      code->mov(rax, (u64)sub);
      emitCall(rax);
      break;
    case 0x23:
      code->mov(rax, (u64)subu);
      emitCall(rax);
      break;
    case 0x24:
      code->mov(rax, (u64)and_);
      emitCall(rax);
      break;
    case 0x25:
      code->mov(rax, (u64)or_);
      emitCall(rax);
      break;
    case 0x26:
      code->mov(rax, (u64)xor_);
      emitCall(rax);
      break;
    case 0x27:
      code->mov(rax, (u64)nor);
      emitCall(rax);
      break;
    case 0x2A:
      code->mov(rax, (u64)slt);
      emitCall(rax);
      break;
    case 0x2B:
      code->mov(rax, (u64)sltu);
      emitCall(rax);
      break;
    case 0x2C:
      code->mov(rax, (u64)dadd);
      emitCall(rax);
      break;
    case 0x2D:
      code->mov(rax, (u64)daddu);
      emitCall(rax);
      break;
    case 0x2E:
      code->mov(rax, (u64)dsub);
      emitCall(rax);
      break;
    case 0x2F:
      code->mov(rax, (u64)dsubu);
      emitCall(rax);
      break;
    case 0x30:
      code->mov(regScratch0, qword[JIT_THIS + GPR_OFFSET(RS(instr), this)]);
      code->mov(regScratch1, qword[JIT_THIS + GPR_OFFSET(RT(instr), this)]);
      code->xor_(regArg1, regArg1);
      code->cmp(regScratch0, regScratch1);
      code->setge(regArg1.cvt8());
      code->mov(rax, (u64)trap);
      emitCall(rax);
      break;
    case 0x31:
      code->mov(regScratch0, qword[JIT_THIS + GPR_OFFSET(RS(instr), this)]);
      code->mov(regScratch1, qword[JIT_THIS + GPR_OFFSET(RT(instr), this)]);
      code->xor_(regArg1, regArg1);
      code->cmp(regScratch0, regScratch1);
      code->setae(regArg1.cvt8());
      code->mov(rax, (u64)trap);
      emitCall(rax);
      break;
    case 0x32:
      code->mov(regScratch0, qword[JIT_THIS + GPR_OFFSET(RS(instr), this)]);
      code->mov(regScratch1, qword[JIT_THIS + GPR_OFFSET(RT(instr), this)]);
      code->xor_(regArg1, regArg1);
      code->cmp(regScratch0, regScratch1);
      code->setl(regArg1.cvt8());
      code->mov(rax, (u64)trap);
      emitCall(rax);
      break;
    case 0x33:
      code->mov(regScratch0, qword[JIT_THIS + GPR_OFFSET(RS(instr), this)]);
      code->mov(regScratch1, qword[JIT_THIS + GPR_OFFSET(RT(instr), this)]);
      code->xor_(regArg1, regArg1);
      code->cmp(regScratch0, regScratch1);
      code->setb(regArg1.cvt8());
      code->mov(rax, (u64)trap);
      emitCall(rax);
      break;
    case 0x34:
      code->mov(regScratch0, qword[JIT_THIS + GPR_OFFSET(RS(instr), this)]);
      code->mov(regScratch1, qword[JIT_THIS + GPR_OFFSET(RT(instr), this)]);
      code->xor_(regArg1, regArg1);
      code->cmp(regScratch0, regScratch1);
      code->sete(regArg1.cvt8());
      code->mov(rax, (u64)trap);
      emitCall(rax);
      break;
    case 0x36:
      code->mov(regScratch0, qword[JIT_THIS + GPR_OFFSET(RS(instr), this)]);
      code->mov(regScratch1, qword[JIT_THIS + GPR_OFFSET(RT(instr), this)]);
      code->xor_(regArg1, regArg1);
      code->cmp(regScratch0, regScratch1);
      code->setne(regArg1.cvt8());
      code->mov(rax, (u64)trap);
      emitCall(rax);
      break;
    case 0x38:
      code->mov(rax, (u64)dsll);
      emitCall(rax);
      break;
    case 0x3A:
      code->mov(rax, (u64)dsrl);
      emitCall(rax);
      break;
    case 0x3B:
      code->mov(rax, (u64)dsra);
      emitCall(rax);
      break;
    case 0x3C:
      code->mov(rax, (u64)dsll32);
      emitCall(rax);
      break;
    case 0x3E:
      code->mov(rax, (u64)dsrl32);
      emitCall(rax);
      break;
    case 0x3F:
      code->mov(rax, (u64)dsra32);
      emitCall(rax);
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
      code->mov(regScratch0, qword[rdi + GPR_OFFSET(RS(instr), this)]);
      code->xor_(regArg2, regArg2);
      code->cmp(regScratch0, 0);
      code->setl(regArg2.cvt8());
      code->mov(rax, (u64)b);
      emitCall(rax);
      break;
    case 0x01:
      code->mov(regScratch0, qword[JIT_THIS + GPR_OFFSET(RS(instr), this)]);
      code->xor_(regArg2, regArg2);
      code->cmp(regScratch0, 0);
      code->setge(regArg2.cvt8());
      code->mov(rax, (u64)b);
      emitCall(rax);
      break;
    case 0x02:
      code->mov(regScratch0, qword[JIT_THIS + GPR_OFFSET(RS(instr), this)]);
      code->xor_(regArg2, regArg2);
      code->cmp(regScratch0, 0);
      code->setl(dl);
      code->mov(rax, (u64)bl);
      emitCall(rax);
      break;
    case 0x03:
      code->mov(regScratch0, qword[JIT_THIS + GPR_OFFSET(RS(instr), this)]);
      code->xor_(regArg2, regArg2);
      code->cmp(regScratch0, 0);
      code->setge(regArg2.cvt8());
      code->mov(rax, (u64)bl);
      emitCall(rax);
      break;
    case 0x08:
      code->mov(regScratch0, qword[JIT_THIS + GPR_OFFSET(RS(instr), this)]);
      code->xor_(regArg1, regArg1);
      code->cmp(regScratch0, s64(s16(instr)));
      code->setge(regArg1.cvt8());
      code->mov(rax, (u64)trap);
      emitCall(rax);
      break;
    case 0x09:
      code->mov(regScratch0, qword[JIT_THIS + GPR_OFFSET(RS(instr), this)]);
      code->xor_(regArg1, regArg1);
      code->cmp(regScratch0, u64(s64(s16(instr))));
      code->setae(regArg1.cvt8());
      code->mov(rax, (u64)trap);
      emitCall(rax);
      break;
    case 0x0A:
      code->mov(regScratch0, qword[JIT_THIS + GPR_OFFSET(RS(instr), this)]);
      code->xor_(regArg1, regArg1);
      code->cmp(regScratch0, s64(s16(instr)));
      code->setl(regArg1.cvt8());
      code->mov(rax, (u64)trap);
      emitCall(rax);
      break;
    case 0x0B:
      code->mov(regScratch0, qword[JIT_THIS + GPR_OFFSET(RS(instr), this)]);
      code->xor_(regArg1, regArg1);
      code->cmp(regScratch0, u64(s64(s16(instr))));
      code->setb(regArg1.cvt8());
      code->mov(rax, (u64)trap);
      emitCall(rax);
      break;
    case 0x0C:
      code->mov(regScratch0, qword[JIT_THIS + GPR_OFFSET(RS(instr), this)]);
      code->xor_(regArg1, regArg1);
      code->cmp(regScratch0, s64(s16(instr)));
      code->sete(regArg1.cvt8());
      code->mov(rax, (u64)trap);
      emitCall(rax);
      break;
    case 0x0E:
      code->mov(regScratch0, qword[JIT_THIS + GPR_OFFSET(RS(instr), this)]);
      code->xor_(regArg1, regArg1);
      code->cmp(regScratch0, s64(s16(instr)));
      code->setne(regArg1.cvt8());
      code->mov(rax, (u64)trap);
      emitCall(rax);
      break;
    case 0x10:
      code->mov(regScratch1, qword[JIT_THIS + GPR_OFFSET(RS(instr), this)]);
      code->xor_(regArg2, regArg2);
      code->cmp(regScratch1, 0);
      code->setl(regArg2.cvt8());
      code->mov(rax, (u64)blink);
      emitCall(rax);
      break;
    case 0x11:
      code->mov(regScratch1, qword[JIT_THIS + GPR_OFFSET(RS(instr), this)]);
      code->xor_(regArg2, regArg2);
      code->cmp(regScratch1, 0);
      code->setge(regArg2.cvt8());
      code->mov(rax, (u64)blink);
      emitCall(rax);
      break;
    case 0x12:
      code->mov(regScratch1, qword[JIT_THIS + GPR_OFFSET(RS(instr), this)]);
      code->xor_(regArg2, regArg2);
      code->cmp(regScratch1, 0);
      code->setl(regArg2.cvt8());
      code->mov(rax, (u64)bllink);
      emitCall(rax);
      break;
    case 0x13:
      code->mov(regScratch1, qword[JIT_THIS + GPR_OFFSET(RS(instr), this)]);
      code->xor_(regArg2, regArg2);
      code->cmp(regScratch1, 0);
      code->setge(regArg2.cvt8());
      code->mov(rax, (u64)bllink);
      emitCall(rax);
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

void JIT::Emit(u32 instr) {
  u8 mask = (instr >> 26) & 0x3f;

  // 00rr_rccc
  switch(mask) { // TODO: named constants for clearer code
    case 0x00: special(instr); break;
    case 0x01: regimm(instr); break;
    case 0x02:
      code->mov(rax, (u64)j);
      emitCall(rax);
      break;
    case 0x03:
      code->mov(rax, (u64)jal);
      emitCall(rax);
      break;
    case 0x04:
      code->mov(regScratch0, qword[JIT_THIS + GPR_OFFSET(RS(instr), this)]);
      code->mov(regScratch1, qword[JIT_THIS + GPR_OFFSET(RT(instr), this)]);
      code->xor_(regArg2, regArg2);
      code->cmp(regScratch0, regScratch1);
      code->sete(regArg2.cvt8());
      code->mov(rax, (u64)b);
      emitCall(rax);
      break;
    case 0x05:
      code->mov(regScratch0, qword[JIT_THIS + GPR_OFFSET(RS(instr), this)]);
      code->mov(regScratch1, qword[JIT_THIS + GPR_OFFSET(RT(instr), this)]);
      code->xor_(regArg2, regArg2);
      code->cmp(regScratch0, regScratch1);
      code->setne(regArg2.cvt8());
      code->mov(rax, (u64)b);
      emitCall(rax);
      break;
    case 0x06:
      code->mov(regScratch0, qword[JIT_THIS + GPR_OFFSET(RS(instr), this)]);
      code->xor_(regArg2, regArg2);
      code->test(regScratch0, regScratch0);
      code->setnz(regArg2.cvt8());
      code->mov(rax, (u64)b);
      emitCall(rax);
      break;
    case 0x07:
      code->mov(regScratch0, qword[JIT_THIS + GPR_OFFSET(RS(instr), this)]);
      code->xor_(regArg2, regArg2);
      code->test(regScratch0, regScratch0);
      code->setg(regArg2.cvt8());
      code->mov(rax, (u64)b);
      emitCall(rax);
      break;
    case 0x08:
      code->mov(rax, (u64)addi);
      emitCall(rax);
      break;
    case 0x09:
      code->mov(rax, (u64)addiu);
      emitCall(rax);
      break;
    case 0x0A:
      code->mov(rax, (u64)slti);
      emitCall(rax);
      break;
    case 0x0B:
      code->mov(rax, (u64)sltiu);
      emitCall(rax);
      break;
    case 0x0C:
      code->mov(rax, (u64)andi);
      emitCall(rax);
      break;
    case 0x0D:
      code->mov(rax, (u64)ori);
      emitCall(rax);
      break;
    case 0x0E:
      code->mov(rax, (u64)xori);
      emitCall(rax);
      break;
    case 0x0F:
      code->mov(rax, (u64)lui);
      emitCall(rax);
      break;
    case 0x10: cop0Emit(*this, instr); break;
    case 0x11: cop1Emit(*this, instr); break;
    case 0x12: cop2Decode(instr); break;
    case 0x14:
      code->mov(regScratch0, qword[JIT_THIS + GPR_OFFSET(RS(instr), this)]);
      code->mov(regScratch1, qword[JIT_THIS + GPR_OFFSET(RT(instr), this)]);
      code->xor_(regArg2, regArg2);
      code->cmp(regScratch0, regScratch1);
      code->sete(regArg2.cvt8());
      code->mov(rax, (u64)bl);
      emitCall(rax);
      break;
    case 0x15:
      code->mov(regScratch0, qword[JIT_THIS + GPR_OFFSET(RS(instr), this)]);
      code->mov(regScratch1, qword[JIT_THIS + GPR_OFFSET(RT(instr), this)]);
      code->xor_(regArg2, regArg2);
      code->cmp(regScratch0, regScratch1);
      code->setne(regArg2.cvt8());
      code->mov(rax, (u64)bl);
      emitCall(rax);
      break;
    case 0x16:
      code->mov(regScratch0, qword[JIT_THIS + GPR_OFFSET(RS(instr), this)]);
      code->xor_(regArg2, regArg2);
      code->cmp(regScratch0, 0);
      code->setle(regArg2.cvt8());
      code->mov(rax, (u64)bl);
      emitCall(rax);
      break;
    case 0x17:
      code->mov(regScratch0, qword[JIT_THIS + GPR_OFFSET(RS(instr), this)]);
      code->xor_(regArg2, regArg2);
      code->cmp(regScratch0, 0);
      code->setg(regArg2.cvt8());
      code->mov(rax, (u64)b);
      emitCall(rax);
      break;
    case 0x18:
      code->mov(rax, (u64)daddi);
      emitCall(rax);
      break;
    case 0x19:
      code->mov(rax, (u64)daddiu);
      emitCall(rax);
      break;
    case 0x1A:
      code->mov(rax, (u64)ldl);
      emitCall(rax);
      break;
    case 0x1B:
      code->mov(rax, (u64)ldr);
      emitCall(rax);
      break;
    case 0x1F: Util::panic("[RECOMPILER] Unhandled reserved instruction exception {:016X}", regs.oldPC); break;
    case 0x20:
      code->mov(rax, (u64)lb);
      emitCall(rax);
      break;
    case 0x21:
      code->mov(rax, (u64)lh);
      emitCall(rax);
      break;
    case 0x22:
      code->mov(rax, (u64)lwl);
      emitCall(rax);
      break;
    case 0x23:
      code->mov(rax, (u64)lw);
      emitCall(rax);
      break;
    case 0x24:
      code->mov(rax, (u64)lbu);
      emitCall(rax);
      break;
    case 0x25:
      code->mov(rax, (u64)lhu);
      emitCall(rax);
      break;
    case 0x26:
      code->mov(rax, (u64)lwr);
      emitCall(rax);
      break;
    case 0x27:
      code->mov(rax, (u64)lwu);
      emitCall(rax);
      break;
    case 0x28:
      code->mov(rax, (u64)sb);
      emitCall(rax);
      break;
    case 0x29:
      code->mov(rax, (u64)sh);
      emitCall(rax);
      break;
    case 0x2A:
      code->mov(rax, (u64)swl);
      emitCall(rax);
      break;
    case 0x2B:
      code->mov(rax, (u64)sw);
      emitCall(rax);
      break;
    case 0x2C:
      code->mov(rax, (u64)sdl);
      emitCall(rax);
      break;
    case 0x2D:
      code->mov(rax, (u64)sdr);
      emitCall(rax);
      break;
    case 0x2E:
      code->mov(rax, (u64)swr);
      emitCall(rax);
      break;
    case 0x2F: break; // CACHE
    case 0x30:
      code->mov(rax, (u64)ll);
      emitCall(rax);
      break;
    case 0x31:
      code->mov(rax, (u64)lwc1);
      emitCall(rax);
      break;
    case 0x34:
      code->mov(rax, (u64)lld);
      emitCall(rax);
      break;
    case 0x35:
      code->mov(rax, (u64)ldc1);
      emitCall(rax);
      break;
    case 0x37:
      code->mov(rax, (u64)ld);
      emitCall(rax);
      break;
    case 0x38:
      code->mov(rax, (u64)sc);
      emitCall(rax);
      break;
    case 0x39:
      code->mov(rax, (u64)swc1);
      emitCall(rax);
      break;
    case 0x3C:
      code->mov(rax, (u64)scd);
      emitCall(rax);
      break;
    case 0x3D:
      code->mov(rax, (u64)sdc1);
      emitCall(rax);
      break;
    case 0x3F:
      code->mov(rax, (u64)sd);
      emitCall(rax);
      break;
    default:
      Util::panic("Unimplemented instruction {:02X} ({:08X}) (pc: {:016X})", mask, instr, (u64)regs.oldPC);
  }
}
}
