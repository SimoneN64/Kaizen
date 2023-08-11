#include <core/JIT.hpp>
#include <log.hpp>

namespace n64 {
void JIT::special(u32 instr) {
  u8 mask = (instr & 0x3F);
  // 00rr_rccc
  switch (mask) { // TODO: named constants for clearer code
    case 0:
      if (instr != 0) {
        sll(instr);
      }
      break;
    case 0x02: srl(instr); break;
    case 0x03: sra(instr); break;
    case 0x04: sllv(instr); break;
    case 0x06: srlv(instr); break;
    case 0x07: srav(instr); break;
    case 0x08: jr(instr); break;
    case 0x09: jalr(instr); break;
    case 0x0C: FireException(regs, ExceptionCode::Syscall, 0, true); break;
    case 0x0D: FireException(regs, ExceptionCode::Breakpoint, 0, true); break;
    case 0x0F: break; // SYNC
    case 0x10: mfhi(instr); break;
    case 0x11: mthi(instr); break;
    case 0x12: mflo(instr); break;
    case 0x13: mtlo(instr); break;
    case 0x14: dsllv(instr); break;
    case 0x16: dsrlv(instr); break;
    case 0x17: dsrav(instr); break;
    case 0x18: mult(instr); break;
    case 0x19: multu(instr); break;
    case 0x1A: div(instr); break;
    case 0x1B: divu(instr); break;
    case 0x1C: dmult(instr); break;
    case 0x1D: dmultu(instr); break;
    case 0x1E: ddiv(instr); break;
    case 0x1F: ddivu(instr); break;
    case 0x20: add(instr); break;
    case 0x21: addu(instr); break;
    case 0x22: sub(instr); break;
    case 0x23: subu(instr); break;
    case 0x24: and_(instr); break;
    case 0x25: or_(instr); break;
    case 0x26: xor_(instr); break;
    case 0x27: nor(instr); break;
    case 0x2A: slt(instr); break;
    case 0x2B: sltu(instr); break;
    case 0x2C: dadd(instr); break;
    case 0x2D: daddu(instr); break;
    case 0x2E: dsub(instr); break;
    case 0x2F: dsubu(instr); break;
    case 0x30: trap(regs.gpr[RS(instr)] >= regs.gpr[RT(instr)]); break;
    case 0x31: trap((u64)regs.gpr[RS(instr)] >= (u64)regs.gpr[RT(instr)]); break;
    case 0x32: trap(regs.gpr[RS(instr)] < regs.gpr[RT(instr)]); break;
    case 0x33: trap((u64)regs.gpr[RS(instr)] < (u64)regs.gpr[RT(instr)]); break;
    case 0x34: trap(regs.gpr[RS(instr)] == regs.gpr[RT(instr)]); break;
    case 0x36: trap(regs.gpr[RS(instr)] != regs.gpr[RT(instr)]); break;
    case 0x38: dsll(instr); break;
    case 0x3A: dsrl(instr); break;
    case 0x3B: dsra(instr); break;
    case 0x3C: dsll32(instr); break;
    case 0x3E: dsrl32(instr); break;
    case 0x3F: dsra32(instr); break;
    default:
      Util::panic("Unimplemented special {} {} ({:08X}) (pc: {:016X})", (mask >> 3) & 7, mask & 7, instr, (u64)regs.oldPC);
  }
}

void JIT::regimm(u32 instr) {
  u8 mask = ((instr >> 16) & 0x1F);
  // 000r_rccc
  switch (mask) { // TODO: named constants for clearer code
    case 0x00: b(instr, regs.gpr[RS(instr)] < 0); break;
    case 0x01: b(instr, regs.gpr[RS(instr)] >= 0); break;
    case 0x02: bl(instr, regs.gpr[RS(instr)] < 0); break;
    case 0x03: bl(instr, regs.gpr[RS(instr)] >= 0); break;
    case 0x08: trap(regs.gpr[RS(instr)] >= s64(s16(instr))); break;
    case 0x09: trap(u64(regs.gpr[RS(instr)]) >= u64(s64(s16(instr)))); break;
    case 0x0A: trap(regs.gpr[RS(instr)] < s64(s16(instr))); break;
    case 0x0B: trap(u64(regs.gpr[RS(instr)]) < u64(s64(s16(instr)))); break;
    case 0x0C: trap(regs.gpr[RS(instr)] == s64(s16(instr))); break;
    case 0x0E: trap(regs.gpr[RS(instr)] != s64(s16(instr))); break;
    case 0x10: blink(instr, regs.gpr[RS(instr)] < 0); break;
    case 0x11: blink(instr, regs.gpr[RS(instr)] >= 0); break;
    case 0x12: bllink(instr, regs.gpr[RS(instr)] < 0); break;
    case 0x13: bllink(instr, regs.gpr[RS(instr)] >= 0); break;
    default:
      Util::panic("Unimplemented regimm {} {} ({:08X}) (pc: {:016X})", (mask >> 3) & 3, mask & 7, instr, (u64)regs.oldPC);
  }
}

void JIT::cop2Decode(u32 instr) {
  if(!regs.cop0.status.cu2) {
    FireException(regs, ExceptionCode::CoprocessorUnusable, 2, true);
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
      FireException(regs, ExceptionCode::ReservedInstruction, 2, true);
  }
}

void JIT::Emit(u32 instr) {
  u8 mask = (instr >> 26) & 0x3f;
  // 00rr_rccc
  switch(mask) { // TODO: named constants for clearer code
    case 0x00: special(instr); break;
    case 0x01: regimm(instr); break;
    case 0x02: j(instr); break;
    case 0x03: jal(instr); break;
    case 0x04: b(instr, regs.gpr[RS(instr)] == regs.gpr[RT(instr)]); break;
    case 0x05: {
      //fmt::print("RS: {:016X}, RT: {:016X}", (u64)regs.gpr[RS(instr)], (u64)regs.gpr[RT(instr)]);
      b(instr, regs.gpr[RS(instr)] != regs.gpr[RT(instr)]);
    } break;
    case 0x06: b(instr, regs.gpr[RS(instr)] <= 0); break;
    case 0x07: b(instr, regs.gpr[RS(instr)] > 0); break;
    case 0x08: addi(instr); break;
    case 0x09: addiu(instr); break;
    case 0x0A: slti(instr); break;
    case 0x0B: sltiu(instr); break;
    case 0x0C: andi(instr); break;
    case 0x0D: ori(instr); break;
    case 0x0E: xori(instr); break;
    case 0x0F: lui(instr); break;
    case 0x10: regs.cop0.decode(*this, instr); break;
    case 0x11: regs.cop1.decode(*this, instr); break;
    case 0x12: cop2Decode(instr); break;
    case 0x14: bl(instr, regs.gpr[RS(instr)] == regs.gpr[RT(instr)]); break;
    case 0x15: bl(instr, regs.gpr[RS(instr)] != regs.gpr[RT(instr)]); break;
    case 0x16: bl(instr, regs.gpr[RS(instr)] <= 0); break;
    case 0x17: bl(instr, regs.gpr[RS(instr)] > 0); break;
    case 0x18: daddi(instr); break;
    case 0x19: daddiu(instr); break;
    case 0x1A: ldl(instr); break;
    case 0x1B: ldr(instr); break;
    case 0x1F: FireException(regs, ExceptionCode::ReservedInstruction, 0, true); break;
    case 0x20: lb(instr); break;
    case 0x21: lh(instr); break;
    case 0x22: lwl(instr); break;
    case 0x23: lw(instr); break;
    case 0x24: lbu(instr); break;
    case 0x25: lhu(instr); break;
    case 0x26: lwr(instr); break;
    case 0x27: lwu(instr); break;
    case 0x28: sb(instr); break;
    case 0x29: sh(instr); break;
    case 0x2A: swl(instr); break;
    case 0x2B: sw(instr); break;
    case 0x2C: sdl(instr); break;
    case 0x2D: sdr(instr); break;
    case 0x2E: swr(instr); break;
    case 0x2F: break; // CACHE
    case 0x30: ll(instr); break;
    case 0x31: regs.cop1.lwc1(regs, mem, instr); break;
    case 0x34: lld(instr); break;
    case 0x35: regs.cop1.ldc1(regs, mem, instr); break;
    case 0x37: ld(instr); break;
    case 0x38: sc(instr); break;
    case 0x39: regs.cop1.swc1(regs, mem, instr); break;
    case 0x3C: scd(instr); break;
    case 0x3D: regs.cop1.sdc1(regs, mem, instr); break;
    case 0x3F: sd(instr); break;
    default:
      Util::panic("Unimplemented instruction {:02X} ({:08X}) (pc: {:016X})", mask, instr, (u64)regs.oldPC);
  }
}
}
