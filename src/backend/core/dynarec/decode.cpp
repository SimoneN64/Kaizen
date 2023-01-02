#include <dynarec/instructions.hpp>
#include <dynarec/cop/cop1decode.hpp>
#include <dynarec/cop/cop0decode.hpp>
#include <Registers.hpp>

namespace n64::JIT {
void Dynarec::cop2Decode(n64::Registers& regs, u32 instr) {
  code.mov(code.rbp, (u64)&regs.cop0.status.raw);
  code.mov(code.eax, code.dword[code.rbp]);
  code.pop(code.rbp);
  code.and_(code.eax, 0x40000000);
  code.cmp(code.eax, 1);
  code.je("NoException2");

  code.mov(code.rsi, (u64)ExceptionCode::CoprocessorUnusable);
  code.mov(code.rdx, 2);
  code.mov(code.rcx, 1);
  code.mov(code.rax, (u64) FireException);
  code.call(code.rax);
  code.xor_(code.eax, code.eax);
  code.ret();

  code.L("NoException2");

  code.mov(code.rdi, (u64)this);
  code.mov(code.rsi, (u64)&regs);
  code.mov(code.rdx, (u64)instr);

  switch(RS(instr)) {
    case 0x00:
      code.mov(code.rax, (u64)mfc2);
      code.call(code.rax);
      break;
    case 0x01:
      code.mov(code.rax, (u64)dmfc2);
      code.call(code.rax);
      break;
    case 0x02: case 0x06: code.nop(); break;
    case 0x04:
      code.mov(code.rax, (u64)mtc2);
      code.call(code.rax);
      break;
    case 0x05:
      code.mov(code.rax, (u64)dmtc2);
      code.call(code.rax);
      break;
    default:
      util::panic("[RECOMPILER] Unhandled reserved instruction exception {:016X}\n", (u64)regs.pc);
  }
}

bool Dynarec::special(n64::Registers& regs, u32 instr) {
  u8 mask = (instr & 0x3F);
  bool res = false;

  // 00rr_rccc
  switch (mask) { // TODO: named constants for clearer code
    case 0:
      if (instr == 0) {
        code.nop();
      } else {
        code.mov(code.rsi, (u64)instr);
        code.mov(code.rax, (u64)sll);
        code.call(code.rax);
      }
      break;
    case 0x02:
      code.mov(code.rsi, (u64)instr);
      code.mov(code.rax, (u64)srl);
      code.call(code.rax);
      break;
    case 0x03:
      code.mov(code.rsi, (u64)instr);
      code.mov(code.rax, (u64)sra);
      code.call(code.rax);
      break;
    case 0x04:
      code.mov(code.rsi, (u64)instr);
      code.mov(code.rax, (u64)sllv);
      code.call(code.rax);
      break;
    case 0x06:
      code.mov(code.rsi, (u64)instr);
      code.mov(code.rax, (u64)srlv);
      code.call(code.rax);
      break;
    case 0x07:
      code.mov(code.rsi, (u64)instr);
      code.mov(code.rax, (u64)srav);
      code.call(code.rax);
      break;
    case 0x08:
      code.mov(code.rsi, (u64)instr);
      code.mov(code.rax, (u64)jr);
      code.call(code.rax);
      res = true;
      break;
    case 0x09:
      code.mov(code.rsi, (u64)instr);
      code.mov(code.rax, (u64)jalr);
      code.call(code.rax);
      res = true;
      break;
    case 0x0C: util::panic("[RECOMPILER] Unhandled syscall instruction {:016X}\n", (u64)regs.pc);
    case 0x0D: util::panic("[RECOMPILER] Unhandled break instruction {:016X}\n", (u64)regs.pc);
    case 0x0F: code.nop(); break; // SYNC
    case 0x10:
      code.mov(code.rsi, (u64)instr);
      code.mov(code.rax, (u64)mfhi);
      code.call(code.rax);
      break;
    case 0x11:
      code.mov(code.rsi, (u64)instr);
      code.mov(code.rax, (u64)mthi);
      code.call(code.rax);
      break;
    case 0x12:
      code.mov(code.rsi, (u64)instr);
      code.mov(code.rax, (u64)mflo);
      code.call(code.rax);
      break;
    case 0x13:
      code.mov(code.rsi, (u64)instr);
      code.mov(code.rax, (u64)mtlo);
      code.call(code.rax);
      break;
    case 0x14:
      code.mov(code.rsi, (u64)instr);
      code.mov(code.rax, (u64)dsllv);
      code.call(code.rax);
      break;
    case 0x16:
      code.mov(code.rsi, (u64)instr);
      code.mov(code.rax, (u64)dsrlv);
      code.call(code.rax);
      break;
    case 0x17:
      code.mov(code.rsi, (u64)instr);
      code.mov(code.rax, (u64)dsrav);
      code.call(code.rax);
      break;
    case 0x18:
      code.mov(code.rsi, (u64)instr);
      code.mov(code.rax, (u64)mult);
      code.call(code.rax);
      break;
    case 0x19:
      code.mov(code.rsi, (u64)instr);
      code.mov(code.rax, (u64)multu);
      code.call(code.rax);
      break;
    case 0x1A:
      code.mov(code.rsi, (u64)instr);
      code.mov(code.rax, (u64)div);
      code.call(code.rax);
      break;
    case 0x1B:
      code.mov(code.rsi, (u64)instr);
      code.mov(code.rax, (u64)divu);
      code.call(code.rax);
      break;
    case 0x1C:
      code.mov(code.rsi, (u64)instr);
      code.mov(code.rax, (u64)dmult);
      code.call(code.rax);
      break;
    case 0x1D:
      code.mov(code.rsi, (u64)instr);
      code.mov(code.rax, (u64)dmultu);
      code.call(code.rax);
      break;
    case 0x1E:
      code.mov(code.rsi, (u64)instr);
      code.mov(code.rax, (u64)ddiv);
      code.call(code.rax);
      break;
    case 0x1F:
      code.mov(code.rsi, (u64)instr);
      code.mov(code.rax, (u64)ddivu);
      code.call(code.rax);
      break;
    case 0x20:
      code.mov(code.rsi, (u64)instr);
      code.mov(code.rax, (u64)add);
      code.call(code.rax);
      break;
    case 0x21:
      code.mov(code.rsi, (u64)instr);
      code.mov(code.rax, (u64)addu);
      code.call(code.rax);
      break;
    case 0x22:
      code.mov(code.rsi, (u64)instr);
      code.mov(code.rax, (u64)sub);
      code.call(code.rax);
      break;
    case 0x23:
      code.mov(code.rsi, (u64)instr);
      code.mov(code.rax, (u64)subu);
      code.call(code.rax);
      break;
    case 0x24:
      code.mov(code.rsi, (u64)instr);
      code.mov(code.rax, (u64)and_);
      code.call(code.rax);
      break;
    case 0x25:
      code.mov(code.rsi, (u64)instr);
      code.mov(code.rax, (u64)or_);
      code.call(code.rax);
      break;
    case 0x26:
      code.mov(code.rsi, (u64)instr);
      code.mov(code.rax, (u64)xor_);
      code.call(code.rax);
      break;
    case 0x27:
      code.mov(code.rsi, (u64)instr);
      code.mov(code.rax, (u64)nor);
      code.call(code.rax);
      break;
    case 0x2A:
      code.mov(code.rsi, (u64)instr);
      code.mov(code.rax, (u64)slt);
      code.call(code.rax);
      break;
    case 0x2B:
      code.mov(code.rsi, (u64)instr);
      code.mov(code.rax, (u64)sltu);
      code.call(code.rax);
      break;
    case 0x2C:
      code.mov(code.rsi, (u64)instr);
      code.mov(code.rax, (u64)dadd);
      code.call(code.rax);
      break;
    case 0x2D:
      code.mov(code.rsi, (u64)instr);
      code.mov(code.rax, (u64)daddu);
      code.call(code.rax);
      break;
    case 0x2E:
      code.mov(code.rsi, (u64)instr);
      code.mov(code.rax, (u64)dsub);
      code.call(code.rax);
      break;
    case 0x2F:
      code.mov(code.rsi, (u64)instr);
      code.mov(code.rax, (u64)dsubu);
      code.call(code.rax);
      break;
    case 0x30:
      code.mov(code.rbp, (u64)regs.gpr);
      code.mov(code.cl, 0);
      code.mov(code.ch, 1);
      code.cmp(code.qword[RS(instr)], code.qword[RT(instr)]);
      code.cmovge(code.cl, code.ch);
      code.mov(code.rsi, code.cl.cvt64());
      code.mov(code.rax, (u64)trap);
      code.call(code.rax);
      res = true;
      break;
    case 0x31:
      code.mov(code.rbp, (u64)regs.gpr);
      code.mov(code.cl, 0);
      code.mov(code.ch, 1);
      code.cmp(code.qword[RS(instr)], code.qword[RT(instr)]);
      code.cmovae(code.cl, code.ch);
      code.mov(code.rsi, code.cl.cvt64());
      code.mov(code.rax, (u64)trap);
      code.call(code.rax);
      res = true;
      break;
    case 0x32:
      code.mov(code.rbp, (u64)regs.gpr);
      code.mov(code.cl, 0);
      code.mov(code.ch, 1);
      code.cmp(code.qword[RS(instr)], code.qword[RT(instr)]);
      code.cmovl(code.cl, code.ch);
      code.mov(code.rsi, code.cl.cvt64());
      code.mov(code.rax, (u64)trap);
      code.call(code.rax);
      res = true;
      break;
    case 0x33:
      code.mov(code.rbp, (u64)regs.gpr);
      code.mov(code.cl, 0);
      code.mov(code.ch, 1);
      code.cmp(code.qword[RS(instr)], code.qword[RT(instr)]);
      code.cmovb(code.cl, code.ch);
      code.mov(code.rsi, code.cl.cvt64());
      code.mov(code.rax, (u64)trap);
      code.call(code.rax);
      res = true;
      break;
    case 0x34:
      code.mov(code.rbp, (u64)regs.gpr);
      code.mov(code.cl, 0);
      code.mov(code.ch, 1);
      code.cmp(code.qword[RS(instr)], code.qword[RT(instr)]);
      code.cmove(code.cl, code.ch);
      code.mov(code.rsi, code.cl.cvt64());
      code.mov(code.rax, (u64)trap);
      code.call(code.rax);
      res = true;
      break;
    case 0x36:
      code.mov(code.rbp, (u64)regs.gpr);
      code.mov(code.cl, 0);
      code.mov(code.ch, 1);
      code.cmp(code.qword[RS(instr)], code.qword[RT(instr)]);
      code.cmovne(code.cl, code.ch);
      code.mov(code.rsi, code.cl.cvt64());
      code.mov(code.rax, (u64)trap);
      code.call(code.rax);
      res = true;
      break;
    case 0x38:
      code.mov(code.rsi, (u64)instr);
      code.mov(code.rax, (u64)dsll);
      code.call(code.rax);
      break;
    case 0x3A:
      code.mov(code.rsi, (u64)instr);
      code.mov(code.rax, (u64)dsrl);
      code.call(code.rax);
      break;
    case 0x3B:
      code.mov(code.rsi, (u64)instr);
      code.mov(code.rax, (u64)dsra);
      code.call(code.rax);
      break;
    case 0x3C:
      code.mov(code.rsi, (u64)instr);
      code.mov(code.rax, (u64)dsll32);
      code.call(code.rax);
      break;
    case 0x3E:
      code.mov(code.rsi, (u64)instr);
      code.mov(code.rax, (u64)dsrl32);
      code.call(code.rax);
      break;
    case 0x3F:
      code.mov(code.rsi, (u64)instr);
      code.mov(code.rax, (u64)dsra32);
      code.call(code.rax);
      break;
    default:
      util::panic("Unimplemented special {} {} ({:08X}) (pc: {:016X})\n", (mask >> 3) & 7, mask & 7, instr, (u64)regs.oldPC);
  }

  return res;
}

bool Dynarec::regimm(n64::Registers& regs, u32 instr) {
  u8 mask = ((instr >> 16) & 0x1F);
  // 000r_rccc
  switch (mask) { // TODO: named constants for clearer code
    case 0x00:
      code.mov(code.rsi, (u64)instr);
      code.mov(code.rbp, (u64)regs.gpr);
      code.mov(code.cl, 0);
      code.mov(code.ch, 1);
      code.cmp(code.qword[RS(instr)], 0);
      code.cmovl(code.cl, code.ch);
      code.mov(code.rdx, code.cl.cvt64());
      code.mov(code.rax, (u64)b);
      code.call(code.rax);
      break;
    case 0x01:
      code.mov(code.rsi, (u64)instr);
      code.mov(code.rbp, (u64)regs.gpr);
      code.mov(code.cl, 0);
      code.mov(code.ch, 1);
      code.cmp(code.qword[RS(instr)], 0);
      code.cmovge(code.cl, code.ch);
      code.mov(code.rdx, code.cl.cvt64());
      code.mov(code.rax, (u64)b);
      code.call(code.rax);
      break;
    case 0x02:
      code.mov(code.rsi, (u64)instr);
      code.mov(code.rbp, (u64)regs.gpr);
      code.mov(code.cl, 0);
      code.mov(code.ch, 1);
      code.cmp(code.qword[RS(instr)], 0);
      code.cmovl(code.cl, code.ch);
      code.mov(code.rdx, code.cl.cvt64());
      code.mov(code.rax, (u64)bl);
      code.call(code.rax);
      break;
    case 0x03:
      code.mov(code.rsi, (u64)instr);
      code.mov(code.rbp, (u64)regs.gpr);
      code.mov(code.cl, 0);
      code.mov(code.ch, 1);
      code.cmp(code.qword[RS(instr)], 0);
      code.cmovge(code.cl, code.ch);
      code.mov(code.rdx, code.cl.cvt64());
      code.mov(code.rax, (u64)bl);
      code.call(code.rax);
      break;
    case 0x08:
      code.mov(code.rbp, (u64)regs.gpr);
      code.mov(code.cl, 0);
      code.mov(code.ch, 1);
      code.cmp(code.qword[RS(instr)], s64(s16(instr)));
      code.cmovge(code.cl, code.ch);
      code.mov(code.rsi, code.cl.cvt64());
      code.mov(code.rax, (u64)trap);
      code.call(code.rax);
      break;
    case 0x09:
      code.mov(code.rbp, (u64)regs.gpr);
      code.mov(code.cl, 0);
      code.mov(code.ch, 1);
      code.cmp(code.qword[RS(instr)], u64(s64(s16(instr))));
      code.cmovae(code.cl, code.ch);
      code.mov(code.rsi, code.cl.cvt64());
      code.mov(code.rax, (u64)trap);
      code.call(code.rax);
      break;
    case 0x0A:
      code.mov(code.rbp, (u64)regs.gpr);
      code.mov(code.cl, 0);
      code.mov(code.ch, 1);
      code.cmp(code.qword[RS(instr)], s64(s16(instr)));
      code.cmovl(code.cl, code.ch);
      code.mov(code.rsi, code.cl.cvt64());
      code.mov(code.rax, (u64)trap);
      code.call(code.rax);
      break;
    case 0x0B:
      code.mov(code.rbp, (u64)regs.gpr);
      code.mov(code.cl, 0);
      code.mov(code.ch, 1);
      code.cmp(code.qword[RS(instr)], u64(s64(s16(instr))));
      code.cmovb(code.cl, code.ch);
      code.mov(code.rsi, code.cl.cvt64());
      code.mov(code.rax, (u64)trap);
      code.call(code.rax);
      break;
    case 0x0C:
      code.mov(code.rbp, (u64)regs.gpr);
      code.mov(code.cl, 0);
      code.mov(code.ch, 1);
      code.cmp(code.qword[RS(instr)], s64(s16(instr)));
      code.cmove(code.cl, code.ch);
      code.mov(code.rsi, code.cl.cvt64());
      code.mov(code.rax, (u64)trap);
      code.call(code.rax);
      break;
    case 0x0E:
      code.mov(code.rbp, (u64)regs.gpr);
      code.mov(code.cl, 0);
      code.mov(code.ch, 1);
      code.cmp(code.qword[RS(instr)], s64(s16(instr)));
      code.cmovne(code.cl, code.ch);
      code.mov(code.rsi, code.cl.cvt64());
      code.mov(code.rax, (u64)trap);
      code.call(code.rax);
      break;
    case 0x10:
      code.mov(code.rsi, (u64)instr);
      code.mov(code.rbp, (u64)regs.gpr);
      code.mov(code.cl, 0);
      code.mov(code.ch, 1);
      code.cmp(code.qword[RS(instr)], 0);
      code.cmovl(code.cl, code.ch);
      code.mov(code.rdx, code.cl.cvt64());
      code.mov(code.rax, (u64)blink);
      code.call(code.rax);
      break;
    case 0x11:
      code.mov(code.rsi, (u64)instr);
      code.mov(code.rbp, (u64)regs.gpr);
      code.mov(code.cl, 0);
      code.mov(code.ch, 1);
      code.cmp(code.qword[RS(instr)], 0);
      code.cmovge(code.cl, code.ch);
      code.mov(code.rdx, code.cl.cvt64());
      code.mov(code.rax, (u64)blink);
      code.call(code.rax);
      break;
    case 0x12:
      code.mov(code.rsi, (u64)instr);
      code.mov(code.rbp, (u64)regs.gpr);
      code.mov(code.cl, 0);
      code.mov(code.ch, 1);
      code.cmp(code.qword[RS(instr)], 0);
      code.cmovl(code.cl, code.ch);
      code.mov(code.rdx, code.cl.cvt64());
      code.mov(code.rax, (u64)bllink);
      code.call(code.rax);
      break;
    case 0x13:
      code.mov(code.rsi, (u64)instr);
      code.mov(code.rbp, (u64)regs.gpr);
      code.mov(code.cl, 0);
      code.mov(code.ch, 1);
      code.cmp(code.qword[RS(instr)], 0);
      code.cmovge(code.cl, code.ch);
      code.mov(code.rdx, code.cl.cvt64());
      code.mov(code.rax, (u64)bllink);
      code.call(code.rax);
      break;
    default:
      util::panic("Unimplemented regimm {} {} ({:08X}) (pc: {:016X})\n", (mask >> 3) & 3, mask & 7, instr, (u64)regs.oldPC);
  }

  return true;
}

bool Dynarec::Exec(n64::Registers& regs, Mem& mem, u32 instr) {
  u8 mask = (instr >> 26) & 0x3f;
  bool res = false;

  code.mov(code.rdi, (u64)&regs);
  code.push(code.rbp);
  // 00rr_rccc
  switch(mask) { // TODO: named constants for clearer code
    case 0x00: res = special(regs, instr); break;
    case 0x01: res = regimm(regs, instr); break;
    case 0x02:
      code.mov(code.rsi, (u64)instr);
      code.mov(code.rax, (u64)j);
      code.call(code.rax);
      res = true;
      break;
    case 0x03:
      code.mov(code.rsi, (u64)instr);
      code.mov(code.rax, (u64)jal);
      code.call(code.rax);
      res = true;
      break;
    case 0x04:
      code.mov(code.rsi, (u64)instr);
      code.mov(code.rbp, (u64)&regs);
      code.mov(code.cl, 0);
      code.mov(code.ch, 1);
      code.cmp(code.qword[RS(instr)], code.qword[RT(instr)]);
      code.cmove(code.cl, code.ch);
      code.mov(code.rdx, code.cl.cvt64());
      code.mov(code.rax, (u64)b);
      code.call(code.rax);
      res = true;
      break;
    case 0x05:
      code.mov(code.rsi, (u64)instr);
      code.mov(code.rbp, (u64)&regs);
      code.mov(code.r8, 0);
      code.mov(code.r9, 1);
      code.mov(code.rax, code.qword[code.rbp + offsetof(n64::Registers, gpr[RS(instr)])]);
      code.mov(code.rcx, code.qword[code.rbp + offsetof(n64::Registers, gpr[RT(instr)])]);
      code.cmp(code.rax, code.rcx);
      code.cmovne(code.r8, code.r9);
      code.mov(code.rdx, code.r8);
      code.mov(code.rax, (u64)b);
      code.call(code.rax);
      res = true;
      break;
    case 0x06:
      code.mov(code.rsi, (u64)instr);
      code.mov(code.rbp, (u64)&regs);
      code.mov(code.cl, 0);
      code.mov(code.ch, 1);
      code.cmp(code.qword[RS(instr)], 0);
      code.cmovle(code.cl, code.ch);
      code.mov(code.rdx, code.cl.cvt64());
      code.mov(code.rax, (u64)b);
      code.call(code.rax);
      res = true;
      break;
    case 0x07:
      code.mov(code.rsi, (u64)instr);
      code.mov(code.rbp, (u64)&regs);
      code.mov(code.cl, 0);
      code.mov(code.ch, 1);
      code.cmp(code.qword[RS(instr)], 0);
      code.cmovg(code.cl, code.ch);
      code.mov(code.rdx, code.cl.cvt64());
      code.mov(code.rax, (u64)b);
      code.call(code.rax);
      res = true;
      break;
    case 0x08:
      code.mov(code.rsi, (u64)instr);
      code.mov(code.rax, (u64)addi);
      code.call(code.rax);
      break;
    case 0x09:
      code.mov(code.rsi, (u64)instr);
      code.mov(code.rax, (u64)addiu);
      code.call(code.rax);
      break;
    case 0x0A:
      code.mov(code.rsi, (u64)instr);
      code.mov(code.rax, (u64)slti);
      code.call(code.rax);
      break;
    case 0x0B:
      code.mov(code.rsi, (u64)instr);
      code.mov(code.rax, (u64)sltiu);
      code.call(code.rax);
      break;
    case 0x0C:
      code.mov(code.rsi, (u64)instr);
      code.mov(code.rax, (u64)andi);
      code.call(code.rax);
      break;
    case 0x0D:
      code.mov(code.rsi, (u64)instr);
      code.mov(code.rax, (u64)ori);
      code.call(code.rax);
      break;
    case 0x0E:
      code.mov(code.rsi, (u64)instr);
      code.mov(code.rax, (u64)xori);
      code.call(code.rax);
      break;
    case 0x0F:
      code.mov(code.rsi, (u64)instr);
      code.mov(code.rax, (u64)lui);
      code.call(code.rax);
      break;
    case 0x10: cop0Decode(regs, instr, *this); break;
    case 0x11: res = cop1Decode(regs, instr, *this); break;
    case 0x12: cop2Decode(regs, instr); break;
    case 0x14:
      code.mov(code.rsi, (u64)instr);
      code.mov(code.rbp, (u64)&regs);
      code.mov(code.cl, 0);
      code.mov(code.ch, 1);
      code.cmp(code.qword[RS(instr)], code.qword[RT(instr)]);
      code.cmove(code.cl, code.ch);
      code.mov(code.rdx, code.cl.cvt64());
      code.mov(code.rax, (u64)bl);
      code.call(code.rax);
      break;
    case 0x15:
      code.mov(code.rsi, (u64)instr);
      code.mov(code.rbp, (u64)&regs);
      code.mov(code.cl, 0);
      code.mov(code.ch, 1);
      code.cmp(code.qword[RS(instr)], code.qword[RT(instr)]);
      code.cmovne(code.cl, code.ch);
      code.mov(code.rdx, code.cl.cvt64());
      code.mov(code.rax, (u64)bl);
      code.call(code.rax);
      break;
    case 0x16:
      code.mov(code.rsi, (u64)instr);
      code.mov(code.rbp, (u64)&regs);
      code.mov(code.cl, 0);
      code.mov(code.ch, 1);
      code.cmp(code.qword[RS(instr)], 0);
      code.cmovle(code.cl, code.ch);
      code.mov(code.rdx, code.cl.cvt64());
      code.mov(code.rax, (u64)bl);
      code.call(code.rax);
      break;
    case 0x17:
      code.mov(code.rsi, (u64)instr);
      code.mov(code.rbp, (u64)&regs);
      code.mov(code.cl, 0);
      code.mov(code.ch, 1);
      code.cmp(code.qword[RS(instr)], 0);
      code.cmovg(code.cl, code.ch);
      code.mov(code.rdx, code.cl.cvt64());
      code.mov(code.rax, (u64)b);
      code.call(code.rax);
      break;
    case 0x18:
      code.mov(code.rsi, (u64)instr);
      code.mov(code.rax, (u64)daddi);
      code.call(code.rax);
      break;
    case 0x19:
      code.mov(code.rsi, (u64)instr);
      code.mov(code.rax, (u64)daddiu);
      code.call(code.rax);
      break;
    case 0x1A:
      code.mov(code.rsi, (u64)&mem);
      code.mov(code.rdx, (u64)instr);
      code.mov(code.rax, (u64)ldl);
      code.call(code.rax);
      break;
    case 0x1B:
      code.mov(code.rsi, (u64)&mem);
      code.mov(code.rdx, (u64)instr);
      code.mov(code.rax, (u64)ldr);
      code.call(code.rax);
      break;
    case 0x1F: util::panic("[RECOMPILER] Unhandled reserved instruction exception {:016X}\n", regs.oldPC); break;
    case 0x20:
      code.mov(code.rsi, (u64)&mem);
      code.mov(code.rdx, (u64)instr);
      code.mov(code.rax, (u64)lb);
      code.call(code.rax);
      break;
    case 0x21:
      code.mov(code.rsi, (u64)&mem);
      code.mov(code.rdx, (u64)instr);
      code.mov(code.rax, (u64)lh);
      code.call(code.rax);
      break;
    case 0x22:
      code.mov(code.rsi, (u64)&mem);
      code.mov(code.rdx, (u64)instr);
      code.mov(code.rax, (u64)lwl);
      code.call(code.rax);
      break;
    case 0x23:
      code.mov(code.rsi, (u64)&mem);
      code.mov(code.rdx, (u64)instr);
      code.mov(code.rax, (u64)lw);
      code.call(code.rax);
      break;
    case 0x24:
      code.mov(code.rsi, (u64)&mem);
      code.mov(code.rdx, (u64)instr);
      code.mov(code.rax, (u64)lbu);
      code.call(code.rax);
      break;
    case 0x25:
      code.mov(code.rsi, (u64)&mem);
      code.mov(code.rdx, (u64)instr);
      code.mov(code.rax, (u64)lhu);
      code.call(code.rax);
      break;
    case 0x26:
      code.mov(code.rsi, (u64)&mem);
      code.mov(code.rdx, (u64)instr);
      code.mov(code.rax, (u64)lwr);
      code.call(code.rax);
      break;
    case 0x27:
      code.mov(code.rsi, (u64)&mem);
      code.mov(code.rdx, (u64)instr);
      code.mov(code.rax, (u64)lwu);
      code.call(code.rax);
      break;
    case 0x28:
      code.mov(code.rsi, (u64)&mem);
      code.mov(code.rdx, (u64)instr);
      code.mov(code.rax, (u64)sb);
      code.call(code.rax);
      break;
    case 0x29:
      code.mov(code.rsi, (u64)&mem);
      code.mov(code.rdx, (u64)instr);
      code.mov(code.rax, (u64)sh);
      code.call(code.rax);
      break;
    case 0x2A:
      code.mov(code.rsi, (u64)&mem);
      code.mov(code.rdx, (u64)instr);
      code.mov(code.rax, (u64)swl);
      code.call(code.rax);
      break;
    case 0x2B:
      code.mov(code.rsi, (u64)&mem);
      code.mov(code.rdx, (u64)instr);
      code.mov(code.rax, (u64)sw);
      code.call(code.rax);
      break;
    case 0x2C:
      code.mov(code.rsi, (u64)&mem);
      code.mov(code.rdx, (u64)instr);
      code.mov(code.rax, (u64)sdl);
      code.call(code.rax);
      break;
    case 0x2D:
      code.mov(code.rsi, (u64)&mem);
      code.mov(code.rdx, (u64)instr);
      code.mov(code.rax, (u64)sdr);
      code.call(code.rax);
      break;
    case 0x2E:
      code.mov(code.rsi, (u64)&mem);
      code.mov(code.rdx, (u64)instr);
      code.mov(code.rax, (u64)swr);
      code.call(code.rax);
      break;
    case 0x2F: code.nop(); break; // CACHE
    case 0x30:
      code.mov(code.rsi, (u64)&mem);
      code.mov(code.rdx, (u64)instr);
      code.mov(code.rax, (u64)ll);
      code.call(code.rax);
      break;
    case 0x31:
      code.mov(code.rsi, (u64)&mem);
      code.mov(code.rdx, (u64)instr);
      code.mov(code.rax, (u64)lwc1);
      code.call(code.rax);
      break;
    case 0x34:
      code.mov(code.rsi, (u64)&mem);
      code.mov(code.rdx, (u64)instr);
      code.mov(code.rax, (u64)lld);
      code.call(code.rax);
      break;
    case 0x35:
      code.mov(code.rsi, (u64)&mem);
      code.mov(code.rdx, (u64)instr);
      code.mov(code.rax, (u64)ldc1);
      code.call(code.rax);
      break;
    case 0x37:
      code.mov(code.rsi, (u64)&mem);
      code.mov(code.rdx, (u64)instr);
      code.mov(code.rax, (u64)ld);
      code.call(code.rax);
      break;
    case 0x38:
      code.mov(code.rsi, (u64)&mem);
      code.mov(code.rdx, (u64)instr);
      code.mov(code.rax, (u64)sc);
      code.call(code.rax);
      break;
    case 0x39:
      code.mov(code.rsi, (u64)&mem);
      code.mov(code.rdx, (u64)instr);
      code.mov(code.rax, (u64)swc1);
      code.call(code.rax);
      break;
    case 0x3C:
      code.mov(code.rsi, (u64)&mem);
      code.mov(code.rdx, (u64)instr);
      code.mov(code.rax, (u64)scd);
      code.call(code.rax);
      break;
    case 0x3D:
      code.mov(code.rsi, (u64)&mem);
      code.mov(code.rdx, (u64)instr);
      code.mov(code.rax, (u64)sdc1);
      code.call(code.rax);
      break;
    case 0x3F:
      code.mov(code.rsi, (u64)&mem);
      code.mov(code.rdx, (u64)instr);
      code.mov(code.rax, (u64)sd);
      code.call(code.rax);
      break;
    default:
      util::panic("Unimplemented instruction {:02X} ({:08X}) (pc: {:016X})\n", mask, instr, (u64)regs.oldPC);
  }

  code.pop(code.rbp);
  return res;
}
}
