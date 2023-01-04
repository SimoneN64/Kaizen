#include <dynarec/instructions.hpp>
#include <dynarec/cop/cop1decode.hpp>
#include <dynarec/cop/cop0decode.hpp>
#include <Registers.hpp>

namespace n64::JIT {
#define GPR_OFFSET(x) ((uintptr_t)&regs.gpr[(x)] - (uintptr_t)&regs)

void Dynarec::cop2Decode(n64::Registers& regs, u32 instr) {
  code.mov(rdi, (u64)this);
  code.mov(rsi, (u64)&regs);
  code.mov(rdx, (u64)instr);

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
      Util::panic("[RECOMPILER] Unhandled reserved instruction exception {:016X}\n", (u64)regs.pc);
  }
}

bool Dynarec::special(n64::Registers& regs, u32 instr) {
  u8 mask = (instr & 0x3F);
  bool res = false;

  // 00rr_rccc
  switch (mask) { // TODO: named constants for clearer code
    case 0:
      if (instr != 0) {
        code.mov(rsi, (u64)instr);
        code.mov(rax, (u64)sll);
        code.call(rax);
      }
      break;
    case 0x02:
      code.mov(rsi, (u64)instr);
      code.mov(rax, (u64)srl);
      code.call(rax);
      break;
    case 0x03:
      code.mov(rsi, (u64)instr);
      code.mov(rax, (u64)sra);
      code.call(rax);
      break;
    case 0x04:
      code.mov(rsi, (u64)instr);
      code.mov(rax, (u64)sllv);
      code.call(rax);
      break;
    case 0x06:
      code.mov(rsi, (u64)instr);
      code.mov(rax, (u64)srlv);
      code.call(rax);
      break;
    case 0x07:
      code.mov(rsi, (u64)instr);
      code.mov(rax, (u64)srav);
      code.call(rax);
      break;
    case 0x08:
      code.mov(rsi, (u64)instr);
      code.mov(rax, (u64)jr);
      code.call(rax);
      res = true;
      break;
    case 0x09:
      code.mov(rsi, (u64)instr);
      code.mov(rax, (u64)jalr);
      code.call(rax);
      res = true;
      break;
    case 0x0C: Util::panic("[RECOMPILER] Unhandled syscall instruction {:016X}\n", (u64)regs.pc);
    case 0x0D: Util::panic("[RECOMPILER] Unhandled break instruction {:016X}\n", (u64)regs.pc);
    case 0x0F: break; // SYNC
    case 0x10:
      code.mov(rsi, (u64)instr);
      code.mov(rax, (u64)mfhi);
      code.call(rax);
      break;
    case 0x11:
      code.mov(rsi, (u64)instr);
      code.mov(rax, (u64)mthi);
      code.call(rax);
      break;
    case 0x12:
      code.mov(rsi, (u64)instr);
      code.mov(rax, (u64)mflo);
      code.call(rax);
      break;
    case 0x13:
      code.mov(rsi, (u64)instr);
      code.mov(rax, (u64)mtlo);
      code.call(rax);
      break;
    case 0x14:
      code.mov(rsi, (u64)instr);
      code.mov(rax, (u64)dsllv);
      code.call(rax);
      break;
    case 0x16:
      code.mov(rsi, (u64)instr);
      code.mov(rax, (u64)dsrlv);
      code.call(rax);
      break;
    case 0x17:
      code.mov(rsi, (u64)instr);
      code.mov(rax, (u64)dsrav);
      code.call(rax);
      break;
    case 0x18:
      code.mov(rsi, (u64)instr);
      code.mov(rax, (u64)mult);
      code.call(rax);
      break;
    case 0x19:
      code.mov(rsi, (u64)instr);
      code.mov(rax, (u64)multu);
      code.call(rax);
      break;
    case 0x1A:
      code.mov(rsi, (u64)instr);
      code.mov(rax, (u64)div);
      code.call(rax);
      break;
    case 0x1B:
      code.mov(rsi, (u64)instr);
      code.mov(rax, (u64)divu);
      code.call(rax);
      break;
    case 0x1C:
      code.mov(rsi, (u64)instr);
      code.mov(rax, (u64)dmult);
      code.call(rax);
      break;
    case 0x1D:
      code.mov(rsi, (u64)instr);
      code.mov(rax, (u64)dmultu);
      code.call(rax);
      break;
    case 0x1E:
      code.mov(rsi, (u64)instr);
      code.mov(rax, (u64)ddiv);
      code.call(rax);
      break;
    case 0x1F:
      code.mov(rsi, (u64)instr);
      code.mov(rax, (u64)ddivu);
      code.call(rax);
      break;
    case 0x20:
      code.mov(rsi, (u64)instr);
      code.mov(rax, (u64)add);
      code.call(rax);
      break;
    case 0x21:
      code.mov(rsi, (u64)instr);
      code.mov(rax, (u64)addu);
      code.call(rax);
      break;
    case 0x22:
      code.mov(rsi, (u64)instr);
      code.mov(rax, (u64)sub);
      code.call(rax);
      break;
    case 0x23:
      code.mov(rsi, (u64)instr);
      code.mov(rax, (u64)subu);
      code.call(rax);
      break;
    case 0x24:
      code.mov(rsi, (u64)instr);
      code.mov(rax, (u64)and_);
      code.call(rax);
      break;
    case 0x25:
      code.mov(rsi, (u64)instr);
      code.mov(rax, (u64)or_);
      code.call(rax);
      break;
    case 0x26:
      code.mov(rsi, (u64)instr);
      code.mov(rax, (u64)xor_);
      code.call(rax);
      break;
    case 0x27:
      code.mov(rsi, (u64)instr);
      code.mov(rax, (u64)nor);
      code.call(rax);
      break;
    case 0x2A:
      code.mov(rsi, (u64)instr);
      code.mov(rax, (u64)slt);
      code.call(rax);
      break;
    case 0x2B:
      code.mov(rsi, (u64)instr);
      code.mov(rax, (u64)sltu);
      code.call(rax);
      break;
    case 0x2C:
      code.mov(rsi, (u64)instr);
      code.mov(rax, (u64)dadd);
      code.call(rax);
      break;
    case 0x2D:
      code.mov(rsi, (u64)instr);
      code.mov(rax, (u64)daddu);
      code.call(rax);
      break;
    case 0x2E:
      code.mov(rsi, (u64)instr);
      code.mov(rax, (u64)dsub);
      code.call(rax);
      break;
    case 0x2F:
      code.mov(rsi, (u64)instr);
      code.mov(rax, (u64)dsubu);
      code.call(rax);
      break;
    case 0x30:
      code.mov(cl, 0);
      code.mov(ch, 1);
      code.cmp(qword[rdi + GPR_OFFSET(RS(instr))],
               qword[rdi + GPR_OFFSET(RT(instr))]);
      code.cmovge(cl, ch);
      code.mov(rsi, cl.cvt64());
      code.mov(rax, (u64)trap);
      code.call(rax);
      res = true;
      break;
    case 0x31:
      code.mov(cl, 0);
      code.mov(ch, 1);
      code.cmp(qword[rdi + GPR_OFFSET(RS(instr))],
               qword[rdi + GPR_OFFSET(RT(instr))]);
      code.cmovae(cl, ch);
      code.mov(rsi, cl.cvt64());
      code.mov(rax, (u64)trap);
      code.call(rax);
      res = true;
      break;
    case 0x32:
      code.mov(cl, 0);
      code.mov(ch, 1);
      code.cmp(qword[rdi + GPR_OFFSET(RS(instr))],
               qword[rdi + GPR_OFFSET(RT(instr))]);
      code.cmovl(cl, ch);
      code.mov(rsi, cl.cvt64());
      code.mov(rax, (u64)trap);
      code.call(rax);
      res = true;
      break;
    case 0x33:
      code.mov(cl, 0);
      code.mov(ch, 1);
      code.cmp(qword[rdi + GPR_OFFSET(RS(instr))],
               qword[rdi + GPR_OFFSET(RT(instr))]);
      code.cmovb(cl, ch);
      code.mov(rsi, cl.cvt64());
      code.mov(rax, (u64)trap);
      code.call(rax);
      res = true;
      break;
    case 0x34:
      code.mov(cl, 0);
      code.mov(ch, 1);
      code.cmp(qword[rdi + GPR_OFFSET(RS(instr))],
               qword[rdi + GPR_OFFSET(RT(instr))]);
      code.cmove(cl, ch);
      code.mov(rsi, cl.cvt64());
      code.mov(rax, (u64)trap);
      code.call(rax);
      res = true;
      break;
    case 0x36:
      code.mov(cl, 0);
      code.mov(ch, 1);
      code.cmp(qword[rdi + GPR_OFFSET(RS(instr))],
               qword[rdi + GPR_OFFSET(RT(instr))]);
      code.cmovne(cl, ch);
      code.mov(rsi, cl.cvt64());
      code.mov(rax, (u64)trap);
      code.call(rax);
      res = true;
      break;
    case 0x38:
      code.mov(rsi, (u64)instr);
      code.mov(rax, (u64)dsll);
      code.call(rax);
      break;
    case 0x3A:
      code.mov(rsi, (u64)instr);
      code.mov(rax, (u64)dsrl);
      code.call(rax);
      break;
    case 0x3B:
      code.mov(rsi, (u64)instr);
      code.mov(rax, (u64)dsra);
      code.call(rax);
      break;
    case 0x3C:
      code.mov(rsi, (u64)instr);
      code.mov(rax, (u64)dsll32);
      code.call(rax);
      break;
    case 0x3E:
      code.mov(rsi, (u64)instr);
      code.mov(rax, (u64)dsrl32);
      code.call(rax);
      break;
    case 0x3F:
      code.mov(rsi, (u64)instr);
      code.mov(rax, (u64)dsra32);
      code.call(rax);
      break;
    default:
      Util::panic("Unimplemented special {} {} ({:08X}) (pc: {:016X})\n", (mask >> 3) & 7, mask & 7, instr, (u64)regs.oldPC);
  }

  return res;
}

