#pragma once
#include <n64/core/mmio/MI.hpp>
#include <n64/core/RDP.hpp>
#include <n64/memory_regions.hpp>

namespace n64 {
union SPStatus {
  u32 raw;
  struct {
    unsigned halt:1;
    unsigned broke:1;
    unsigned dmaBusy:1;
    unsigned dmaFull:1;
    unsigned ioFull:1;
    unsigned singleStep:1;
    unsigned interruptOnBreak:1;
    unsigned signal0Set:1;
    unsigned signal1Set:1;
    unsigned signal2Set:1;
    unsigned signal3Set:1;
    unsigned signal4Set:1;
    unsigned signal5Set:1;
    unsigned signal6Set:1;
    unsigned signal7Set:1;
    unsigned:17;
  };
};

union SPStatusWrite {
  u32 raw;
  struct {
    unsigned clearHalt:1;
    unsigned setHalt:1;
    unsigned clearBroke:1;
    unsigned clearIntr:1;
    unsigned setIntr:1;
    unsigned clearSstep:1;
    unsigned setSstep:1;
    unsigned clearIntrOnBreak:1;
    unsigned setIntrOnBreak:1;
    unsigned clearSignal0:1;
    unsigned setSignal0:1;
    unsigned clearSignal1:1;
    unsigned setSignal1:1;
    unsigned clearSignal2:1;
    unsigned setSignal2:1;
    unsigned clearSignal3:1;
    unsigned setSignal3:1;
    unsigned clearSignal4:1;
    unsigned setSignal4:1;
    unsigned clearSignal5:1;
    unsigned setSignal5:1;
    unsigned clearSignal6:1;
    unsigned setSignal6:1;
    unsigned clearSignal7:1;
    unsigned setSignal7:1;
    unsigned:7;
  };
};

union SPDMALen {
  struct {
    unsigned len:12;
    unsigned count:8;
    unsigned skip:12;
  };
  u32 raw;
};

union SPDMASPAddr {
  struct {
    unsigned address:12;
    unsigned bank:1;
    unsigned: 19;
  };
  u32 raw;
};

union SPDMADRAMAddr {
  struct {
    unsigned address:24;
    unsigned:8;
  };
  u32 raw;
};

union VPR {
  s16 selement[8];
  u16 element[8];
  u8 byte[8];
  u32 word[4];
};

struct Mem;
struct Registers;

#define VT(x) (((x) >> 16) & 0x1F)
#define VS(x) (((x) >> 11) & 0x1F)
#define VD(x) (((x) >> 6) & 0x1F)
#define E(x) (((x) >> 21) & 0x1F)
#define DE(x) (((x) >> 11) & 0x1F)
#define CLEAR_SET(val, clear, set) do { \
  if(clear) (val) = 0; \
  if(set) (val) = 1; \
} while(0)

struct RSP {
  RSP();
  void Reset();
  void Step(MI& mi, Registers& regs, RDP& rdp);
  auto Read(u32 addr) const -> u32;
  void Write(Mem& mem, Registers& regs, u32 addr, u32 value);
  void Exec(MI& mi, Registers& regs, RDP& rdp, u32 instr);
  SPStatus spStatus{.raw = 1};
  u16 oldPC{}, pc{}, nextPC = 4;
  SPDMASPAddr spDMASPAddr{};
  SPDMADRAMAddr spDMADRAMAddr{};
  SPDMALen spDMARDLen{}, spDMAWRLen{};
  u8 dmem[DMEM_SIZE]{}, imem[IMEM_SIZE]{};
  VPR vpr[32]{};
  u32 gpr[32]{};
  u8 vce{};

  struct {
    VPR h{}, m{}, l{};
  } acc;

  struct {
    VPR l{}, h{};
  } vcc, vco;

  bool semaphore = false;

  inline void SetPC(u16 val) {
    pc = val;
    nextPC = val += 4;
  }

  inline u16 VCOasU16() {
    u16 val = 0;
    for(int i = 0; i < 8; i++) {
      bool h = vco.h.element[7 - i] != 0;
      bool l = vco.l.element[7 - i] != 0;
      u32 mask = (l << i) | (h << (i + 8));
      val |= mask;
    }
    return val;
  }

  inline u16 VCCasU16() {
    u16 val = 0;
    for(int i = 0; i < 8; i++) {
      bool h = vcc.h.element[7 - i] != 0;
      bool l = vcc.l.element[7 - i] != 0;
      u32 mask = (l << i) | (h << (i + 8));
      val |= mask;
    }
    return val;
  }

  void add(u32 instr);
  void addi(u32 instr);
  void and_(u32 instr);
  void andi(u32 instr);
  void cfc2(u32 instr);
  void b(u32 instr, bool cond);
  void lh(u32 instr);
  void lw(u32 instr);
  void lui(u32 instr);
  void lqv(u32 instr);
  void j(u32 instr);
  void jal(u32 instr);
  void jr(u32 instr);
  void nor(u32 instr);
  void or_(u32 instr);
  void ori(u32 instr);
  void sb(u32 instr);
  void sh(u32 instr);
  void sw(u32 instr);
  void sqv(u32 instr);
  void sllv(u32 instr);
  void sll(u32 instr);
  void vabs(u32 instr);
  void vmov(u32 instr);
  void veq(u32 instr);
  void vne(u32 instr);
  void vsar(u32 instr);
  void mfc0(RDP& rdp, u32 instr);
  void mtc0(MI& mi, Registers& regs, RDP& rdp, u32 instr);
private:
  inline void branch(u16 address, bool cond) {
    if(cond) {
      nextPC = address & 0xFFF;
    }
  }
};
}