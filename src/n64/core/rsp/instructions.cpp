#include <RSP.hpp>
#include <util.hpp>
#include <n64/core/cpu/Registers.hpp>

namespace n64 {
inline bool AcquireSemaphore(RSP& rsp) {
  if(rsp.semaphore) {
    return true;
  } else {
    rsp.semaphore = true;
    return false;
  }
}

inline void ReleaseSemaphore(RSP& rsp) {
  rsp.semaphore = false;
}

inline auto GetCop0Reg(RSP& rsp, RDP& rdp, u8 index) -> u32{
  switch(index) {
    case 0: return rsp.spDMASPAddr.raw;
    case 1: return rsp.spDMADRAMAddr.raw;
    case 2:
    case 3: return rsp.spDMALen.raw;
    case 4: return rsp.spStatus.raw;
    case 5: return rsp.spStatus.dmaFull;
    case 6: return 0;
    case 7: return AcquireSemaphore(rsp);
    case 11: return rdp.dpc.status.raw;
    default: util::panic("Unhandled RSP COP0 register read at index {}\n", index);
  }
  return 0;
}

inline void SetCop0Reg(MI& mi, Registers& regs, RSP& rsp, RDP& rdp, u8 index, u32 val) {
  switch(index) {
    case 0: rsp.spDMASPAddr.raw = val; break;
    case 1: rsp.spDMADRAMAddr.raw = val; break;
    case 2:
    case 3: rsp.spDMALen.raw = val; break;
    case 4: rsp.spStatus.raw = val; break;
    case 7:
      if(val == 0) {
        ReleaseSemaphore(rsp);
      }
      break;
    case 8:
      rdp.dpc.start = val & 0xFFFFF8;
      rdp.dpc.current = rdp.dpc.start;
      break;
    case 9:
      rdp.dpc.end = val & 0xFFFFF8;
      rdp.RunCommand(mi, regs, rsp);
      break;
    case 11: rdp.StatusWrite(mi, regs, rsp, val); break;
    default: util::panic("Unhandled RSP COP0 register write at index {}\n", index);
  }
}

inline VPR Broadcast(VPR vt, int l0, int l1, int l2, int l3, int l4, int l5, int l6, int l7) {
  VPR vte{};
  vte.element[ELEMENT_INDEX(0)] = vt.element[l0];
  vte.element[ELEMENT_INDEX(1)] = vt.element[l1];
  vte.element[ELEMENT_INDEX(2)] = vt.element[l2];
  vte.element[ELEMENT_INDEX(3)] = vt.element[l3];
  vte.element[ELEMENT_INDEX(4)] = vt.element[l4];
  vte.element[ELEMENT_INDEX(5)] = vt.element[l5];
  vte.element[ELEMENT_INDEX(6)] = vt.element[l6];
  vte.element[ELEMENT_INDEX(7)] = vt.element[l7];
  return vte;
}

inline VPR GetVTE(VPR vt, u8 e) {
  VPR vte{};
  switch(e & 0xf) {
    case 0 ... 1: return vt;
    case 2 ... 3:
      vte = Broadcast(vt, e - 2, e - 2, e, e, e + 2, e + 2, e + 4, e + 4);
      break;
    case 4 ... 7:
      vte = Broadcast(vt, e - 4, e - 4, e - 4, e - 4, e, e, e, e);
      break;
    case 8 ... 15: {
      int index = e - 8;
      for (u16& i : vte.element) {
        i = vt.element[index];
      }
    } break;
  }
  return vte;
}

void RSP::add(u32 instr) {
  gpr[RD(instr)] = gpr[RS(instr)] + gpr[RT(instr)];
}

void RSP::addi(u32 instr) {
  s32 op1 = gpr[RS(instr)];
  s16 op2 = instr;
  s32 result = op1 + op2;
  gpr[RT(instr)] = result;
}

void RSP::and_(u32 instr) {
  gpr[RD(instr)] = gpr[RT(instr)] & gpr[RS(instr)];
}

void RSP::andi(u32 instr) {
  gpr[RT(instr)] = gpr[RS(instr)] & (u16)instr;
}

void RSP::cfc2(u32 instr) {

}

void RSP::b(u32 instr, bool cond) {
  s32 address = ((s32)((s16)(instr & 0xFFFF) << 2)) + pc;
  branch(address, cond);
}

void RSP::bl(u32 instr, bool cond) {
  b(instr, cond);
  gpr[31] = pc + 4;
}

void RSP::lb(u32 instr) {
  u32 address = gpr[BASE(instr)] + (s16)instr;
  gpr[RT(instr)] = (s32)(s8)ReadByte(address);
}

void RSP::lh(u32 instr) {
  u32 address = gpr[BASE(instr)] + (s16)instr;
  gpr[RT(instr)] = (s32)(s16)ReadHalf(address);
}

void RSP::lw(u32 instr) {
  u32 address = gpr[BASE(instr)] + (s16)instr;
  gpr[RT(instr)] = ReadWord(address);
}

void RSP::lbu(u32 instr) {
  u32 address = gpr[BASE(instr)] + (s16)instr;
  gpr[RT(instr)] = ReadByte(address);
}

void RSP::lhu(u32 instr) {
  u32 address = gpr[BASE(instr)] + (s16)instr;
  gpr[RT(instr)] = ReadHalf(address);
}

void RSP::lui(u32 instr) {
  u32 imm = ((u16)instr) << 16;
  gpr[RT(instr)] = imm;
}

void RSP::lqv(u32 instr) {

}

void RSP::j(u32 instr) {
  u32 target = (instr & 0x3ffffff) << 2;
  nextPC = target;
}

void RSP::jal(u32 instr) {
  j(instr);
  gpr[31] = pc + 4;
}

void RSP::jr(u32 instr) {
  nextPC = gpr[RS(instr)];
}

void RSP::jalr(u32 instr) {
  jr(instr);
  gpr[RD(instr)] = pc + 4;
}

void RSP::nor(u32 instr) {
  gpr[RD(instr)] = ~(gpr[RT(instr)] | gpr[RS(instr)]);
}

void RSP::ori(u32 instr) {
  s32 op1 = gpr[RS(instr)];
  u32 op2 = instr & 0xffff;
  s32 result = op1 | op2;
  gpr[RT(instr)] = result;
}

void RSP::xori(u32 instr) {
  s32 op1 = gpr[RS(instr)];
  u32 op2 = instr & 0xffff;
  s32 result = op1 ^ op2;
  gpr[RT(instr)] = result;
}

void RSP::or_(u32 instr) {
  gpr[RD(instr)] = gpr[RT(instr)] | gpr[RS(instr)];
}

void RSP::xor_(u32 instr) {
  gpr[RD(instr)] = gpr[RT(instr)] ^ gpr[RS(instr)];
}

void RSP::sb(u32 instr) {
  u32 address = gpr[BASE(instr)] + (s16)instr;
  WriteByte(address, gpr[RT(instr)]);
}

void RSP::sh(u32 instr) {
  u32 address = gpr[BASE(instr)] + (s16)instr;
  WriteHalf(address, gpr[RT(instr)]);
}

void RSP::sw(u32 instr) {
  u32 address = gpr[BASE(instr)] + (s16)instr;
  WriteWord(address, gpr[RT(instr)]);
}

void RSP::sub(u32 instr) {
  gpr[RD(instr)] = gpr[RS(instr)] - gpr[RT(instr)];
}

void RSP::sqv(u32 instr) {

}

void RSP::sllv(u32 instr) {
  u8 sa = (gpr[RS(instr)]) & 0x1F;
  u32 rt = gpr[RT(instr)];
  u32 result = rt << sa;
  gpr[RD(instr)] = result;
}

void RSP::srlv(u32 instr) {
  u8 sa = (gpr[RS(instr)]) & 0x1F;
  u32 rt = gpr[RT(instr)];
  u32 result = rt >> sa;
  gpr[RD(instr)] = result;
}

void RSP::srav(u32 instr) {
  u8 sa = (gpr[RS(instr)]) & 0x1F;
  s32 rt = gpr[RT(instr)];
  s32 result = rt >> sa;
  gpr[RD(instr)] = result;
}

void RSP::sll(u32 instr) {
  u8 sa = (instr >> 6) & 0x1f;
  gpr[RD(instr)] = (u32)gpr[RT(instr)] << sa;
}

void RSP::srl(u32 instr) {
  u8 sa = (instr >> 6) & 0x1f;
  gpr[RD(instr)] = (u32)gpr[RT(instr)] >> sa;
}

void RSP::sra(u32 instr) {
  u8 sa = (instr >> 6) & 0x1f;
  gpr[RD(instr)] = gpr[RT(instr)] >> sa;
}

void RSP::slt(u32 instr) {
  gpr[RD(instr)] = gpr[RS(instr)] < gpr[RT(instr)];
}

void RSP::sltu(u32 instr) {
  gpr[RD(instr)] = (u32)gpr[RS(instr)] < (u32)gpr[RT(instr)];
}

void RSP::slti(u32 instr) {
  s32 imm = (s16)instr;
  gpr[RT(instr)] = gpr[RS(instr)] < imm;
}

void RSP::sltiu(u32 instr) {
  s32 imm = (s16)instr;
  gpr[RT(instr)] = (u32)gpr[RS(instr)] < imm;
}

void RSP::vabs(u32 instr) {

}

void RSP::vmov(u32 instr) {

}

void RSP::veq(u32 instr) {

}

void RSP::vne(u32 instr) {

}

void RSP::vsar(u32 instr) {

}

void RSP::mfc0(RDP& rdp, u32 instr) {
  gpr[RT(instr)] = GetCop0Reg(*this, rdp, RD(instr));
}

void RSP::mtc0(MI& mi, Registers& regs, RDP& rdp, u32 instr) {
  SetCop0Reg(mi, regs, *this, rdp, RD(instr), gpr[RT(instr)]);
}
}