bool Dynarec::regimm(n64::Registers& regs, u32 instr) {
  u8 mask = ((instr >> 16) & 0x1F);
  // 000r_rccc
  switch (mask) { // TODO: named constants for clearer code
    case 0x00:
      code.mov(rsi, (u64)instr);
      code.mov(cl, 0);
      code.mov(ch, 1);
      code.cmp(qword[rdi + GPR_OFFSET(RS(instr))], 0);
      code.cmovl(cl, ch);
      code.mov(rdx, cl.cvt64());
      code.mov(rax, (u64)b);
      code.call(rax);
      break;
    case 0x01:
      code.mov(rsi, (u64)instr);
      code.mov(cl, 0);
      code.mov(ch, 1);
      code.cmp(qword[rdi + GPR_OFFSET(RS(instr))], 0);
      code.cmovge(cl, ch);
      code.mov(rdx, cl.cvt64());
      code.mov(rax, (u64)b);
      code.call(rax);
      break;
    case 0x02:
      code.mov(rsi, (u64)instr);
      code.mov(cl, 0);
      code.mov(ch, 1);
      code.cmp(qword[rdi + GPR_OFFSET(RS(instr))], 0);
      code.cmovl(cl, ch);
      code.mov(rdx, cl.cvt64());
      code.mov(rax, (u64)bl);
      code.call(rax);
      break;
    case 0x03:
      code.mov(rsi, (u64)instr);
      code.mov(cl, 0);
      code.mov(ch, 1);
      code.cmp(qword[rdi + GPR_OFFSET(RS(instr))], 0);
      code.cmovge(cl, ch);
      code.mov(rdx, cl.cvt64());
      code.mov(rax, (u64)bl);
      code.call(rax);
      break;
    case 0x08:
      code.mov(cl, 0);
      code.mov(ch, 1);
      code.cmp(qword[rdi + GPR_OFFSET(RS(instr))],
               s64(s16(instr)));
      code.cmovge(cl, ch);
      code.mov(rsi, cl.cvt64());
      code.mov(rax, (u64)trap);
      code.call(rax);
      break;
    case 0x09:
      code.mov(cl, 0);
      code.mov(ch, 1);
      code.cmp(qword[rdi + GPR_OFFSET(RS(instr))],
               u64(s64(s16(instr))));
      code.cmovae(cl, ch);
      code.mov(rsi, cl.cvt64());
      code.mov(rax, (u64)trap);
      code.call(rax);
      break;
    case 0x0A:
      code.mov(cl, 0);
      code.mov(ch, 1);
      code.cmp(qword[rdi + GPR_OFFSET(RS(instr))],
               s64(s16(instr)));
      code.cmovl(cl, ch);
      code.mov(rsi, cl.cvt64());
      code.mov(rax, (u64)trap);
      code.call(rax);
      break;
    case 0x0B:
      code.mov(cl, 0);
      code.mov(ch, 1);
      code.cmp(qword[rdi + GPR_OFFSET(RS(instr))],
               u64(s64(s16(instr))));
      code.cmovb(cl, ch);
      code.mov(rsi, cl.cvt64());
      code.mov(rax, (u64)trap);
      code.call(rax);
      break;
    case 0x0C:
      code.mov(cl, 0);
      code.mov(ch, 1);
      code.cmp(qword[rdi + GPR_OFFSET(RS(instr))],
               s64(s16(instr)));
      code.cmove(cl, ch);
      code.mov(rsi, cl.cvt64());
      code.mov(rax, (u64)trap);
      code.call(rax);
      break;
    case 0x0E:
      code.mov(cl, 0);
      code.mov(ch, 1);
      code.cmp(qword[rdi + GPR_OFFSET(RS(instr))],
               s64(s16(instr)));
      code.cmovne(cl, ch);
      code.mov(rsi, cl.cvt64());
      code.mov(rax, (u64)trap);
      code.call(rax);
      break;
    case 0x10:
      code.mov(rsi, (u64)instr);
      code.mov(cl, 0);
      code.mov(ch, 1);
      code.cmp(qword[rdi + GPR_OFFSET(RS(instr))], 0);
      code.cmovl(cl, ch);
      code.mov(rdx, cl.cvt64());
      code.mov(rax, (u64)blink);
      code.call(rax);
      break;
    case 0x11:
      code.mov(rsi, (u64)instr);
      code.mov(cl, 0);
      code.mov(ch, 1);
      code.cmp(qword[rdi + GPR_OFFSET(RS(instr))], 0);
      code.cmovge(cl, ch);
      code.mov(rdx, cl.cvt64());
      code.mov(rax, (u64)blink);
      code.call(rax);
      break;
    case 0x12:
      code.mov(rsi, (u64)instr);
      code.mov(cl, 0);
      code.mov(ch, 1);
      code.cmp(qword[rdi + GPR_OFFSET(RS(instr))], 0);
      code.cmovl(cl, ch);
      code.mov(rdx, cl.cvt64());
      code.mov(rax, (u64)bllink);
      code.call(rax);
      break;
    case 0x13:
      code.mov(rsi, (u64)instr);
      code.mov(cl, 0);
      code.mov(ch, 1);
      code.cmp(qword[rdi + GPR_OFFSET(RS(instr))], 0);
      code.cmovge(cl, ch);
      code.mov(rdx, cl.cvt64());
      code.mov(rax, (u64)bllink);
      code.call(rax);
      break;
    default:
      Util::panic("Unimplemented regimm {} {} ({:08X}) (pc: {:016X})\n", (mask >> 3) & 3, mask & 7, instr, (u64)regs.oldPC);
  }

  return true;
}

