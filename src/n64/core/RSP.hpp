#pragma once
#include <n64/core/mmio/MI.hpp>
#include <n64/core/RDP.hpp>
#include <n64/memory_regions.hpp>

#define RSP_BYTE(addr) (dmem[BYTE_ADDRESS(addr) & 0xFFF])
#define GET_RSP_HALF(addr) ((RSP_BYTE(addr) << 8) | RSP_BYTE((addr) + 1))
#define SET_RSP_HALF(addr, value) do { RSP_BYTE(addr) = ((value) >> 8) & 0xFF; RSP_BYTE((addr) + 1) = (value) & 0xFF;} while(0)
#define GET_RSP_WORD(addr) ((GET_RSP_HALF(addr) << 16) | GET_RSP_HALF((addr) + 2))
#define SET_RSP_WORD(addr, value) do { SET_RSP_HALF(addr, ((value) >> 16) & 0xFFFF); SET_RSP_HALF((addr) + 2, (value) & 0xFFFF);} while(0)

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
  u8 byte[16];
  u32 word[4];
};

struct Mem;
struct Registers;

#define DE(x) (((x) >> 11) & 0x1F)
#define CLEAR_SET(val, clear, set) do { \
  if(clear && !set) (val) = 0; \
  if(set && !clear) (val) = 1; \
} while(0)

struct RSP {
  RSP();
  void Reset();
  void Step(MI& mi, Registers& regs, RDP& rdp);
  auto Read(u32 addr) -> u32;
  void Write(Mem& mem, Registers& regs, u32 addr, u32 value);
  void Exec(MI& mi, Registers& regs, RDP& rdp, u32 instr);
  SPStatus spStatus;
  u16 oldPC{}, pc{}, nextPC{};
  SPDMASPAddr spDMASPAddr{};
  SPDMADRAMAddr spDMADRAMAddr{};
  SPDMALen spDMALen{};
  u8 dmem[DMEM_SIZE]{}, imem[IMEM_SIZE]{};
  VPR vpr[32]{};
  s32 gpr[32]{};
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

  inline u32 ReadWord(u32 addr) {
    return GET_RSP_WORD(addr);
  }

  inline void WriteWord(u32 addr, u32 val) {
    SET_RSP_WORD(addr, val);
  }

  inline u16 ReadHalf(u32 addr) {
    return GET_RSP_HALF(addr);
  }

  inline void WriteHalf(u32 addr, u16 val) {
    SET_RSP_HALF(addr, val);
  }

  inline u8 ReadByte(u32 addr) {
    return RSP_BYTE(addr);
  }

  inline void WriteByte(u32 addr, u8 val) {
    RSP_BYTE(addr) = val;
  }

  inline bool AcquireSemaphore() {
    if(semaphore) {
      return true;
    } else {
      semaphore = true;
      return false;
    }
  }

  inline void ReleaseSemaphore() {
    semaphore = false;
  }

  void add(u32 instr);
  void addi(u32 instr);
  void and_(u32 instr);
  void andi(u32 instr);
  void cfc2(u32 instr);
  void b(u32 instr, bool cond);
  void bl(u32 instr, bool cond);
  void lb(u32 instr);
  void lh(u32 instr);
  void lw(u32 instr);
  void lbu(u32 instr);
  void lhu(u32 instr);
  void lui(u32 instr);
  void lqv(u32 instr);
  void j(u32 instr);
  void jal(u32 instr);
  void jr(u32 instr);
  void jalr(u32 instr);
  void nor(u32 instr);
  void or_(u32 instr);
  void ori(u32 instr);
  void xor_(u32 instr);
  void xori(u32 instr);
  void sb(u32 instr);
  void sh(u32 instr);
  void sw(u32 instr);
  void sub(u32 instr);
  void sqv(u32 instr);
  void sllv(u32 instr);
  void srlv(u32 instr);
  void srav(u32 instr);
  void sll(u32 instr);
  void srl(u32 instr);
  void sra(u32 instr);
  void slt(u32 instr);
  void sltu(u32 instr);
  void slti(u32 instr);
  void sltiu(u32 instr);
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
      nextPC = address & 0xFFC;
    }
  }

  inline void branch_likely(u16 address, bool cond) {
    if(cond) {
      nextPC = address & 0xFFC;
    } else {
      pc = nextPC & 0xFFC;
      nextPC = pc + 4;
    }
  }
};
}