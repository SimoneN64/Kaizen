#include <n64/core/RSP.hpp>
#include <util.hpp>
#include <n64/core/cpu/Registers.hpp>
#include <Interrupt.hpp>

namespace n64 {
inline void special(MI& mi, Registers& regs, RSP& rsp, u32 instr) {
  u8 mask = instr & 0x3f;
  switch(mask) {
    case 0x00:
      if(instr != 0) {
        rsp.sll(instr);
      }
      break;
    case 0x02: rsp.srl(instr); break;
    case 0x03: rsp.sra(instr); break;
    case 0x04: rsp.sllv(instr); break;
    case 0x06: rsp.srlv(instr); break;
    case 0x07: rsp.srav(instr); break;
    case 0x08: rsp.jr(instr); break;
    case 0x09: rsp.jalr(instr); break;
    case 0x0C:
    case 0x0D:
      rsp.spStatus.halt = true;
      rsp.spStatus.broke = true;
      if(rsp.spStatus.interruptOnBreak) {
        InterruptRaise(mi, regs, Interrupt::SP);
      }
      break;
    case 0x20: case 0x21:
      rsp.add(instr);
      break;
    case 0x22: case 0x23:
      rsp.sub(instr);
      break;
    case 0x24: rsp.and_(instr); break;
    case 0x25: rsp.or_(instr); break;
    case 0x26: rsp.xor_(instr); break;
    case 0x27: rsp.nor(instr); break;
    case 0x2A: rsp.slt(instr); break;
    case 0x2B: rsp.sltu(instr); break;
    default: util::panic("Unhandled RSP special instruction ({:06b})\n", mask);
  }
}

inline void regimm(RSP& rsp, u32 instr) {
  u8 mask = ((instr >> 16) & 0x1F);
  switch(mask) {
    case 0x00: rsp.b(instr, (s32)rsp.gpr[RS(instr)] < 0); break;
    case 0x01: rsp.b(instr, (s32)rsp.gpr[RS(instr)] >= 0); break;
    case 0x10: rsp.bl(instr, (s32)rsp.gpr[RS(instr)] < 0); break;
    case 0x11: rsp.bl(instr, (s32)rsp.gpr[RS(instr)] >= 0); break;
    default: util::panic("Unhandled RSP regimm instruction ({:06b})\n", mask);
  }
}

inline void lwc2(RSP& rsp, u32 instr) {
  u8 mask = (instr >> 11) & 0x1F;
  switch(mask) {
    //case 0x04: rsp.lqv(instr); break;
    default: util::panic("Unhandled RSP LWC2 {} {}\n", (mask >> 3) & 3, mask & 7);
  }
}

inline void swc2(RSP& rsp, u32 instr) {
  u8 mask = (instr >> 11) & 0x1F;
  switch(mask) {
    //case 0x04: rsp.sqv(instr); break;
    default: util::panic("Unhandled RSP SWC2 {} {}\n", (mask >> 3) & 3, mask & 7);
  }
}

inline void cop2(RSP& rsp, u32 instr) {
  u8 mask = instr & 0x3F;
  u8 mask_sub = (instr >> 21) & 0x1F;
  switch(mask) {
    case 0x00:
      switch(mask_sub) {
        //case 0x02: rsp.cfc2(instr); break;
        default: util::panic("Unhandled RSP COP2 sub ({:06b})\n", mask_sub);
      }
      break;
    //case 0x13: rsp.vabs(instr); break;
    //case 0x1D: rsp.vsar(instr); break;
    //case 0x21: rsp.veq(instr); break;
    //case 0x22: rsp.vne(instr); break;
    //case 0x33: rsp.vmov(instr); break;
    default: util::panic("Unhandled RSP COP2 ({:06b})\n", mask);
  }
}

inline void cop0(MI& mi, Registers& regs, RSP& rsp, RDP& rdp, u32 instr) {
  u8 mask = (instr >> 21) & 0x1F;
  switch(mask) {
    case 0x00: rsp.mfc0(rdp, instr); break;
    case 0x04: rsp.mtc0(mi, regs, rdp, instr); break;
    default: util::panic("Unhandled RSP COP0 ({:06b})\n", mask);
  }
}

void RSP::Exec(MI &mi, Registers &regs, RDP &rdp, u32 instr) {
  u8 mask = (instr >> 26) & 0x3F;
  switch(mask) {
    case 0x00: special(mi, regs, *this, instr); break;
    case 0x01: regimm(*this, instr); break;
    case 0x02: j(instr); break;
    case 0x03: jal(instr); break;
    case 0x04: b(instr, (s32)gpr[RT(instr)] == (s32)gpr[RS(instr)]); break;
    case 0x05: b(instr, (s32)gpr[RT(instr)] != (s32)gpr[RS(instr)]); break;
    case 0x06: b(instr, (s32)gpr[RS(instr)] <= 0); break;
    case 0x07: b(instr, (s32)gpr[RS(instr)] > 0); break;
    case 0x08: case 0x09: addi(instr); break;
    case 0x0A: slti(instr); break;
    case 0x0B: sltiu(instr); break;
    case 0x0C: andi(instr); break;
    case 0x0D: ori(instr); break;
    case 0x0E: xori(instr); break;
    case 0x0F: lui(instr); break;
    case 0x10: cop0(mi, regs, *this, rdp, instr); break;
    //case 0x12: cop2(*this, instr); break;
    case 0x20: lb(instr); break;
    case 0x21: lh(instr); break;
    case 0x23: case 0x27:
      lw(instr);
      break;
    case 0x24: lbu(instr); break;
    case 0x25: lhu(instr); break;
    case 0x28: sb(instr); break;
    case 0x29: sh(instr); break;
    case 0x2B: sw(instr); break;
    //case 0x32: lwc2(*this, instr); break;
    //case 0x3A: swc2(*this, instr); break;
    default: util::panic("Unhandled RSP instruction ({:06b})\n", mask);
  }
}
}