bool Dynarec::Exec(n64::Registers& regs, Mem& mem, u32 instr) {
  u8 mask = (instr >> 26) & 0x3f;
  bool res = false;

  // 00rr_rccc
  switch(mask) { // TODO: named constants for clearer code
    case 0x00: res = special(regs, instr); break;
    case 0x01: res = regimm(regs, instr); break;
    case 0x02:
      code.mov(rsi, (u64)instr);
      code.mov(rax, (u64)j);
      code.call(rax);
      res = true;
      break;
    case 0x03:
      code.mov(rsi, (u64)instr);
      code.mov(rax, (u64)jal);
      code.call(rax);
      res = true;
      break;
    case 0x04:
      code.mov(rsi, (u64)instr);
      code.mov(rbx, qword[rdi + GPR_OFFSET(RS(instr))]);
      code.mov(rcx, qword[rdi + GPR_OFFSET(RT(instr))]);
      code.xor_(rdx, rdx);
      code.cmp(rbx, rcx);
      code.sete(dl);
      code.mov(rax, (u64)b);
      code.call(rax);
      res = true;
      break;
    case 0x05:
      code.mov(rsi, (u64)instr);
      code.mov(rbx, qword[rdi + GPR_OFFSET(RS(instr))]);
      code.mov(rcx, qword[rdi + GPR_OFFSET(RT(instr))]);
      code.xor_(rdx, rdx);
      code.cmp(rbx, rcx);
      code.setne(dl);
      code.mov(rax, (u64)b);
      code.call(rax);
      res = true;
      break;
    case 0x06:
      code.mov(rsi, (u64)instr);
      code.mov(rbx, qword[rdi + GPR_OFFSET(RS(instr))]);
      code.test(rbx, rbx);
      code.xor_(rdx, rdx);
      code.setnz(dl);
      code.mov(rax, (u64)b);
      code.call(rax);
      res = true;
      break;
    case 0x07:
      code.mov(rsi, (u64)instr);
      code.mov(rbx, qword[rdi + GPR_OFFSET(RS(instr))]);
      code.test(rbx, rbx);
      code.xor_(rdx, rdx);
      code.setg(dl);
      code.mov(rax, (u64)b);
      code.call(rax);
      res = true;
      break;
    case 0x08:
      code.mov(rsi, (u64)instr);
      code.mov(rax, (u64)addi);
      code.call(rax);
      break;
    case 0x09:
      code.mov(rsi, (u64)instr);
      code.mov(rax, (u64)addiu);
      code.call(rax);
      break;
    case 0x0A:
      code.mov(rsi, (u64)instr);
      code.mov(rax, (u64)slti);
      code.call(rax);
      break;
    case 0x0B:
      code.mov(rsi, (u64)instr);
      code.mov(rax, (u64)sltiu);
      code.call(rax);
      break;
    case 0x0C:
      code.mov(rsi, (u64)instr);
      code.mov(rax, (u64)andi);
      code.call(rax);
      break;
    case 0x0D:
      code.mov(rsi, (u64)instr);
      code.mov(rax, (u64)ori);
      code.call(rax);
      break;
    case 0x0E:
      code.mov(rsi, (u64)instr);
      code.mov(rax, (u64)xori);
      code.call(rax);
      break;
    case 0x0F:
      code.mov(rsi, (u64)instr);
      code.mov(rax, (u64)lui);
      code.call(rax);
      break;
    case 0x10: cop0Decode(regs, instr, *this); break;
    case 0x11: res = cop1Decode(regs, instr, *this); break;
    case 0x12: cop2Decode(regs, instr); break;
    case 0x14:
      code.mov(rsi, (u64)instr);
      code.mov(cl, 0);
      code.mov(ch, 1);
      code.cmp(qword[rdi + GPR_OFFSET(RS(instr))],
               qword[rdi + GPR_OFFSET(RT(instr))]);
      code.cmove(cl, ch);
      code.mov(rdx, cl.cvt64());
      code.mov(rax, (u64)bl);
      code.call(rax);
      break;
    case 0x15:
      code.mov(rsi, (u64)instr);
      code.mov(cl, 0);
      code.mov(ch, 1);
      code.cmp(qword[rdi + GPR_OFFSET(RS(instr))],
               qword[rdi + GPR_OFFSET(RT(instr))]);
      code.cmovne(cl, ch);
      code.mov(rdx, cl.cvt64());
      code.mov(rax, (u64)bl);
      code.call(rax);
      break;
    case 0x16:
      code.mov(rsi, (u64)instr);
      code.mov(cl, 0);
      code.mov(ch, 1);
      code.cmp(qword[rdi + GPR_OFFSET(RS(instr))], 0);
      code.cmovle(cl, ch);
      code.mov(rdx, cl.cvt64());
      code.mov(rax, (u64)bl);
      code.call(rax);
      break;
    case 0x17:
      code.mov(rsi, (u64)instr);
      code.mov(cl, 0);
      code.mov(ch, 1);
      code.cmp(qword[rdi + GPR_OFFSET(RS(instr))], 0);
      code.cmovg(cl, ch);
      code.mov(rdx, cl.cvt64());
      code.mov(rax, (u64)b);
      code.call(rax);
      break;
    case 0x18:
      code.mov(rsi, (u64)instr);
      code.mov(rax, (u64)daddi);
      code.call(rax);
      break;
    case 0x19:
      code.mov(rsi, (u64)instr);
      code.mov(rax, (u64)daddiu);
      code.call(rax);
      break;
    case 0x1A:
      code.mov(rsi, (u64)&mem);
      code.mov(rdx, (u64)instr);
      code.mov(rax, (u64)ldl);
      code.call(rax);
      break;
    case 0x1B:
      code.mov(rsi, (u64)&mem);
      code.mov(rdx, (u64)instr);
      code.mov(rax, (u64)ldr);
      code.call(rax);
      break;
    case 0x1F: Util::panic("[RECOMPILER] Unhandled reserved instruction exception {:016X}\n", regs.oldPC); break;
    case 0x20:
      code.mov(rsi, (u64)&mem);
      code.mov(rdx, (u64)instr);
      code.mov(rax, (u64)lb);
      code.call(rax);
      break;
    case 0x21:
      code.mov(rsi, (u64)&mem);
      code.mov(rdx, (u64)instr);
      code.mov(rax, (u64)lh);
      code.call(rax);
      break;
    case 0x22:
      code.mov(rsi, (u64)&mem);
      code.mov(rdx, (u64)instr);
      code.mov(rax, (u64)lwl);
      code.call(rax);
      break;
    case 0x23:
      code.mov(rsi, (u64)&mem);
      code.mov(rdx, (u64)instr);
      code.mov(rax, (u64)lw);
      code.call(rax);
      break;
    case 0x24:
      code.mov(rsi, (u64)&mem);
      code.mov(rdx, (u64)instr);
      code.mov(rax, (u64)lbu);
      code.call(rax);
      break;
    case 0x25:
      code.mov(rsi, (u64)&mem);
      code.mov(rdx, (u64)instr);
      code.mov(rax, (u64)lhu);
      code.call(rax);
      break;
    case 0x26:
      code.mov(rsi, (u64)&mem);
      code.mov(rdx, (u64)instr);
      code.mov(rax, (u64)lwr);
      code.call(rax);
      break;
    case 0x27:
      code.mov(rsi, (u64)&mem);
      code.mov(rdx, (u64)instr);
      code.mov(rax, (u64)lwu);
      code.call(rax);
      break;
    case 0x28:
      code.mov(rsi, (u64)&mem);
      code.mov(rdx, (u64)instr);
      code.mov(rax, (u64)sb);
      code.call(rax);
      break;
    case 0x29:
      code.mov(rsi, (u64)&mem);
      code.mov(rdx, (u64)instr);
      code.mov(rax, (u64)sh);
      code.call(rax);
      break;
    case 0x2A:
      code.mov(rsi, (u64)&mem);
      code.mov(rdx, (u64)instr);
      code.mov(rax, (u64)swl);
      code.call(rax);
      break;
    case 0x2B:
      code.mov(rsi, (u64)&mem);
      code.mov(rdx, (u64)instr);
      code.mov(rax, (u64)sw);
      code.call(rax);
      break;
    case 0x2C:
      code.mov(rsi, (u64)&mem);
      code.mov(rdx, (u64)instr);
      code.mov(rax, (u64)sdl);
      code.call(rax);
      break;
    case 0x2D:
      code.mov(rsi, (u64)&mem);
      code.mov(rdx, (u64)instr);
      code.mov(rax, (u64)sdr);
      code.call(rax);
      break;
    case 0x2E:
      code.mov(rsi, (u64)&mem);
      code.mov(rdx, (u64)instr);
      code.mov(rax, (u64)swr);
      code.call(rax);
      break;
    case 0x2F: break; // CACHE
    case 0x30:
      code.mov(rsi, (u64)&mem);
      code.mov(rdx, (u64)instr);
      code.mov(rax, (u64)ll);
      code.call(rax);
      break;
    case 0x31:
      code.mov(rsi, (u64)&mem);
      code.mov(rdx, (u64)instr);
      code.mov(rax, (u64)lwc1);
      code.call(rax);
      break;
    case 0x34:
      code.mov(rsi, (u64)&mem);
      code.mov(rdx, (u64)instr);
      code.mov(rax, (u64)lld);
      code.call(rax);
      break;
    case 0x35:
      code.mov(rsi, (u64)&mem);
      code.mov(rdx, (u64)instr);
      code.mov(rax, (u64)ldc1);
      code.call(rax);
      break;
    case 0x37:
      code.mov(rsi, (u64)&mem);
      code.mov(rdx, (u64)instr);
      code.mov(rax, (u64)ld);
      code.call(rax);
      break;
    case 0x38:
      code.mov(rsi, (u64)&mem);
      code.mov(rdx, (u64)instr);
      code.mov(rax, (u64)sc);
      code.call(rax);
      break;
    case 0x39:
      code.mov(rsi, (u64)&mem);
      code.mov(rdx, (u64)instr);
      code.mov(rax, (u64)swc1);
      code.call(rax);
      break;
    case 0x3C:
      code.mov(rsi, (u64)&mem);
      code.mov(rdx, (u64)instr);
      code.mov(rax, (u64)scd);
      code.call(rax);
      break;
    case 0x3D:
      code.mov(rsi, (u64)&mem);
      code.mov(rdx, (u64)instr);
      code.mov(rax, (u64)sdc1);
      code.call(rax);
      break;
    case 0x3F:
      code.mov(rsi, (u64)&mem);
      code.mov(rdx, (u64)instr);
      code.mov(rax, (u64)sd);
      code.call(rax);
      break;
    default:
      Util::panic("Unimplemented instruction {:02X} ({:08X}) (pc: {:016X})\n", mask, instr, (u64)regs.oldPC);
  }

  return res;
}
}
