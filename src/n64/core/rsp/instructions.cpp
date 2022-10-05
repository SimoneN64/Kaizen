#include <RSP.hpp>
#include <util.hpp>
#include <n64/core/cpu/Registers.hpp>
#include <Mem.hpp>
#include <rcp_rsq.hpp>

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
    case 4: rsp.WriteStatus(mi, regs, val); break;
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
      int index = ELEMENT_INDEX(e - 8);
      for (u16& vteE : vte.element) {
        vteE = vt.element[index];
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
  int e = E1(instr);
  u32 addr = gpr[BASE(instr)] + SignExt7bit(OFFSET(instr), 4);
  u32 end = ((addr & ~15) + 15);

  for(int i = 0; addr + i <= end && i + e < 16; i++) {
    vpr[VT(instr)].byte[BYTE_INDEX(i + e)] = ReadByte(addr + i);
  }
}

void RSP::lrv(u32 instr) {
  int e = E1(instr);
  u32 addr = gpr[BASE(instr)] + SignExt7bit(OFFSET(instr), 4);
  int start = 16 - ((addr & 0xf) - e);
  addr &= 0xFFFFFFF0;

  for(int i = start; i < 16; i++) {
    vpr[VT(instr)].byte[BYTE_INDEX(i & 0xf)] = ReadByte(addr++);
  }
}

void RSP::lpv(u32 instr) {
  int e = E1(instr);
  u32 addr = gpr[BASE(instr)] + SignExt7bit(OFFSET(instr), 3);

  int addrOffset = addr & 7;
  addr &= ~7;

  for(int elem = 0; elem < 8; elem++) {
    int elemOffset = (16 - e + (elem + addrOffset)) & 0xF;

    u16 value = ReadByte(addr + elemOffset);
    value <<= 8;
    vpr[VT(instr)].element[ELEMENT_INDEX(elem)] = value;
  }
}

void RSP::luv(u32 instr) {
  u32 addr = gpr[BASE(instr)] + SignExt7bit(OFFSET(instr), 3);

  int e = E1(instr);

  int addrOffset = addr & 7;
  addr &= ~7;

  for (int elem = 0; elem < 8; elem++) {
    int elemOffset = (16 - e + (elem + addrOffset)) & 0xF;

    u16 value = ReadByte(addr + elemOffset);
    value <<= 7;
    vpr[VT(instr)].element[ELEMENT_INDEX(elem)] = value;
  }
}

void RSP::suv(u32 instr) {
  u32 addr = gpr[BASE(instr)] + SignExt7bit(OFFSET(instr), 3);

  int start = E1(instr);
  int end = start + 8;

  for (int offset = start; offset < end; offset++) {
    if((offset & 15) < 8) {
      WriteByte(addr++, vpr[VT(instr)].element[ELEMENT_INDEX(offset & 7)] >> 7);
    } else {
      WriteByte(addr++, vpr[VT(instr)].byte[BYTE_INDEX((offset & 7) << 1)]);
    }
  }
}

void RSP::ldv(u32 instr) {
  int e = E1(instr);
  u32 addr = gpr[BASE(instr)] + SignExt7bit(OFFSET(instr), 3);
  u32 end = e + 8 > 16 ? 16 : e + 8;

  for(int i = e; i < end; i++) {
    vpr[VT(instr)].byte[BYTE_INDEX(i)] = ReadByte(addr);
    addr++;
  }
}

void RSP::lsv(u32 instr) {
  int e = E1(instr);
  u32 addr = gpr[BASE(instr)] + SignExt7bit(OFFSET(instr), 1);
  u16 val = ReadHalf(addr);
  vpr[VT(instr)].byte[BYTE_INDEX(e)] = val >> 8;
  if(e < 15) {
    vpr[VT(instr)].byte[BYTE_INDEX(e + 1)] = val;
  }
}

