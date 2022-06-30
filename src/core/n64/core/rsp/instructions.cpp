#include <RSP.hpp>
#include <util.hpp>
#include <n64/core/cpu/Registers.hpp>

namespace natsukashii::n64::core {

#define ELEMENT_INDEX(i) (7 - (i))
#define BYTE_INDEX(i) (15 - (i))

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

inline u32 GetCop0Reg(RSP& rsp, RDP& rdp, u8 index) {
  switch(index) {
    case 4: return rsp.spStatus.raw;
    case 5: return rsp.spStatus.dmaFull;
    case 6: return 0;
    case 7: return AcquireSemaphore(rsp);
    case 11: return rdp.dpc.status.raw;
    default: util::panic("Unhandled RSP COP0 register read at index {}\n", index);
  }
}

inline void SetCop0Reg(MI& mi, Registers& regs, RSP& rsp, RDP& rdp, u8 index, u32 val) {
  switch(index) {
    case 0: rsp.spDMASPAddr.raw = val; break;
    case 1: rsp.spDMADRAMAddr.raw = val; break;
    case 2: rsp.spDMARDLen.raw = val; break;
    case 3: rsp.spDMAWRLen.raw = val; break;
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
      rdp.RunCommand(mi, regs, rdp, rsp);
      break;
    case 11: rdp.StatusWrite(val); break;
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

}

void RSP::addi(u32 instr) {

}

void RSP::and_(u32 instr) {

}

void RSP::andi(u32 instr) {

}

void RSP::cfc2(u32 instr) {

}

void RSP::b(u32 instr, bool cond) {
  s32 address = ((s32)((s16)(instr & 0xFFFF) << 2)) + pc;
  branch(address, cond);
}

void RSP::lh(u32 instr) {

}

void RSP::lw(u32 instr) {

}

void RSP::lui(u32 instr) {

}

void RSP::lqv(u32 instr) {

}

void RSP::j(u32 instr) {

}

void RSP::jal(u32 instr) {

}

void RSP::jr(u32 instr) {

}

void RSP::nor(u32 instr) {

}

void RSP::or_(u32 instr) {

}

void RSP::ori(u32 instr) {

}

void RSP::sb(u32 instr) {

}

void RSP::sh(u32 instr) {

}

void RSP::sw(u32 instr) {

}

void RSP::sqv(u32 instr) {

}

void RSP::sllv(u32 instr) {
  u8 sa = gpr[RS(instr)] & 0x1F;
  gpr[RD(instr)] = gpr[RT(instr)] << sa;
}

void RSP::sll(u32 instr) {

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