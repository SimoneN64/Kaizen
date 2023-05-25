#include <core/CachedInterpreter.hpp>
#include <log.hpp>

namespace n64 {
auto CachedInterpreter::special(u32 instr) {
  u8 mask = (instr & 0x3F);
  // 00rr_rccc
  switch (mask) { // TODO: named constants for clearer code
    case 0:
      if (instr != 0) {
        sll(instr);
      }
      break;
    case 0x02: return srl(instr);
    case 0x03: return sra(instr);
    case 0x04: return sllv(instr);
    case 0x06: return srlv(instr);
    case 0x07: return srav(instr);
    case 0x08: return jr(instr);
    case 0x09: return jalr(instr);
    case 0x0C: return FireException(regs, ExceptionCode::Syscall, 0, true);
    case 0x0D: return FireException(regs, ExceptionCode::Breakpoint, 0, true);
    case 0x0F: break; // SYNC
    case 0x10: return mfhi(instr);
    case 0x11: return mthi(instr);
    case 0x12: return mflo(instr);
    case 0x13: return mtlo(instr);
    case 0x14: return dsllv(instr);
    case 0x16: return dsrlv(instr);
    case 0x17: return dsrav(instr);
    case 0x18: return mult(instr);
    case 0x19: return multu(instr);
    case 0x1A: return div(instr);
    case 0x1B: return divu(instr);
    case 0x1C: return dmult(instr);
    case 0x1D: return dmultu(instr);
    case 0x1E: return ddiv(instr);
    case 0x1F: return ddivu(instr);
    case 0x20: return add(instr);
    case 0x21: return addu(instr);
    case 0x22: return sub(instr);
    case 0x23: return subu(instr);
    case 0x24: return and_(instr);
    case 0x25: return or_(instr);
    case 0x26: return xor_(instr);
    case 0x27: return nor(instr);
    case 0x2A: return slt(instr);
    case 0x2B: return sltu(instr);
    case 0x2C: return dadd(instr);
    case 0x2D: return daddu(instr);
    case 0x2E: return dsub(instr);
    case 0x2F: return dsubu(instr);
    case 0x30: return trap(regs.gpr[RS(instr)] >= regs.gpr[RT(instr)]);
    case 0x31: return trap((u64)regs.gpr[RS(instr)] >= (u64)regs.gpr[RT(instr)]);
    case 0x32: return trap(regs.gpr[RS(instr)] < regs.gpr[RT(instr)]);
    case 0x33: return trap((u64)regs.gpr[RS(instr)] < (u64)regs.gpr[RT(instr)]);
    case 0x34: return trap(regs.gpr[RS(instr)] == regs.gpr[RT(instr)]);
    case 0x36: return trap(regs.gpr[RS(instr)] != regs.gpr[RT(instr)]);
    case 0x38: return dsll(instr);
    case 0x3A: return dsrl(instr);
    case 0x3B: return dsra(instr);
    case 0x3C: return dsll32(instr);
    case 0x3E: return dsrl32(instr);
    case 0x3F: return dsra32(instr);
    default:
      Util::panic("Unimplemented special {} {} ({:08X}) (pc: {:016X})", (mask >> 3) & 7, mask & 7, instr, (u64)regs.oldPC);
  }
}

auto CachedInterpreter::regimm(u32 instr) {
  u8 mask = ((instr >> 16) & 0x1F);
  // 000r_rccc
  switch (mask) { // TODO: named constants for clearer code
    case 0x00: return b(instr, regs.gpr[RS(instr)] < 0);
    case 0x01: return b(instr, regs.gpr[RS(instr)] >= 0);
    case 0x02: return bl(instr, regs.gpr[RS(instr)] < 0);
    case 0x03: return bl(instr, regs.gpr[RS(instr)] >= 0);
    case 0x08: return trap(regs.gpr[RS(instr)] >= s64(s16(instr)));
    case 0x09: return trap(u64(regs.gpr[RS(instr)]) >= u64(s64(s16(instr))));
    case 0x0A: return trap(regs.gpr[RS(instr)] < s64(s16(instr)));
    case 0x0B: return trap(u64(regs.gpr[RS(instr)]) < u64(s64(s16(instr))));
    case 0x0C: return trap(regs.gpr[RS(instr)] == s64(s16(instr)));
    case 0x0E: return trap(regs.gpr[RS(instr)] != s64(s16(instr)));
    case 0x10: return blink(instr, regs.gpr[RS(instr)] < 0);
    case 0x11: return blink(instr, regs.gpr[RS(instr)] >= 0);
    case 0x12: return bllink(instr, regs.gpr[RS(instr)] < 0);
    case 0x13: return bllink(instr, regs.gpr[RS(instr)] >= 0);
    default:
      Util::panic("Unimplemented regimm {} {} ({:08X}) (pc: {:016X})", (mask >> 3) & 3, mask & 7, instr, (u64)regs.oldPC);
  }
}

auto CachedInterpreter::cop2Decode(u32 instr) {
  if(!regs.cop0.status.cu2) {
    FireException(regs, ExceptionCode::CoprocessorUnusable, 2, true);
    return;
  }
  switch(RS(instr)) {
    case 0x00: return mfc2(instr);
    case 0x01: return dmfc2(instr);
    case 0x02: return cfc2(instr);
    case 0x04: return mtc2(instr);
    case 0x05: return dmtc2(instr);
    case 0x06: return ctc2(instr);
    default:
      FireException(regs, ExceptionCode::ReservedInstruction, 2, true);
  }
}

auto CachedInterpreter::GetInstrFunc(u32 instr) {
  u8 mask = (instr >> 26) & 0x3f;
  // 00rr_rccc
  switch(mask) { // TODO: named constants for clearer code
    case 0x00: return special(instr);
    case 0x01: return regimm(instr);
    case 0x02: return j(instr);
    case 0x03: return jal(instr);
    case 0x04: return b(instr, regs.gpr[RS(instr)] == regs.gpr[RT(instr)]);
    case 0x05: return b(instr, regs.gpr[RS(instr)] != regs.gpr[RT(instr)]);
    case 0x06: return b(instr, regs.gpr[RS(instr)] <= 0);
    case 0x07: return b(instr, regs.gpr[RS(instr)] > 0);
    case 0x08: return addi(instr);
    case 0x09: return addiu(instr);
    case 0x0A: return slti(instr);
    case 0x0B: return sltiu(instr);
    case 0x0C: return andi(instr);
    case 0x0D: return ori(instr);
    case 0x0E: return xori(instr);
    case 0x0F: return lui(instr);
    case 0x10: return regs.cop0.decode(regs, instr);
    case 0x11: return regs.cop1.decode(regs, *this, instr);
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