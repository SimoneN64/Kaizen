#include <core/Interpreter.hpp>
#include <log.hpp>

namespace n64 {
void Interpreter::special(Registers& regs, u32 instr) {
  u8 mask = (instr & 0x3F);
  // 00rr_rccc
  switch (mask) { // TODO: named constants for clearer code
    case 0:
      if (instr != 0) {
        sll(regs, instr);
      }
      break;
    case 0x02: srl(regs, instr); break;
    case 0x03: sra(regs, instr); break;
    case 0x04: sllv(regs, instr); break;
    case 0x06: srlv(regs, instr); break;
    case 0x07: srav(regs, instr); break;
    case 0x08: jr(regs, instr); break;
    case 0x09: jalr(regs, instr); break;
    case 0x0C: FireException(regs, ExceptionCode::Syscall, 0, true); break;
    case 0x0D: FireException(regs, ExceptionCode::Breakpoint, 0, true); break;
    case 0x0F: break; // SYNC
    case 0x10: mfhi(regs, instr); break;
    case 0x11: mthi(regs, instr); break;
    case 0x12: mflo(regs, instr); break;
    case 0x13: mtlo(regs, instr); break;
    case 0x14: dsllv(regs, instr); break;
    case 0x16: dsrlv(regs, instr); break;
    case 0x17: dsrav(regs, instr); break;
    case 0x18: mult(regs, instr); break;
    case 0x19: multu(regs, instr); break;
    case 0x1A: div(regs, instr); break;
    case 0x1B: divu(regs, instr); break;
    case 0x1C: dmult(regs, instr); break;
    case 0x1D: dmultu(regs, instr); break;
    case 0x1E: ddiv(regs, instr); break;
    case 0x1F: ddivu(regs, instr); break;
    case 0x20: add(regs, instr); break;
    case 0x21: addu(regs, instr); break;
    case 0x22: sub(regs, instr); break;
    case 0x23: subu(regs, instr); break;
    case 0x24: and_(regs, instr); break;
    case 0x25: or_(regs, instr); break;
    case 0x26: xor_(regs, instr); break;
    case 0x27: nor(regs, instr); break;
    case 0x2A: slt(regs, instr); break;
    case 0x2B: sltu(regs, instr); break;
    case 0x2C: dadd(regs, instr); break;
    case 0x2D: daddu(regs, instr); break;
    case 0x2E: dsub(regs, instr); break;
    case 0x2F: dsubu(regs, instr); break;
    case 0x30: trap(regs, regs.gpr[RS(instr)] >= regs.gpr[RT(instr)]); break;
    case 0x31: trap(regs, (u64)regs.gpr[RS(instr)] >= (u64)regs.gpr[RT(instr)]); break;
    case 0x32: trap(regs, regs.gpr[RS(instr)] < regs.gpr[RT(instr)]); break;
    case 0x33: trap(regs, (u64)regs.gpr[RS(instr)] < (u64)regs.gpr[RT(instr)]); break;
    case 0x34: trap(regs, regs.gpr[RS(instr)] == regs.gpr[RT(instr)]); break;
    case 0x36: trap(regs, regs.gpr[RS(instr)] != regs.gpr[RT(instr)]); break;
    case 0x38: dsll(regs, instr); break;
    case 0x3A: dsrl(regs, instr); break;
    case 0x3B: dsra(regs, instr); break;
    case 0x3C: dsll32(regs, instr); break;
    case 0x3E: dsrl32(regs, instr); break;
    case 0x3F: dsra32(regs, instr); break;
    default:
      Util::panic("Unimplemented special {} {} ({:08X}) (pc: {:016X})\n", (mask >> 3) & 7, mask & 7, instr, (u64)regs.oldPC);
  }
}

void Interpreter::regimm(Registers& regs, u32 instr) {
  u8 mask = ((instr >> 16) & 0x1F);
  // 000r_rccc
  switch (mask) { // TODO: named constants for clearer code
    case 0x00: b(regs, instr, regs.gpr[RS(instr)] < 0); break;
    case 0x01: b(regs, instr, regs.gpr[RS(instr)] >= 0); break;
    case 0x02: bl(regs, instr, regs.gpr[RS(instr)] < 0); break;
    case 0x03: bl(regs, instr, regs.gpr[RS(instr)] >= 0); break;
    case 0x08: trap(regs, regs.gpr[RS(instr)] >= s64(s16(instr))); break;
    case 0x09: trap(regs, u64(regs.gpr[RS(instr)]) >= u64(s64(s16(instr)))); break;
    case 0x0A: trap(regs, regs.gpr[RS(instr)] < s64(s16(instr))); break;
    case 0x0B: trap(regs, u64(regs.gpr[RS(instr)]) < u64(s64(s16(instr)))); break;
    case 0x0C: trap(regs, regs.gpr[RS(instr)] == s64(s16(instr))); break;
    case 0x0E: trap(regs, regs.gpr[RS(instr)] != s64(s16(instr))); break;
    case 0x10: blink(regs, instr, regs.gpr[RS(instr)] < 0); break;
    case 0x11: blink(regs, instr, regs.gpr[RS(instr)] >= 0); break;
    case 0x12: bllink(regs, instr, regs.gpr[RS(instr)] < 0); break;
    case 0x13: bllink(regs, instr, regs.gpr[RS(instr)] >= 0); break;
    default:
      Util::panic("Unimplemented regimm {} {} ({:08X}) (pc: {:016X})\n", (mask >> 3) & 3, mask & 7, instr, (u64)regs.oldPC);
  }
}

void Interpreter::cop2Decode(Registers& regs, u32 instr) {
  if(!regs.cop0.status.cu2) {
    FireException(regs, ExceptionCode::CoprocessorUnusable, 2, true);
    return;
  }
  switch(RS(instr)) {
    case 0x00: mfc2(regs, instr); break;
    case 0x01: dmfc2(regs, instr); break;
    case 0x02: cfc2(regs, instr); break;
    case 0x04: mtc2(regs, instr); break;
    case 0x05: dmtc2(regs, instr); break;
    case 0x06: ctc2(regs, instr); break;
    default:
      FireException(regs, ExceptionCode::ReservedInstruction, 2, true);
  }
}

void Interpreter::Exec(Registers& regs, Mem& mem, u32 instr) {
  u8 mask = (instr >> 26) & 0x3f;
  // 00rr_rccc
  switch(mask) { // TODO: named constants for clearer code
    case 0x00: special(regs, instr); break;
    case 0x01: regimm(regs, instr); break;
    case 0x02: j(regs, instr); break;
    case 0x03: jal(regs, instr); break;
    case 0x04: b(regs, instr, regs.gpr[RS(instr)] == regs.gpr[RT(instr)]); break;
    case 0x05: {
      //fmt::print("RS: {:016X}, RT: {:016X}\n", (u64)regs.gpr[RS(instr)], (u64)regs.gpr[RT(instr)]);
      b(regs, instr, regs.gpr[RS(instr)] != regs.gpr[RT(instr)]);
    } break;
    case 0x06: b(regs, instr, regs.gpr[RS(instr)] <= 0); break;
    case 0x07: b(regs, instr, regs.gpr[RS(instr)] > 0); break;
    case 0x08: addi(regs, instr); break;
    case 0x09: addiu(regs, instr); break;
    case 0x0A: slti(regs, instr); break;
    case 0x0B: sltiu(regs, instr); break;
    case 0x0C: andi(regs, instr); break;
    case 0x0D: ori(regs, instr); break;
    case 0x0E: xori(regs, instr); break;
    case 0x0F: lui(regs, instr); break;
    case 0x10: regs.cop0.decode(regs, mem, instr); break;
    case 0x11: regs.cop1.decode(regs, *this, instr); break;
    case 0x12: cop2Decode(regs, instr); break;
    case 0x14: bl(regs, instr, regs.gpr[RS(instr)] == regs.gpr[RT(instr)]); break;
    case 0x15: bl(regs, instr, regs.gpr[RS(instr)] != regs.gpr[RT(instr)]); break;
    case 0x16: bl(regs, instr, regs.gpr[RS(instr)] <= 0); break;
    case 0x17: bl(regs, instr, regs.gpr[RS(instr)] > 0); break;
    case 0x18: daddi(regs, instr); break;
    case 0x19: daddiu(regs, instr); break;
    case 0x1A: ldl(regs, mem, instr); break;
    case 0x1B: ldr(regs, mem, instr); break;
    case 0x1F: FireException(regs, ExceptionCode::ReservedInstruction, 0, true); break;
    case 0x20: lb(regs, mem, instr); break;
    case 0x21: lh(regs, mem, instr); break;
    case 0x22: lwl(regs, mem, instr); break;
    case 0x23: lw(regs, mem, instr); break;
    case 0x24: lbu(regs, mem, instr); break;
    case 0x25: lhu(regs, mem, instr); break;
    case 0x26: lwr(regs, mem, instr); break;
    case 0x27: lwu(regs, mem, instr); break;
    case 0x28: sb(regs, mem, instr); break;
    case 0x29: sh(regs, mem, instr); break;
    case 0x2A: swl(regs, mem, instr); break;
    case 0x2B: sw(regs, mem, instr); break;
    case 0x2C: sdl(regs, mem, instr); break;
    case 0x2D: sdr(regs, mem, instr); break;
    case 0x2E: swr(regs, mem, instr); break;
    case 0x2F: break; // CACHE
    case 0x30: ll(regs, mem, instr); break;
    case 0x31: regs.cop1.lwc1(regs, mem, instr); break;
    case 0x34: lld(regs, mem, instr); break;
    case 0x35: regs.cop1.ldc1(regs, mem, instr); break;
    case 0x37: ld(regs, mem, instr); break;
    case 0x38: sc(regs, mem, instr); break;
    case 0x39: regs.cop1.swc1(regs, mem, instr); break;
    case 0x3C: scd(regs, mem, instr); break;
    case 0x3D: regs.cop1.sdc1(regs, mem, instr); break;
    case 0x3F: sd(regs, mem, instr); break;
    default:
      Util::panic("Unimplemented instruction {:02X} ({:08X}) (pc: {:016X})\n", mask, instr, (u64)regs.oldPC);
  }
}
}