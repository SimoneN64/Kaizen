#pragma once
#include <n64/core/mmio/MI.hpp>
#include <n64/core/RDP.hpp>
#include <n64/memory_regions.hpp>
#include <Interrupt.hpp>

#define RSP_BYTE(addr, buf) (buf[BYTE_ADDRESS(addr) & 0xFFF])
#define GET_RSP_HALF(addr, buf) ((RSP_BYTE(addr, buf) << 8) | RSP_BYTE((addr) + 1, buf))
#define SET_RSP_HALF(addr, buf, value) do { RSP_BYTE(addr, buf) = ((value) >> 8) & 0xFF; RSP_BYTE((addr) + 1, buf) = (value) & 0xFF;} while(0)
#define GET_RSP_WORD(addr, buf) ((GET_RSP_HALF(addr, buf) << 16) | GET_RSP_HALF((addr) + 2, buf))
#define SET_RSP_WORD(addr, buf, value) do { SET_RSP_HALF(addr, buf, ((value) >> 16) & 0xFFFF); SET_RSP_HALF((addr) + 2, buf, (value) & 0xFFFF);} while(0)
#define GET_RSP_DWORD(addr, buf) (((u64)GET_RSP_WORD(addr, buf) << 32) | (u64)GET_RSP_WORD((addr) + 4, buf))
#define SET_RSP_DWORD(addr, buf, value) do { SET_RSP_WORD(addr, buf, ((value) >> 32) & 0xFFFFFFFF); SET_RSP_WORD((addr) + 4, buf, (value) & 0xFFFFFFFF);} while(0)

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
    unsigned signal0:1;
    unsigned signal1:1;
    unsigned signal2:1;
    unsigned signal3:1;
    unsigned signal4:1;
    unsigned signal5:1;
    unsigned signal6:1;
    unsigned signal7:1;
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
  void Step(Registers& regs, Mem& mem);
  auto Read(u32 addr) -> u32;
  void Write(Mem& mem, Registers& regs, u32 addr, u32 value);
  void Exec(Registers& regs, Mem& mem, u32 instr);
  SPStatus spStatus;
  u16 oldPC{}, pc{}, nextPC{};
  SPDMASPAddr spDMASPAddr{};
  SPDMADRAMAddr spDMADRAMAddr{};
  SPDMASPAddr lastSuccessfulSPAddr;
  SPDMADRAMAddr lastSuccessfulDRAMAddr;
  SPDMALen spDMALen{};
  u8 dmem[DMEM_SIZE]{}, imem[IMEM_SIZE]{};
  VPR vpr[32]{};
  s32 gpr[32]{};
  VPR vce{};

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

  inline s64 GetACC(int e) {
    s64 val = s64(acc.h.element[e]) << 32;
    val |= s64(acc.m.element[e]) << 16;
    val |= s64(acc.l.element[e]);
    if((val & 0x800000000000) != 0) {
      val |= 0xFFFF000000000000;
    }
    return val;
  }

  inline void SetACC(int e, s64 val) {
    acc.h.element[e] = val >> 32;
    acc.m.element[e] = val >> 16;
    acc.l.element[e] = val;
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

  inline void WriteStatus(MI& mi, Registers& regs, u32 value) {
    auto write = SPStatusWrite{.raw = value};
    CLEAR_SET(spStatus.halt, write.clearHalt, write.setHalt);
    if(write.clearBroke) spStatus.broke = false;
    if(write.clearIntr && !write.setIntr)
      InterruptLower(mi, regs, Interrupt::SP);
    if(write.setIntr && !write.clearIntr)
      InterruptRaise(mi, regs, Interrupt::SP);
    CLEAR_SET(spStatus.singleStep, write.clearSstep, write.setSstep);
    CLEAR_SET(spStatus.interruptOnBreak, write.clearIntrOnBreak, write.setIntrOnBreak);
    CLEAR_SET(spStatus.signal0, write.clearSignal0, write.setSignal0);
    CLEAR_SET(spStatus.signal1, write.clearSignal1, write.setSignal1);
    CLEAR_SET(spStatus.signal2, write.clearSignal2, write.setSignal2);
    CLEAR_SET(spStatus.signal3, write.clearSignal3, write.setSignal3);
    CLEAR_SET(spStatus.signal4, write.clearSignal4, write.setSignal4);
    CLEAR_SET(spStatus.signal5, write.clearSignal5, write.setSignal5);
    CLEAR_SET(spStatus.signal6, write.clearSignal6, write.setSignal6);
    CLEAR_SET(spStatus.signal7, write.clearSignal7, write.setSignal7);
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

  inline u8 GetVCE() {
    u8 value = 0;
    for(int i = 0; i < 8; i++) {
      bool l = vce.element[7 - i] != 0;
      value |= (l << i);
    }
    return value;
  }

  inline u64 ReadDword(u32 addr, bool i) {
    if (i) {
      return GET_RSP_DWORD(addr, imem);
    } else {
      return GET_RSP_DWORD(addr, dmem);
    }
  }

  inline void WriteDword(u32 addr, u64 val, bool i) {
    if (i) {
      SET_RSP_DWORD(addr, imem, val);
    } else {
      SET_RSP_DWORD(addr, dmem, val);
    }
  }

  inline u32 ReadWord(u32 addr, bool i) {
    if (i) {
      return GET_RSP_WORD(addr, imem);
    } else {
      return GET_RSP_WORD(addr, dmem);
    }
  }

  inline void WriteWord(u32 addr, u32 val, bool i) {
    if (i) {
      SET_RSP_WORD(addr, imem, val);
    } else {
      SET_RSP_WORD(addr, dmem, val);
    }
  }

  inline u16 ReadHalf(u32 addr, bool i) {
    if (i) {
      return GET_RSP_HALF(addr, imem);
    } else {
      return GET_RSP_HALF(addr, dmem);
    }
  }

  inline void WriteHalf(u32 addr, u16 val, bool i) {
    if (i) {
      SET_RSP_HALF(addr, imem, val);
    } else {
      SET_RSP_HALF(addr, dmem, val);
    }
  }

  inline u8 ReadByte(u32 addr, bool i) {
    if (i) {
      return RSP_BYTE(addr, imem);
    } else {
      return RSP_BYTE(addr, dmem);
    }
  }

  inline void WriteByte(u32 addr, u8 val, bool i) {
    if (i) {
      RSP_BYTE(addr, imem) = val;
    } else {
      RSP_BYTE(addr, dmem) = val;
    }
  }

  inline u64 ReadDword(u32 addr) {
    return GET_RSP_DWORD(addr, dmem);
  }

  inline void WriteDword(u32 addr, u64 val) {
    SET_RSP_DWORD(addr, dmem, val);
  }

  inline u32 ReadWord(u32 addr) {
    return GET_RSP_WORD(addr, dmem);
  }

  inline void WriteWord(u32 addr, u32 val) {
    SET_RSP_WORD(addr, dmem, val);
  }

  inline u16 ReadHalf(u32 addr) {
    return GET_RSP_HALF(addr, dmem);
  }

  inline void WriteHalf(u32 addr, u16 val) {
    SET_RSP_HALF(addr, dmem, val);
  }

  inline u8 ReadByte(u32 addr) {
    return RSP_BYTE(addr, dmem);
  }

  inline void WriteByte(u32 addr, u8 val) {
    RSP_BYTE(addr, dmem) = val;
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
  void b(u32 instr, bool cond);
  void bl(u32 instr, bool cond);
  void cfc2(u32 instr);
  void ctc2(u32 instr);
  void lb(u32 instr);
  void lh(u32 instr);
  void lw(u32 instr);
  void lbu(u32 instr);
  void lhu(u32 instr);
  void lui(u32 instr);
  void ldv(u32 instr);
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
  void sdv(u32 instr);
  void ssv(u32 instr);
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
  void vmacf(u32 instr);
  void veq(u32 instr);
  void vne(u32 instr);
  void vsar(u32 instr);
  void vzero(u32 instr);
  void mfc0(RDP& rdp, u32 instr);
  void mtc0(Registers& regs, Mem& mem, u32 instr);
  void mfc2(u32 instr);
  void mtc2(u32 instr);

  template <bool isDRAMdest>
  inline void DMA(SPDMALen len, u8* rdram, RSP& rsp, bool bank) {
    u32 length = len.len + 1;

    length = (length + 0x7) & ~0x7;

    u8* dst, *src;
    if constexpr (isDRAMdest) {
      dst = rdram;
      src = bank ? rsp.imem : rsp.dmem;
    } else {
      src = rdram;
      dst = bank ? rsp.imem : rsp.dmem;
    }

    u32 mem_address = rsp.spDMASPAddr.address & 0xFF8;
    u32 dram_address = rsp.spDMADRAMAddr.address & 0xFFFFF8;

    for (int i = 0; i < len.count + 1; i++) {
      for(int j = 0; j < length; j++) {
        if constexpr (isDRAMdest) {
          dst[dram_address + j] = src[(mem_address + j) & 0xFFF];
        } else {
          dst[(mem_address + j) & 0xFFF] = src[dram_address + j];
        }
      }

      int skip = i == len.count ? 0 : len.skip;

      dram_address += (length + skip) & 0xFFFFF8;
      mem_address += length;
    }

    rsp.lastSuccessfulSPAddr.address = mem_address & 0xFF8;
    rsp.lastSuccessfulSPAddr.bank = bank;
    rsp.lastSuccessfulDRAMAddr.address = dram_address & 0xFFFFF8;
    rsp.spDMALen.raw = 0xFF8 | (rsp.spDMALen.skip << 20);
  }
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