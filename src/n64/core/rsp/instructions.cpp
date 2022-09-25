#include <RSP.hpp>
#include <util.hpp>
#include <n64/core/cpu/Registers.hpp>
#include <Mem.hpp>

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

inline int SignExt7bit(u8 val, int sa) {
  s8 sval = ((val << 1) & 0x80) | val;

  s32 sval32 = sval;
  u32 val32 = sval32;
  return val32 << sa;
}

inline auto GetCop0Reg(RSP& rsp, RDP& rdp, u8 index) -> u32{
  switch(index) {
    case 0: return rsp.lastSuccessfulSPAddr.raw;
    case 1: return rsp.lastSuccessfulDRAMAddr.raw;
    case 2:
    case 3: return rsp.spDMALen.raw;
    case 4: return rsp.spStatus.raw;
    case 5: return rsp.spStatus.dmaFull;
    case 6: return rsp.spStatus.dmaBusy;
    case 7: return AcquireSemaphore(rsp);
    case 9: return rdp.dpc.end;
    case 10: return rdp.dpc.current;
    case 11: return rdp.dpc.status.raw;
    case 12: return 0;
    default: util::panic("Unhandled RSP COP0 register read at index {}\n", index);
  }
  return 0;
}

inline void SetCop0Reg(Registers& regs, Mem& mem, u8 index, u32 val) {
  MMIO& mmio = mem.mmio;
  RSP& rsp = mmio.rsp;
  RDP& rdp = mmio.rdp;
  MI& mi = mmio.mi;
  switch(index) {
    case 0: rsp.spDMASPAddr.raw = val; break;
    case 1: rsp.spDMADRAMAddr.raw = val; break;
    case 2:
      rsp.spDMALen.raw = val;
      rsp.DMA<false>(rsp.spDMALen, mem.GetRDRAM(), rsp, rsp.spDMASPAddr.bank);
      break;
    case 3:
      rsp.spDMALen.raw = val;
      rsp.DMA<true>(rsp.spDMALen, mem.GetRDRAM(), rsp, rsp.spDMASPAddr.bank);
      break;
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
  s16 value = 0;
  switch(RD(instr) & 3) {
    case 0: value = VCOasU16(); break;
    case 1: value = VCCasU16(); break;
    case 2 ... 3: value = GetVCE(); break;
  }

  gpr[RT(instr)] = s32(value);
}

void RSP::ctc2(u32 instr) {
  u16 value = gpr[RT(instr)];
  switch(RD(instr) & 3) {
    case 0:
      for(int i = 0; i < 8; i++) {
        vco.h.element[7 - i] = ((value >> (i + 8)) & 1) == 1 ? 0xFFFF : 0;
        vco.l.element[7 - i] = ((value >> i) & 1) == 1 ? 0xFFFF : 0;
      }
      break;
    case 1:
      for(int i = 0; i < 8; i++) {
        vcc.h.element[7 - i] = ((value >> (i + 8)) & 1) == 1 ? 0xFFFF : 0;
        vcc.l.element[7 - i] = ((value >> i) & 1) == 1 ? 0xFFFF : 0;
      }
      break;
    case 2: case 3:
      for(int i = 0; i < 8; i++) {
        vce.element[7 - i] = ((value >> i) & 1) == 1 ? 0xFFFF : 0;
      }
      break;
  }
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

#define OFFSET(x) ((x) & 0x7F)

void RSP::lqv(u32 instr) {
  int e = E(instr);
  u32 addr = gpr[BASE(instr)] + SignExt7bit(OFFSET(instr), 4);
  u32 end = ((addr & ~15) + 15);

  for(int i = 0; addr + i <= end && i + e < 16; i++) {
    vpr[VT(instr)].byte[BYTE_INDEX(i + e)] = ReadByte(addr + i);
  }
}

void RSP::ldv(u32 instr) {
  int e = E(instr);
  u32 addr = gpr[BASE(instr)] + SignExt7bit(OFFSET(instr), 3);
  u32 end = e + 8 > 16 ? 16 : e + 8;

  for(int i = e; i < end; i++) {
    vpr[VT(instr)].byte[BYTE_INDEX(i)] = ReadByte(addr);
    addr++;
  }
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
  int e = E(instr);
  u32 addr = gpr[BASE(instr)] + SignExt7bit(OFFSET(instr), 4);
  u32 end = ((addr & ~15) + 15);

  for(int i = 0; addr + i <= end; i++) {
    WriteByte(addr + i, vpr[VT(instr)].byte[BYTE_INDEX((i + e) & 15)]);
  }
}

void RSP::sdv(u32 instr) {
  int e = E(instr);
  u32 addr = gpr[BASE(instr)] + SignExt7bit(OFFSET(instr), 3);

  for(int i = 0; i < 8; i++) {
    WriteByte(addr + i, vpr[VT(instr)].byte[BYTE_INDEX((i + e) & 0xF)]);
  }
}

void RSP::ssv(u32 instr) {
  int e = E(instr);
  u32 addr = gpr[BASE(instr)] + SignExt7bit(OFFSET(instr), 1);

  u8 hi = vpr[VT(instr)].byte[BYTE_INDEX(e & 15)];
  u8 lo = vpr[VT(instr)].byte[BYTE_INDEX((e + 1) & 15)];
  u16 val = (u16)hi << 8 | lo;

  WriteHalf(addr, val);
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

inline s16 clamp_signed(s64 val) {
  if(val > 32767) return 32767;
  if(val < -32768) return -32768;
  return val;
}

void RSP::vabs(u32 instr) {

}

void RSP::vmov(u32 instr) {

}

void RSP::vmacf(u32 instr) {
  VPR& vd = vpr[VD(instr)];
  VPR& vs = vpr[VS(instr)];
  VPR vte = GetVTE(vpr[VT(instr)], E(instr));

  for(int i = 0; i < 8; i++) {
    s32 prod = vs.selement[i] * vte.selement[i];
    s64 accDelta = prod * 2;
    s64 accum = GetACC(i) + accDelta;
    SetACC(i, accum);

    s16 result = clamp_signed(accum >> 16);
    vd.element[i] = result;
  }
}

void RSP::veq(u32 instr) {

}

void RSP::vne(u32 instr) {

}

void RSP::vsar(u32 instr) {

}

void RSP::vzero(u32 instr) {
  VPR& vs = vpr[VS(instr)];
  VPR& vd = vpr[VD(instr)];
  VPR vte = GetVTE(vpr[VT(instr)], E(instr));

  for(int i = 0; i < 8; i++) {
    acc.l.element[i] = vte.element[i] + vs.element[i];
  }

  memset(&vd, 0, sizeof(VPR));
}

void RSP::mfc0(RDP& rdp, u32 instr) {
  gpr[RT(instr)] = GetCop0Reg(*this, rdp, RD(instr));
}

void RSP::mtc0(Registers& regs, Mem& mem, u32 instr) {
  SetCop0Reg(regs, mem, RD(instr), gpr[RT(instr)]);
}

void RSP::mfc2(u32 instr) {
  u8 hi = vpr[RD(instr)].byte[BYTE_INDEX(E(instr))];
  u8 lo = vpr[RD(instr)].byte[BYTE_INDEX((E(instr) + 1) & 0xF)];
  s16 elem = (hi << 8) | lo;
  gpr[RT(instr)] = s32(elem);
}

void RSP::mtc2(u32 instr) {
  u16 element = gpr[RT(instr)];
  u8 lo = element;
  u8 hi = element >> 8;
  vpr[RD(instr)].byte[BYTE_INDEX(E(instr))] = hi;
  if(E(instr) < 15) {
    vpr[RD(instr)].byte[BYTE_INDEX(E(instr) + 1)] = lo;
  }
}
}