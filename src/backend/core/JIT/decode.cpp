#include <core/JIT.hpp>
#include <log.hpp>

namespace n64 {
void JIT::special(u32 instr) {
  u8 mask = (instr & 0x3F);
  // 00rr_rccc
  switch (mask) { // TODO: named constants for clearer code
    case SLL:
      if (instr != 0) {
        sll(instr);
      }
      break;
    case SRL: srl(instr); break;
    case SRA: sra(instr); break;
    case SLLV: sllv(instr); break;
    case SRLV: srlv(instr); break;
    case SRAV: srav(instr); break;
    case JR: jr(instr); break;
    case JALR: jalr(instr); break;
    case SYSCALL: FireException(regs, ExceptionCode::Syscall, 0, regs.oldPC); break;
    case BREAK: FireException(regs, ExceptionCode::Breakpoint, 0, regs.oldPC); break;
    case SYNC: break; // SYNC
    case MFHI: mfhi(instr); break;
    case MTHI: mthi(instr); break;
    case MFLO: mflo(instr); break;
    case MTLO: mtlo(instr); break;
    case DSLLV: dsllv(instr); break;
    case DSRLV: dsrlv(instr); break;
    case DSRAV: dsrav(instr); break;
    case MULT: mult(instr); break;
    case MULTU: multu(instr); break;
    case DIV: div(instr); break;
    case DIVU: divu(instr); break;
    case DMULT: dmult(instr); break;
    case DMULTU: dmultu(instr); break;
    case DDIV: ddiv(instr); break;
    case DDIVU: ddivu(instr); break;
    case ADD: add(instr); break;
    case ADDU: addu(instr); break;
    case SUB: sub(instr); break;
    case SUBU: subu(instr); break;
    case AND: and_(instr); break;
    case OR: or_(instr); break;
    case XOR: xor_(instr); break;
    case NOR: nor(instr); break;
    case SLT: slt(instr); break;
    case SLTU: sltu(instr); break;
    case DADD: dadd(instr); break;
    case DADDU: daddu(instr); break;
    case DSUB: dsub(instr); break;
    case DSUBU: dsubu(instr); break;
    case TGE: tge(instr); break;
    case TGEU: tgeu(instr); break;
    case TLT: tlt(instr); break;
    case TLTU: tltu(instr); break;
    case TEQ: teq(instr); break;
    case TNE: tne(instr); break;
    case DSLL: dsll(instr); break;
    case DSRL: dsrl(instr); break;
    case DSRA: dsra(instr); break;
    case DSLL32: dsll32(instr); break;
    case DSRL32: dsrl32(instr); break;
    case DSRA32: dsra32(instr); break;
    default:
      Util::panic("Unimplemented special {} {} ({:08X}) (pc: {:016X})", (mask >> 3) & 7, mask & 7, instr, (u64)regs.oldPC);
  }
}

void JIT::regimm(u32 instr) {
  u8 mask = ((instr >> 16) & 0x1F);
  // 000r_rccc
  switch (mask) { // TODO: named constants for clearer code
    case BLTZ: bltz(instr); break;
    case BGEZ: bgez(instr); break;
    case BLTZL: bltzl(instr); break;
    case BGEZL: bgezl(instr); break;
    case TGEI: tgei(instr); break;
    case TGEIU: tgeiu(instr); break;
    case TLTI: tlti(instr); break;
    case TLTIU: tltiu(instr); break;
    case TEQI: teqi(instr); break;
    case TNEI: tnei(instr); break;
    case BLTZAL: bltzal(instr); break;
    case BGEZAL: bgezal(instr); break;
    case BLTZALL: bltzall(instr); break;
    case BGEZALL: bgezall(instr); break;
    default:
      Util::panic("Unimplemented regimm {} {} ({:08X}) (pc: {:016X})", (mask >> 3) & 3, mask & 7, instr, (u64)regs.oldPC);
  }
}

void JIT::cop2Decode(u32 instr) {
  if(!regs.cop0.status.cu2) {
    FireException(regs, ExceptionCode::CoprocessorUnusable, 2, regs.oldPC);
    return;
  }
  switch(RS(instr)) {
    case 0x00: mfc2(instr); break;
    case 0x01: dmfc2(instr); break;
    case 0x02: cfc2(instr); break;
    case 0x04: mtc2(instr); break;
    case 0x05: dmtc2(instr); break;
    case 0x06: ctc2(instr); break;
    default:
      FireException(regs, ExceptionCode::ReservedInstruction, 2, regs.oldPC);
  }
}

void JIT::Emit(u32 instr) {
  u8 mask = (instr >> 26) & 0x3f;
  // 00rr_rccc
  switch(mask) { // TODO: named constants for clearer code
    case SPECIAL: special(instr); break;
    case REGIMM: regimm(instr); break;
    case J: j(instr); break;
    case JAL: jal(instr); break;
    case BEQ: beq(instr); break;
    case BNE: bne(instr); break;
    case BLEZ: blez(instr); break;
    case BGTZ: bgtz(instr); break;
    case ADDI: addi(instr); break;
    case ADDIU: addiu(instr); break;
    case SLTI: slti(instr); break;
    case SLTIU: sltiu(instr); break;
    case ANDI: andi(instr); break;
    case ORI: ori(instr); break;
    case XORI: xori(instr); break;
    case LUI: lui(instr); break;
    case COP0: regs.cop0.decode(*this, instr); break;
    case COP1: regs.cop1.decode(*this, instr); break;
    case COP2: cop2Decode(instr); break;
    case BEQL: beql(instr); break;
    case BNEL: bnel(instr); break;
    case BLEZL: blezl(instr); break;
    case BGTZL: bgtzl(instr); break;
    case DADDI: daddi(instr); break;
    case DADDIU: daddiu(instr); break;
    case LDL: ldl(instr); break;
    case LDR: ldr(instr); break;
    case 0x1F: FireException(regs, ExceptionCode::ReservedInstruction, 0, regs.oldPC); break;
    case LB: lb(instr); break;
    case LH: lh(instr); break;
    case LWL: lwl(instr); break;
    case LW: lw(instr); break;
    case LBU: lbu(instr); break;
    case LHU: lhu(instr); break;
    case LWR: lwr(instr); break;
    case LWU: lwu(instr); break;
    case SB: sb(instr); break;
    case SH: sh(instr); break;
    case SWL: swl(instr); break;
    case SW: sw(instr); break;
    case SDL: sdl(instr); break;
    case SDR: sdr(instr); break;
    case SWR: swr(instr); break;
    case CACHE: break; // CACHE
    case LL: ll(instr); break;
    case LWC1: regs.cop1.lwc1(*this, mem, instr); break;
    case LLD: lld(instr); break;
    case LDC1: regs.cop1.ldc1(*this, mem, instr); break;
    case LD: ld(instr); break;
    case SC: sc(instr); break;
    case SWC1: regs.cop1.swc1(*this, mem, instr); break;
    case SCD: scd(instr); break;
    case SDC1: regs.cop1.sdc1(*this, mem, instr); break;
    case SD: sd(instr); break;
    default:
      Util::panic("Unimplemented instruction {:02X} ({:08X}) (pc: {:016X})", mask, instr, (u64)regs.oldPC);
  }
}
}