void RSP::llv(u32 instr) {
  int e = E1(instr);
  u32 addr = gpr[BASE(instr)] + SignExt7bit(OFFSET(instr), 2);

  for(int i = 0; i < 4; i++) {
    int elem = i + e;
    if(elem > 15) {
      break;
    }

    vpr[VT(instr)].byte[BYTE_INDEX(elem)] = ReadByte(addr + i);
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
  int e = E1(instr);
  u32 addr = gpr[BASE(instr)] + SignExt7bit(OFFSET(instr), 4);
  u32 end = ((addr & ~15) + 15);

  for(int i = 0; addr + i <= end; i++) {
    WriteByte(addr + i, vpr[VT(instr)].byte[BYTE_INDEX((i + e) & 15)]);
  }
}

void RSP::spv(u32 instr) {
  int e = E1(instr);
  u32 addr = gpr[BASE(instr)] + SignExt7bit(OFFSET(instr), 3);

  int start = E1(instr);
  int end = start + 8;

  for(int offset = start; offset < end; offset++) {
    if((offset & 15) < 8) {
      WriteByte(addr++, vpr[VT(instr)].byte[BYTE_INDEX((offset & 7) << 1)]);
    } else {
      WriteByte(addr++, vpr[VT(instr)].byte[ELEMENT_INDEX(offset & 7)] >> 7);
    }
  }
}

void RSP::sbv(u32 instr) {
  int e = E1(instr);
  u32 addr = gpr[BASE(instr)] + SignExt7bit(OFFSET(instr), 3);

  WriteByte(addr, vpr[VT(instr)].byte[BYTE_INDEX(e & 0xF)]);
}

void RSP::sdv(u32 instr) {
  int e = E1(instr);
  u32 addr = gpr[BASE(instr)] + SignExt7bit(OFFSET(instr), 3);

  for(int i = 0; i < 8; i++) {
    WriteByte(addr + i, vpr[VT(instr)].byte[BYTE_INDEX((i + e) & 0xF)]);
  }
}

void RSP::ssv(u32 instr) {
  int e = E1(instr);
  u32 addr = gpr[BASE(instr)] + SignExt7bit(OFFSET(instr), 1);

  u8 hi = vpr[VT(instr)].byte[BYTE_INDEX(e & 15)];
  u8 lo = vpr[VT(instr)].byte[BYTE_INDEX((e + 1) & 15)];
  u16 val = (u16)hi << 8 | lo;

  WriteHalf(addr, val);
}

void RSP::slv(u32 instr) {
  int e = E1(instr);
  u32 addr = gpr[BASE(instr)] + SignExt7bit(OFFSET(instr), 2);

  for(int i = 0; i < 4; i++) {
    WriteByte(addr + i, vpr[VT(instr)].byte[BYTE_INDEX((i + e) & 0xF)]);
  }
}

void RSP::stv(u32 instr) {
  u32 base = gpr[BASE(instr)] + SignExt7bit(OFFSET(instr), 4);
  u32 addrOffset = base & 0x7;
  base &= ~0x7;

  u8 e = E1(instr) >> 1;

  for (int i = 0; i < 8; i++) {
    u32 address = base;
    u32 offset = (i << 1) + addrOffset;

    int reg = (VT(instr) & 0x18) | ((i + e) & 0x7);

    u16 val = vpr[reg].element[ELEMENT_INDEX(i & 0x7)];
    u16 hi = (val >> 8) & 0xFF;
    u16 lo = (val >> 0) & 0xFF;

    WriteByte(address + ((offset + 0) & 0xF), hi);
    WriteByte(address + ((offset + 1) & 0xF), lo);
  }
}

void RSP::ltv(u32 instr) {
  u32 base = gpr[BASE(instr)] + SignExt7bit(OFFSET(instr), 4);
  base &= ~0x7;

  u8 e = E1(instr);

  for (int i = 0; i < 8; i++) {
    u32 address = base;
    u32 offset = (i << 1) + e + (base & 8);

    u16 hi = ReadByte(address + (offset & 0xF));
    u16 lo = ReadByte(address + ((offset + 1) & 0xF));

    int reg = (VT(instr) & 0x18) | ((i + (e >> 1)) & 0x7);

    vpr[reg].element[ELEMENT_INDEX(i & 0x7)] = (hi << 8) | lo;
  }
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
  u8 sa = gpr[RS(instr)] & 0x1F;
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
  VPR& vs = vpr[VS(instr)];
  VPR& vd = vpr[VD(instr)];
  VPR vte = GetVTE(vpr[VT(instr)], E2(instr));

  for(int i = 0; i < 8; i++) {
    if(vs.selement[i] < 0) {
      if(vte.element[i] == 0x8000) {
        vd.element[i] = 0x7FFF;
        acc.l.element[i] = 0x8000;
      } else {
        vd.element[i] = -vte.selement[i];
        acc.l.element[i] = -vte.selement[i];
      }
    } else if (vs.element[i] == 0) {
      vd.element[i] = 0;
      acc.l.element[i] = 0;
    } else {
      vd.element[i] = vte.element[i];
      acc.l.element[i] = vte.element[i];
    }
  }
}

void RSP::vadd(u32 instr) {
  VPR& vs = vpr[VS(instr)];
  VPR& vd = vpr[VD(instr)];
  VPR vte = GetVTE(vpr[VT(instr)], E2(instr));

  for(int i = 0; i < 8; i++) {
    s32 result = vs.selement[i] + vte.selement[i] + (vco.l.element[i] != 0);
    acc.l.element[i] = result;
    vd.element[i] = clamp_signed(result);
    vco.l.element[i] = 0;
    vco.h.element[i] = 0;
  }
}

void RSP::vaddc(u32 instr) {
  VPR& vs = vpr[VS(instr)];
  VPR& vd = vpr[VD(instr)];
  VPR vte = GetVTE(vpr[VT(instr)], E2(instr));

  for(int i = 0; i < 8; i++) {
    s32 result = vs.selement[i] + vte.selement[i];
    acc.l.element[i] = result;
    vd.element[i] = result;
    vco.l.element[i] = (result >> 16) & 1 ? 0xffff : 0;
    vco.h.element[i] = 0;
  }
}

void RSP::vch(u32 instr) {
  int e = E2(instr);
  VPR& vs = vpr[VS(instr)];
  VPR& vd = vpr[VD(instr)];
  VPR vte = GetVTE(vpr[VT(instr)], e);

  for(int i = 0; i < 8; i++) {
    s16 vsElem = vs.selement[i];
    s16 vteElem = vte.selement[i];

    if((vsElem ^ vteElem) < 0) {
      s16 result = vsElem + vteElem;

      acc.l.selement[i] = (result <= 0) ? -vteElem : vsElem;
      vcc.l.element[i] = result <= 0 ? 0xffff : 0;
      vcc.h.element[i] = vteElem < 0 ? 0xffff : 0;
      vco.l.element[i] = 0xffff;
      vco.h.element[i] = (result != 0 && (u16)vsElem != ((u16)vteElem ^ 0xffff)) ? 0xffff : 0;
      vce.element[i] = result == -1 ? 0xffff : 0;
    } else {
      s16 result = vsElem - vteElem;
      acc.l.selement[i] = (result >= 0) ? vteElem : vsElem;
      vcc.l.element[i] = vteElem < 0 ? 0xffff : 0;
      vcc.h.element[i] = result >= 0 ? 0xffff : 0;
      vco.l.element[i] = 0;
      vco.h.element[i] = (result != 0 && (u16)vsElem != ((u16)vteElem ^ 0xffff)) ? 0xffff : 0;
      vce.element[i] = 0;
    }

    vd.element[i] = acc.l.element[i];
  }
}

void RSP::vcr(u32 instr) {
  int e = E2(instr);
  VPR& vs = vpr[VS(instr)];
  VPR& vd = vpr[VD(instr)];
  VPR vte = GetVTE(vpr[VT(instr)], e);

  for(int i = 0; i < 8; i++) {
    u16 vsE = vs.element[i];
    u16 vteE = vte.element[i];

    bool signDiff = (0x8000 & (vsE ^ vteE)) == 0x8000;

    u16 vtAbs = signDiff ? ~vteE : vteE;

    bool gte = s16(vteE) <= s16(signDiff ? 0xffff : vsE);
    bool lte = (((signDiff ? vsE : 0) + vteE) & 0x8000) == 0x8000;

    bool check = signDiff ? lte : gte;
    u16 result = check ? vtAbs : vsE;

    acc.l.element[i] = result;
    vd.element[i] = result;

    vcc.h.element[i] = gte ? 0xffff : 0;
    vcc.l.element[i] = lte ? 0xffff : 0;

    vco.l.element[i] = 0;
    vco.h.element[i] = 0;
    vce.element[i] = 0;
  }
}

void RSP::vcl(u32 instr) {
  int e = E2(instr);
  VPR& vs = vpr[VS(instr)];
  VPR& vd = vpr[VD(instr)];
  VPR vte = GetVTE(vpr[VT(instr)], e);

  for(int i = 0; i < 8; i++) {
    s16 vsElem = vs.selement[i];
    s16 vteElem = vte.selement[i];

    if(vco.l.element[i]) {
      if(vco.h.element[i]) {
        acc.l.element[i] = vcc.l.element[i] ? -vteElem : vsElem;
      } else {
        u16 clampSum = vsElem + vteElem;
        bool overflow = (vsElem + vteElem) != clampSum;

        acc.l.element[i] = vcc.l.element[i] ? -vteElem : vsElem;
        if(vce.element[i]) {
          vcc.l.element[i] = !clampSum || !overflow ? 0xffff : 0;
        } else {
          vcc.l.element[i] = !clampSum && !overflow ? 0xffff : 0;
        }
      }
    } else {
      acc.l.element[i] = vcc.l.element[i] ? -vteElem : vsElem;
      if(!vco.h.element[i]) {
        vcc.h.element[i] = s32(vsElem) - s32(vteElem) >= 0 ? 0xffff : 0;
      }
    }

    vco.l.element[i] = 0;
    vco.h.element[i] = 0;
    vce.element[i] = 0;
    vd.element[i] = acc.l.element[i];
  }
}

void RSP::vmov(u32 instr) {
  int e = E2(instr);
  VPR& vd = vpr[VD(instr)];
  VPR vte = GetVTE(vpr[VT(instr)], e);

  u32 se;

  switch (e) {
    case 0 ... 1:
      se = VS(instr) & 7;
      break;
    case 2 ... 3:
      se = (e & 1) | (VS(instr) & 6);
      break;
    case 4 ... 7:
      se = (e & 3) | (VS(instr) & 4);
      break;
    case 8 ... 15:
      se = e & 7;
      break;
    default:
      util::panic("VMOV: This should be unreachable!\n");
  }

  u8 de = VS(instr) & 7;

  vd.element[ELEMENT_INDEX(de)] = vte.element[ELEMENT_INDEX(se)];
  for(int i = 0; i < 8; i++) {
    acc.l.element[i] = vte.element[i];
  }
}

inline bool IsSignExtension(s16 hi, s16 lo) {
  if (hi == 0) {
    return (lo & 0x8000) == 0;
  } else if (hi == -1) {
    return (lo & 0x8000) == 0x8000;
  }
  return false;
}

void RSP::vmudl(u32 instr) {
  int e = E2(instr);
  VPR& vs = vpr[VS(instr)];
  VPR& vd = vpr[VD(instr)];
  VPR vte = GetVTE(vpr[VT(instr)], e);
  for(int i = 0; i < 8; i++) {
    u64 prod = (u64)vs.selement[i] * (u64)vte.selement[i];
    u64 accum = prod >> 16;

    SetACC(i, accum);

    u16 result;
    if(IsSignExtension(acc.h.selement[i], acc.m.selement[i])) {
      result = acc.l.element[i];
    } else if (acc.h.selement[i] < 0) {
      result = 0;
    } else {
      result = 0xffff;
    }

    vd.element[i] = result;
  }
}

void RSP::vmudh(u32 instr) {
  int e = E2(instr);
  VPR& vs = vpr[VS(instr)];
  VPR& vd = vpr[VD(instr)];
  VPR vte = GetVTE(vpr[VT(instr)], e);
  for(int i = 0; i < 8; i++) {
    s32 prod = vs.selement[i] * vte.selement[i];
    s64 accum = prod;

    s16 result = clamp_signed(accum);

    accum <<= 16;
    SetACC(i, accum);

    vd.element[i] = result;
  }
}

void RSP::vmudm(u32 instr) {
  int e = E2(instr);
  VPR& vs = vpr[VS(instr)];
  VPR& vd = vpr[VD(instr)];
  VPR vte = GetVTE(vpr[VT(instr)], e);
  for(int i = 0; i < 8; i++) {
    s32 prod = vs.selement[i] * vte.element[i];
    s64 accum = prod;

    s16 result = clamp_signed(accum >> 16);
    SetACC(i, accum);

    vd.element[i] = result;
  }
}

void RSP::vmudn(u32 instr) {
  int e = E2(instr);
  VPR& vs = vpr[VS(instr)];
  VPR& vd = vpr[VD(instr)];
  VPR vte = GetVTE(vpr[VT(instr)], e);
  for(int i = 0; i < 8; i++) {
    s32 prod = vs.element[i] * vte.selement[i];
    s64 accum = prod;
    SetACC(i, accum);

    u16 result;
    if(IsSignExtension(acc.h.selement[i], acc.m.selement[i])) {
      result = acc.l.element[i];
    } else if(acc.h.selement[i] < 0) {
      result = 0;
    } else {
      result = 0xffff;
    }

    vd.element[i] = result;
  }
}

void RSP::vmadh(u32 instr) {
  int e = E2(instr);
  VPR& vs = vpr[VS(instr)];
  VPR& vd = vpr[VD(instr)];
  VPR vte = GetVTE(vpr[VT(instr)], e);
  for(int i = 0; i < 8; i++) {
    s32 prod = vs.selement[i] * vte.selement[i];
    s64 accum = GetACC(i) + ((u64)prod << 16);
    SetACC(i, accum);

    s16 result = clamp_signed(accum >> 16);

    vd.element[i] = result;
  }
}

void RSP::vmadl(u32 instr) {
  int e = E2(instr);
  VPR& vs = vpr[VS(instr)];
  VPR& vd = vpr[VD(instr)];
  VPR vte = GetVTE(vpr[VT(instr)], e);
  for(int i = 0; i < 8; i++) {
    u64 prod = (u64)vs.selement[i] * (u64)vte.selement[i];
    u64 accum = (prod >> 16) + GetACC(i);

    SetACC(i, accum);

    u16 result;
    if(IsSignExtension(acc.h.selement[i], acc.m.selement[i])) {
      result = acc.l.element[i];
    } else if (acc.h.selement[i] < 0) {
      result = 0;
    } else {
      result = 0xffff;
    }

    vd.element[i] = result;
  }
}

void RSP::vmadm(u32 instr) {
  int e = E2(instr);
  VPR& vs = vpr[VS(instr)];
  VPR& vd = vpr[VD(instr)];
  VPR vte = GetVTE(vpr[VT(instr)], e);
  for(int i = 0; i < 8; i++) {
    s32 prod = vs.selement[i] * vte.element[i];
    s64 accum = GetACC(i);
    accum += prod;
    SetACC(i, accum);
    accum = GetACC(i);

    s16 result = clamp_signed(accum >> 16);

    vd.element[i] = result;
  }
}

void RSP::vmadn(u32 instr) {
  int e = E2(instr);
  VPR& vs = vpr[VS(instr)];
  VPR& vd = vpr[VD(instr)];
  VPR vte = GetVTE(vpr[VT(instr)], e);
  for(int i = 0; i < 8; i++) {
    s32 prod = vs.element[i] * vte.selement[i];
    s64 accum = GetACC(i) + prod;

    SetACC(i, accum);

    u16 result;
    if(IsSignExtension(acc.h.selement[i], acc.m.selement[i])) {
      result = acc.l.element[i];
    } else if (acc.h.selement[i] < 0) {
      result = 0;
    } else {
      result = 0xffff;
    }

    vd.element[i] = result;
  }
}

void RSP::vmacf(u32 instr) {
  VPR& vd = vpr[VD(instr)];
  VPR& vs = vpr[VS(instr)];
  VPR vte = GetVTE(vpr[VT(instr)], E2(instr));

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
  int e = E2(instr);
  VPR& vd = vpr[VD(instr)];
  VPR& vs = vpr[VS(instr)];
  VPR vte = GetVTE(vpr[VT(instr)], e);

  for(int i = 0; i < 8; i++) {
    vcc.l.element[i] = vco.h.element[i] || (vs.element[i] == vte.element[i]) ? 0xffff : 0;
    acc.l.element[i] = vcc.l.element[i] ? vs.element[i] : vte.element[i];
    vd.element[i] = acc.l.element[i];
    vcc.h.element[i] = vco.h.element[i] = vco.l.element[i] = 0;
  }
}

void RSP::vne(u32 instr) {
  int e = E2(instr);
  VPR& vd = vpr[VD(instr)];
  VPR& vs = vpr[VS(instr)];
  VPR vte = GetVTE(vpr[VT(instr)], e);

  for(int i = 0; i < 8; i++) {
    vcc.l.element[i] = vco.h.element[i] || (vs.element[i] != vte.element[i]) ? 0xffff : 0;
    acc.l.element[i] = vcc.l.element[i] ? vs.element[i] : vte.element[i];
    vd.element[i] = acc.l.element[i];
    vcc.h.element[i] = vco.h.element[i] = vco.l.element[i] = 0;
  }
}

void RSP::vge(u32 instr) {
  int e = E2(instr);
  VPR& vd = vpr[VD(instr)];
  VPR& vs = vpr[VS(instr)];
  VPR vte = GetVTE(vpr[VT(instr)], e);

  for(int i = 0; i < 8; i++) {
    bool eql = vs.element[i] == vte.element[i];
    bool neg = !(vco.h.element[i] & vco.l.element[i]) & eql;
    vcc.l.element[i] = (neg || (vs.element[i] > vte.element[i])) ? 0xffff : 0;
    acc.l.element[i] = vcc.l.element[i] ? vs.element[i] : vte.element[i];
    vd.element[i] = acc.l.element[i];
    vcc.h.element[i] = vco.h.element[i] = vco.l.element[i] = 0;
  }
}

void RSP::vlt(u32 instr) {
  int e = E2(instr);
  VPR& vd = vpr[VD(instr)];
  VPR& vs = vpr[VS(instr)];
  VPR vte = GetVTE(vpr[VT(instr)], e);

  for(int i = 0; i < 8; i++) {
    bool eql = vs.element[i] == vte.element[i];
    bool neg = vco.h.element[i] & vco.l.element[i] & eql;
    vcc.l.element[i] = (neg || (vs.element[i] < vte.element[i])) ? 0xffff : 0;
    acc.l.element[i] = vcc.l.element[i] ? vs.element[i] : vte.element[i];
    vd.element[i] = acc.l.element[i];
    vcc.h.element[i] = vco.h.element[i] = vco.l.element[i] = 0;
  }
}

inline u32 rcp(s32 sinput) {
  s32 mask = sinput >> 31;
  s32 input = sinput ^ mask;
  if (sinput > INT16_MIN) {
    input -= mask;
  }
  if (input == 0) {
    return 0x7FFFFFFF;
  } else if (sinput == INT16_MIN) {
    return 0xFFFF0000;
  }

  u32 shift = __builtin_clz(input);
  u64 dinput = (u64)input;
  u32 index = ((dinput << shift) & 0x7FC00000) >> 22;

  s32 result = rcp_rom[index];
  result = (0x10000 | result) << 14;
  result = (result >> (31 - shift)) ^ mask;
  return result;
}

inline u32 rsq(s32 input) {
  if (input == 0) {
    return 0x7FFFFFFF;
  } else if (input == 0xFFFF8000) {
    return 0xFFFF0000;
  } else if (input > 0xFFFF8000) {
    input--;
  }

  s32 sinput = input;
  s32 mask = sinput >> 31;
  input ^= mask;

  int shift = __builtin_clz(input) + 1;

  int index = (((input << shift) >> 24) | ((shift & 1) << 8));
  u32 rom = (((u32)rsq_rom[index]) << 14);
  int r_shift = ((32 - shift) >> 1);
  u32 result = (0x40000000 | rom) >> r_shift;

  return result ^ mask;
}

void RSP::vrcpl(u32 instr) {
  VPR& vd = vpr[VD(instr)];
  VPR& vt = vpr[VT(instr)];
  VPR vte = GetVTE(vpr[VT(instr)], E2(instr));
  int e = E2(instr) & 7;
  int de = DE(instr) & 7;

  s32 input;
  if(divInLoaded) {
    input = (s32(divIn) << 16) | vt.element[ELEMENT_INDEX(e)];
  } else {
    input = vt.selement[ELEMENT_INDEX(e)];
  }

  s32 result = rcp(input);
  divOut = result >> 16;
  divIn = 0;
  divInLoaded = false;

  for(int i = 0; i < 8; i++) {
    acc.l.element[i] = vte.element[i];
  }

  vd.element[ELEMENT_INDEX(de)] = result;
}

void RSP::vrcp(u32 instr) {
  VPR& vd = vpr[VD(instr)];
  VPR& vt = vpr[VT(instr)];
  VPR vte = GetVTE(vpr[VT(instr)], E2(instr));
  int e = E2(instr) & 7;
  int de = DE(instr) & 7;
  s32 input = vt.selement[ELEMENT_INDEX(e)];
  s32 result = rcp(input);
  vd.element[ELEMENT_INDEX(de)] = result;
  divOut = result >> 16;
  divInLoaded = false;

  for (int i = 0; i < 8; i++) {
    acc.l.element[i] = vte.element[i];
  }
}

void RSP::vrsq(u32 instr) {
  VPR& vd = vpr[VD(instr)];
  VPR& vt = vpr[VT(instr)];
  VPR vte = GetVTE(vpr[VT(instr)], E2(instr));
  int e = E2(instr) & 7;
  int de = DE(instr) & 7;
  s32 input = vt.selement[ELEMENT_INDEX(e)];
  s32 result = rsq(input);
  vd.element[ELEMENT_INDEX(de)] = result;
  divOut = result >> 16;
  divInLoaded = false;

  for (int i = 0; i < 8; i++) {
    acc.l.element[i] = vte.element[i];
  }
}

void RSP::vrsql(u32 instr) {
  VPR& vd = vpr[VD(instr)];
  VPR& vt = vpr[VT(instr)];
  VPR vte = GetVTE(vpr[VT(instr)], E2(instr));
  int e = E2(instr) & 7;
  int de = DE(instr) & 7;

  s32 input;
  if(divInLoaded) {
    input = (divIn << 16) | vt.element[ELEMENT_INDEX(e)];
  } else {
    input = vt.selement[ELEMENT_INDEX(e)];
  }

  u32 result = rsq(input);
  divOut = result >> 16;
  divInLoaded = false;

  for(int i = 0; i < 8; i++) {
    acc.l.element[i] = vte.element[i];
  }

  vd.element[ELEMENT_INDEX(de)] = result;
}

void RSP::vrcph(u32 instr) {
  int e = E2(instr);
  int de = DE(instr);
  VPR& vd = vpr[VD(instr)];
  VPR& vt = vpr[VT(instr)];
  VPR vte = GetVTE(vpr[VT(instr)], e);

  for(int i = 0; i < 8; i++) {
    acc.l.element[i] = vte.element[i];
  }

  vd.element[ELEMENT_INDEX(de)] = divOut;
  divIn = vt.element[ELEMENT_INDEX(e)];
  divInLoaded = true;
}

void RSP::vsar(u32 instr) {
  u8 e = E2(instr);
  switch(e) {
    case 0x8:
      for(int i = 0; i < 8; i++) {
        vpr[VD(instr)].element[i] = acc.h.element[i];
      }
      break;
    case 0x9:
      for(int i = 0; i < 8; i++) {
        vpr[VD(instr)].element[i] = acc.m.element[i];
      }
      break;
    case 0xA:
      for(int i = 0; i < 8; i++) {
        vpr[VD(instr)].element[i] = acc.l.element[i];
      }
      break;
    default:
      for(int i = 0; i < 8; i++) {
        vpr[VD(instr)].element[i] = 0;
      }
      break;
  }
}

void RSP::vsubc(u32 instr) {
  int e = E2(instr);
  VPR& vs = vpr[VS(instr)];
  VPR& vd = vpr[VD(instr)];
  VPR vte = GetVTE(vpr[VT(instr)], e);

  for(int i = 0; i < 8; i++) {
    u32 result = vs.element[i] - vte.element[i];
    acc.l.element[i] = result;
    vd.element[i] = result;

    vco.l.element[i] = (result >> 16) & 1 ? 0xffff : 0;
    vco.h.element[i] = result != 0 ? 0xffff : 0;
  }
}

void RSP::vsub(u32 instr) {
  int e = E2(instr);
  VPR& vs = vpr[VS(instr)];
  VPR& vd = vpr[VD(instr)];
  VPR vte = GetVTE(vpr[VT(instr)], e);

  for(int i = 0; i < 8; i++) {
    u32 result = vs.element[i] - vte.element[i] - vco.l.element[i];
    acc.l.element[i] = result;
    vd.element[i] = clamp_signed(s32(result) & 0x1FFFF);

    vco.l.element[i] = 0;
    vco.h.element[i] = 0;
  }
}

void RSP::vmrg(u32 instr) {
  int e = E2(instr);
  VPR& vs = vpr[VS(instr)];
  VPR& vd = vpr[VD(instr)];
  VPR vte = GetVTE(vpr[VT(instr)], e);

  for(int i = 0; i < 8; i++) {
    acc.l.element[i] = vcc.l.element[i] ? vs.element[i] : vte.element[i];
    vd.element[i] = acc.l.element[i];
  }
}

void RSP::vxor(u32 instr) {
  int e = E2(instr);
  VPR& vs = vpr[VS(instr)];
  VPR& vd = vpr[VD(instr)];
  VPR vte = GetVTE(vpr[VT(instr)], e);

  for(int i = 0; i < 8; i++) {
    acc.l.element[i] = vte.element[i] ^ vs.element[i];
    vd.element[i] = acc.l.element[i];
  }
}

void RSP::vxnor(u32 instr) {
  int e = E2(instr);
  VPR& vs = vpr[VS(instr)];
  VPR& vd = vpr[VD(instr)];
  VPR vte = GetVTE(vpr[VT(instr)], e);

  for(int i = 0; i < 8; i++) {
    acc.l.element[i] = ~(vte.element[i] ^ vs.element[i]);
    vd.element[i] = acc.l.element[i];
  }
}

void RSP::vand(u32 instr) {
  int e = E2(instr);
  VPR& vs = vpr[VS(instr)];
  VPR& vd = vpr[VD(instr)];
  VPR vte = GetVTE(vpr[VT(instr)], e);

  for(int i = 0; i < 8; i++) {
    acc.l.element[i] = vte.element[i] & vs.element[i];
    vd.element[i] = acc.l.element[i];
  }
}

void RSP::vnand(u32 instr) {
  int e = E2(instr);
  VPR& vs = vpr[VS(instr)];
  VPR& vd = vpr[VD(instr)];
  VPR vte = GetVTE(vpr[VT(instr)], e);

  for(int i = 0; i < 8; i++) {
    acc.l.element[i] = ~(vte.element[i] & vs.element[i]);
    vd.element[i] = acc.l.element[i];
  }
}

void RSP::vnor(u32 instr) {
  int e = E2(instr);
  VPR& vs = vpr[VS(instr)];
  VPR& vd = vpr[VD(instr)];
  VPR vte = GetVTE(vpr[VT(instr)], e);

  for(int i = 0; i < 8; i++) {
    acc.l.element[i] = ~(vte.element[i] | vs.element[i]);
    vd.element[i] = acc.l.element[i];
  }
}

void RSP::vzero(u32 instr) {
  VPR& vs = vpr[VS(instr)];
  VPR& vd = vpr[VD(instr)];
  VPR vte = GetVTE(vpr[VT(instr)], E2(instr));

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
  u8 hi = vpr[RD(instr)].byte[BYTE_INDEX(E1(instr))];
  u8 lo = vpr[RD(instr)].byte[BYTE_INDEX((E1(instr) + 1) & 0xF)];
  s16 elem = (hi << 8) | lo;
  gpr[RT(instr)] = s32(elem);
}

void RSP::mtc2(u32 instr) {
  u16 element = gpr[RT(instr)];
  u8 lo = element;
  u8 hi = element >> 8;
  vpr[RD(instr)].byte[BYTE_INDEX(E1(instr))] = hi;
  if(E1(instr) < 15) {
    vpr[RD(instr)].byte[BYTE_INDEX(E1(instr) + 1)] = lo;
  }
}
}