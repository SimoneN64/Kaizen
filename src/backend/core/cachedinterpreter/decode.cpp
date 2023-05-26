#include <CachedInterpreter.hpp>
#include <cachedinterpreter/instructions.hpp>
#include <cachedinterpreter/cop/cop0decode.hpp>
#include <cachedinterpreter/cop/cop1decode.hpp>
#include <log.hpp>

namespace n64 {
CachedFn CachedInterpreter::special(u32 instr) {
  u8 mask = (instr & 0x3F);
  // 00rr_rccc
  switch (mask) { // TODO: named constants for clearer code
    case 0:
      if (instr != 0) {
        return sll;
      }
      return [](CachedInterpreter&, u32){};
    case 0x02: return srl;
    case 0x03: return sra;
    case 0x04: return sllv;
    case 0x06: return srlv;
    case 0x07: return srav;
    case 0x08: return jr;
    case 0x09: return jalr;
    case 0x0C: return [](CachedInterpreter& cpu, u32) {
      FireException(cpu.regs, ExceptionCode::Syscall, 0, true);
    };
    case 0x0D: return [](CachedInterpreter& cpu, u32) {
      FireException(cpu.regs, ExceptionCode::Breakpoint, 0, true);
    };
    case 0x0F: break; // SYNC
    case 0x10: return mfhi;
    case 0x11: return mthi;
    case 0x12: return mflo;
    case 0x13: return mtlo;
    case 0x14: return dsllv;
    case 0x16: return dsrlv;
    case 0x17: return dsrav;
    case 0x18: return mult;
    case 0x19: return multu;
    case 0x1A: return div;
    case 0x1B: return divu;
    case 0x1C: return dmult;
    case 0x1D: return dmultu;
    case 0x1E: return ddiv;
    case 0x1F: return ddivu;
    case 0x20: return add;
    case 0x21: return addu;
    case 0x22: return sub;
    case 0x23: return subu;
    case 0x24: return and_;
    case 0x25: return or_;
    case 0x26: return xor_;
    case 0x27: return nor;
    case 0x2A: return slt;
    case 0x2B: return sltu;
    case 0x2C: return dadd;
    case 0x2D: return daddu;
    case 0x2E: return dsub;
    case 0x2F: return dsubu;
    case 0x30: return [](CachedInterpreter& cpu, u32 instr) { trap(cpu, cpu.regs.gpr[RS(instr)] >= cpu.regs.gpr[RT(instr)]); };
    case 0x31: return [](CachedInterpreter& cpu, u32 instr) { trap(cpu, (u64)cpu.regs.gpr[RS(instr)] >= (u64)cpu.regs.gpr[RT(instr)]); };
    case 0x32: return [](CachedInterpreter& cpu, u32 instr) { trap(cpu, cpu.regs.gpr[RS(instr)] < cpu.regs.gpr[RT(instr)]); };
    case 0x33: return [](CachedInterpreter& cpu, u32 instr) { trap(cpu, (u64)cpu.regs.gpr[RS(instr)] < (u64)cpu.regs.gpr[RT(instr)]); };
    case 0x34: return [](CachedInterpreter& cpu, u32 instr) { trap(cpu, cpu.regs.gpr[RS(instr)] == cpu.regs.gpr[RT(instr)]); };
    case 0x36: return [](CachedInterpreter& cpu, u32 instr) { trap(cpu, cpu.regs.gpr[RS(instr)] != cpu.regs.gpr[RT(instr)]); };
    case 0x38: return dsll;
    case 0x3A: return dsrl;
    case 0x3B: return dsra;
    case 0x3C: return dsll32;
    case 0x3E: return dsrl32;
    case 0x3F: return dsra32;
    default:
      Util::panic("Unimplemented special {} {} ({:08X}) (pc: {:016X})", (mask >> 3) & 7, mask & 7, instr, (u64)regs.oldPC);
  }
}

CachedFn CachedInterpreter::regimm(u32 instr) {
  u8 mask = ((instr >> 16) & 0x1F);
  // 000r_rccc
  switch (mask) { // TODO: named constants for clearer code
    case 0x00: return [](CachedInterpreter& cpu, u32 instr) { b(cpu, instr, cpu.regs.gpr[RS(instr)] < 0); };
    case 0x01: return [](CachedInterpreter& cpu, u32 instr) { b(cpu, instr, cpu.regs.gpr[RS(instr)] >= 0); };
    case 0x02: return [](CachedInterpreter& cpu, u32 instr) { bl(cpu, instr, cpu.regs.gpr[RS(instr)] < 0); };
    case 0x03: return [](CachedInterpreter& cpu, u32 instr) { bl(cpu, instr, cpu.regs.gpr[RS(instr)] >= 0); };
    case 0x08: return [](CachedInterpreter& cpu, u32 instr) { trap(cpu, cpu.regs.gpr[RS(instr)] >= s64(s16(instr))); };
    case 0x09: return [](CachedInterpreter& cpu, u32 instr) { trap(cpu, u64(cpu.regs.gpr[RS(instr)]) >= u64(s64(s16(instr)))); };
    case 0x0A: return [](CachedInterpreter& cpu, u32 instr) { trap(cpu, cpu.regs.gpr[RS(instr)] < s64(s16(instr))); };
    case 0x0B: return [](CachedInterpreter& cpu, u32 instr) { trap(cpu, u64(cpu.regs.gpr[RS(instr)]) < u64(s64(s16(instr)))); };
    case 0x0C: return [](CachedInterpreter& cpu, u32 instr) { trap(cpu, cpu.regs.gpr[RS(instr)] == s64(s16(instr))); };
    case 0x0E: return [](CachedInterpreter& cpu, u32 instr) { trap(cpu, cpu.regs.gpr[RS(instr)] != s64(s16(instr))); };
    case 0x10: return [](CachedInterpreter& cpu, u32 instr) { blink(cpu, instr, cpu.regs.gpr[RS(instr)] < 0); };
    case 0x11: return [](CachedInterpreter& cpu, u32 instr) { blink(cpu, instr, cpu.regs.gpr[RS(instr)] >= 0); };
    case 0x12: return [](CachedInterpreter& cpu, u32 instr) { bllink(cpu, instr, cpu.regs.gpr[RS(instr)] < 0); };
    case 0x13: return [](CachedInterpreter& cpu, u32 instr) { bllink(cpu, instr, cpu.regs.gpr[RS(instr)] >= 0); };
    default:
      Util::panic("Unimplemented regimm {} {} ({:08X}) (pc: {:016X})", (mask >> 3) & 3, mask & 7, instr, (u64)regs.oldPC);
  }
}

CachedFn CachedInterpreter::cop2GetFunc(u32 instr) {
  if(!regs.cop0.status.cu2) {
    return [](CachedInterpreter& cpu, u32) { FireException(cpu.regs, ExceptionCode::CoprocessorUnusable, 2, true); };
  }
  switch(RS(instr)) {
    case 0x00: return mfc2;
    case 0x01: return dmfc2;
    case 0x02: return cfc2;
    case 0x04: return mtc2;
    case 0x05: return dmtc2;
    case 0x06: return ctc2;
    default:
      return [](CachedInterpreter& cpu, u32) { FireException(cpu.regs, ExceptionCode::ReservedInstruction, 2, true); };
  }
}

CachedFn CachedInterpreter::GetInstrFunc(u32 instr) {
  u8 mask = (instr >> 26) & 0x3f;
  // 00rr_rccc
  switch(mask) { // TODO: named constants for clearer code
    case 0x00: return special(instr);
    case 0x01: return regimm(instr);
    case 0x02: return j;
    case 0x03: return jal;
    case 0x04: return [](CachedInterpreter& cpu, u32 instr) { b(cpu, instr, cpu.regs.gpr[RS(instr)] == cpu.regs.gpr[RT(instr)]); };
    case 0x05: return [](CachedInterpreter& cpu, u32 instr) { b(cpu, instr, cpu.regs.gpr[RS(instr)] != cpu.regs.gpr[RT(instr)]); };
    case 0x06: return [](CachedInterpreter& cpu, u32 instr) { b(cpu, instr, cpu.regs.gpr[RS(instr)] <= 0); };
    case 0x07: return [](CachedInterpreter& cpu, u32 instr) { b(cpu, instr, cpu.regs.gpr[RS(instr)] > 0); };
    case 0x08: return addi;
    case 0x09: return addiu;
    case 0x0A: return slti;
    case 0x0B: return sltiu;
    case 0x0C: return andi;
    case 0x0D: return ori;
    case 0x0E: return xori;
    case 0x0F: return lui;
    case 0x10: return cop0GetFunc(*this, instr);
    case 0x11: return cop1GetFunc(*this, instr);
    case 0x12: return cop2GetFunc(instr);
    case 0x14: return [](CachedInterpreter& cpu, u32 instr) { bl(cpu, instr, cpu.regs.gpr[RS(instr)] == cpu.regs.gpr[RT(instr)]); };
    case 0x15: return [](CachedInterpreter& cpu, u32 instr) { bl(cpu, instr, cpu.regs.gpr[RS(instr)] != cpu.regs.gpr[RT(instr)]); };
    case 0x16: return [](CachedInterpreter& cpu, u32 instr) { bl(cpu, instr, cpu.regs.gpr[RS(instr)] <= 0); };
    case 0x17: return [](CachedInterpreter& cpu, u32 instr) { bl(cpu, instr, cpu.regs.gpr[RS(instr)] > 0); };
    case 0x18: return daddi;
    case 0x19: return daddiu;
    case 0x1A: return ldl;
    case 0x1B: return ldr;
    case 0x1F: return [](CachedInterpreter& cpu, u32 instr) { FireException(cpu.regs, ExceptionCode::ReservedInstruction, 0, true); };
    case 0x20: return lb;
    case 0x21: return lh;
    case 0x22: return lwl;
    case 0x23: return lw;
    case 0x24: return lbu;
    case 0x25: return lhu;
    case 0x26: return lwr;
    case 0x27: return lwu;
    case 0x28: return sb;
    case 0x29: return sh;
    case 0x2A: return swl;
    case 0x2B: return sw;
    case 0x2C: return sdl;
    case 0x2D: return sdr;
    case 0x2E: return swr;
    case 0x2F: return [](CachedInterpreter&, u32) {};
    case 0x30: return ll;
    case 0x31: return [](CachedInterpreter& cpu, u32 instr) { cpu.regs.cop1.lwc1(cpu.regs, cpu.mem, instr); };
    case 0x34: return lld;
    case 0x35: return [](CachedInterpreter& cpu, u32 instr) { cpu.regs.cop1.ldc1(cpu.regs, cpu.mem, instr); };
    case 0x37: return ld;
    case 0x38: return sc;
    case 0x39: return [](CachedInterpreter& cpu, u32 instr) {
      if(!cpu.regs.cop0.status.cu1) {
        FireException(cpu.regs, ExceptionCode::CoprocessorUnusable, 1, true);
        return;
      }
      u64 addr = (s64)(s16)instr + cpu.regs.gpr[BASE(instr)];

      u32 physical;
      if(!MapVAddr(cpu.regs, STORE, addr, physical)) {
        HandleTLBException(cpu.regs, addr);
        FireException(cpu.regs, GetTLBExceptionCode(cpu.regs.cop0.tlbError, STORE), 0, true);
      } else {
        cpu.mem.Write32(cpu.regs, cpu, physical, cpu.regs.cop1.GetReg<u32>(cpu.regs.cop0, FT(instr)));
      }
    };
    case 0x3C: return scd;
    case 0x3D: return [](CachedInterpreter& cpu, u32 instr) {
      if(!cpu.regs.cop0.status.cu1) {
        FireException(cpu.regs, ExceptionCode::CoprocessorUnusable, 1, true);
        return;
      }
      u64 addr = (s64)(s16)instr + cpu.regs.gpr[BASE(instr)];

      u32 physical;
      if(!MapVAddr(cpu.regs, STORE, addr, physical)) {
        HandleTLBException(cpu.regs, addr);
        FireException(cpu.regs, GetTLBExceptionCode(cpu.regs.cop0.tlbError, STORE), 0, true);
      } else {
        cpu.mem.Write64(cpu.regs, cpu, physical, cpu.regs.cop1.GetReg<u64>(cpu.regs.cop0, FT(instr)));
      }
    };
    case 0x3F: return sd;
    default:
      Util::panic("Unimplemented instruction {:02X} ({:08X}) (pc: {:016X})", mask, instr, (u64)regs.oldPC);
  }
}
}