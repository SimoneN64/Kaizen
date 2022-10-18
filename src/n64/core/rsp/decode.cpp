#include <n64/core/RSP.hpp>
#include <util.hpp>
#include <n64/core/cpu/Registers.hpp>
#include <Interrupt.hpp>
#include <Mem.hpp>

namespace n64 {
inline void special(MI& mi, Registers& regs, RSP& rsp, u32 instr) {
  u8 mask = instr & 0x3f;
  //util::print("rsp special {:02X}\n", mask);
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
    case 0x0D:
      rsp.spStatus.halt = true;
      rsp.steps = 0;
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
  //util::print("rsp regimm {:02X}\n", mask);
  switch(mask) {
    case 0x00: rsp.b(instr, (s32)rsp.gpr[RS(instr)] < 0); break;
    case 0x01: rsp.b(instr, (s32)rsp.gpr[RS(instr)] >= 0); break;
    case 0x10: rsp.blink(instr, (s32)rsp.gpr[RS(instr)] < 0); break;
    case 0x11: rsp.blink(instr, (s32)rsp.gpr[RS(instr)] >= 0); break;
    default: util::panic("Unhandled RSP regimm instruction ({:05b})\n", mask);
  }
}

inline void lwc2(RSP& rsp, u32 instr) {
  u8 mask = (instr >> 11) & 0x1F;
  //util::print("lwc2 {:02X}\n", mask);
  switch(mask) {
    case 0x01: rsp.lsv(instr); break;
    case 0x02: rsp.llv(instr); break;
    case 0x03: rsp.ldv(instr); break;
    case 0x04: rsp.lqv(instr); break;
    case 0x05: rsp.lrv(instr); break;
    case 0x06: rsp.lpv(instr); break;
    case 0x07: rsp.luv(instr); break;
    case 0x0A: break;
    case 0x0B: rsp.ltv(instr); break;
    default: util::panic("Unhandled RSP LWC2 {:05b}\n", mask);
  }
}

inline void swc2(RSP& rsp, u32 instr) {
  u8 mask = (instr >> 11) & 0x1F;
  //util::print("swc2 {:02X}\n", mask);
  switch(mask) {
    case 0x00: rsp.sbv(instr); break;
    case 0x01: rsp.ssv(instr); break;
    case 0x02: rsp.slv(instr); break;
    case 0x03: rsp.sdv(instr); break;
    case 0x04: rsp.sqv(instr); break;
    case 0x06: rsp.spv(instr); break;
    case 0x07: rsp.suv(instr); break;
    case 0x0A: rsp.swv(instr); break;
    case 0x0B: rsp.stv(instr); break;
    default: util::panic("Unhandled RSP SWC2 {:05b}\n", mask);
  }
}

inline void cop2(RSP& rsp, u32 instr) {
  u8 mask = instr & 0x3F;
  u8 mask_sub = (instr >> 21) & 0x1F;
  //util::print("Cop2 {:02X}\n", mask);
  switch(mask) {
    case 0x00:
      if((instr >> 25) & 1) {
        rsp.vmulf(instr);
      } else {
        switch (mask_sub) {
          case 0x00: rsp.mfc2(instr); break;
          case 0x02: rsp.cfc2(instr); break;
          case 0x04: rsp.mtc2(instr); break;
          case 0x06: rsp.ctc2(instr); break;
          default: util::panic("Unhandled RSP COP2 sub ({:05b})\n", mask_sub);
        }
      }
      break;
    case 0x01: rsp.vmulu(instr); break;
    case 0x04: rsp.vmudl(instr); break;
    case 0x05: rsp.vmudm(instr); break;
    case 0x06: rsp.vmudn(instr); break;
    case 0x07: rsp.vmudh(instr); break;
    case 0x08: rsp.vmacf(instr); break;
    case 0x09: rsp.vmacu(instr); break;
    case 0x0C: rsp.vmadl(instr); break;
    case 0x0D: rsp.vmadm(instr); break;
    case 0x0E: rsp.vmadn(instr); break;
    case 0x0F: rsp.vmadh(instr); break;
    case 0x10: rsp.vadd(instr); break;
    case 0x11: rsp.vsub(instr); break;
    case 0x13: rsp.vabs(instr); break;
    case 0x14: rsp.vaddc(instr); break;
    case 0x15: rsp.vsubc(instr); break;
    case 0x1D: rsp.vsar(instr); break;
    case 0x20: rsp.vlt(instr); break;
    case 0x21: rsp.veq(instr); break;
    case 0x22: rsp.vne(instr); break;
    case 0x23: rsp.vge(instr); break;
    case 0x24: rsp.vcl(instr); break;
    case 0x25: rsp.vch(instr); break;
    case 0x26: rsp.vcr(instr); break;
    case 0x27: rsp.vmrg(instr); break;
    case 0x28: rsp.vand(instr); break;
    case 0x29: rsp.vnand(instr); break;
    case 0x2A: rsp.vor(instr); break;
    case 0x2B: rsp.vnor(instr); break;
    case 0x2C: rsp.vxor(instr); break;
    case 0x2D: rsp.vnxor(instr); break;
    case 0x31: rsp.vrcpl(instr); break;
    case 0x35: rsp.vrsql(instr); break;
    case 0x32: case 0x36:
      rsp.vrcph(instr);
      break;
    case 0x30: rsp.vrcp(instr); break;
    case 0x33: rsp.vmov(instr); break;
    case 0x34: rsp.vrsq(instr); break;
    case 0x37: case 0x3F: break;
    default: util::panic("Unhandled RSP COP2 ({:06b})\n", mask);
  }
}

inline void cop0(Registers& regs, Mem& mem, u32 instr) {
  u8 mask = (instr >> 21) & 0x1F;
  MMIO& mmio = mem.mmio;
  RSP& rsp = mmio.rsp;
  RDP& rdp = mmio.rdp;
  //util::print("Cop0 {:02X}\n", mask);
  if((instr & 0x7FF) == 0) {
    switch (mask) {
      case 0x00: rsp.mfc0(rdp, instr); break;
      case 0x04: rsp.mtc0(regs, mem, instr); break;
      default: util::panic("Unhandled RSP COP0 ({:05b})\n", mask);
    }
  } else {
    util::panic("RSP COP0 unknown {:08X}\n", instr);
  }
}

void RSP::Exec(Registers &regs, Mem& mem, u32 instr) {
  u8 mask = (instr >> 26) & 0x3F;
  MMIO& mmio = mem.mmio;
  RDP& rdp = mmio.rdp;
  MI& mi = mmio.mi;
  //util::print("RSP {:02X}\n", mask);
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
    case 0x10: cop0(regs, mem, instr); break;
    case 0x12: cop2(*this, instr); break;
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
    case 0x32: lwc2(*this, instr); break;
    case 0x3A: swc2(*this, instr); break;
    default: util::panic("Unhandled RSP instruction ({:06b})\n", mask);
  }
}
}