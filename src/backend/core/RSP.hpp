#pragma once
#include <core/mmio/MI.hpp>
#include <core/RDP.hpp>
#include <MemoryRegions.hpp>
#include <MemoryHelpers.hpp>
#include <Interrupt.hpp>

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
  m128i single;
} __attribute__((packed));

static_assert(sizeof(VPR) == 16);

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

  FORCE_INLINE void Step(Registers& regs, Mem& mem) {
    gpr[0] = 0;
    u32 instr = Util::ReadAccess<u32>(imem, pc & IMEM_DSIZE);
    oldPC = pc & 0xFFC;
    pc = nextPC & 0xFFC;
    nextPC += 4;

    Exec(regs, mem, instr);
  }
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
  s16 divIn{}, divOut{};
  bool divInLoaded = false;
  int steps = 0;

  struct {
    VPR h{}, m{}, l{};
  } acc;

  struct {
    VPR l{}, h{};
  } vcc, vco;

  bool semaphore = false;

  FORCE_INLINE void SetPC(u16 val) {
    oldPC = pc & 0xFFC;
    pc = val & 0xFFC;
    nextPC = pc + 4;
  }

  FORCE_INLINE s64 GetACC(int e) const {
    s64 val = u64(acc.h.element[e]) << 32;
    val    |= u64(acc.m.element[e]) << 16;
    val    |= u64(acc.l.element[e]) << 00;
    if((val & 0x0000800000000000) != 0) {
      val  |= 0xFFFF000000000000;
    }
    return val;
  }

  FORCE_INLINE void SetACC(int e, s64 val) {
    acc.h.element[e] = val >> 32;
    acc.m.element[e] = val >> 16;
    acc.l.element[e] = val;
  }

  FORCE_INLINE u16 GetVCO() const {
    u16 value = 0;
    for (int i = 0; i < 8; i++) {
      bool h = vco.h.element[7 - i] != 0;
      bool l = vco.l.element[7 - i] != 0;
      u32 mask = (l << i) | (h << (i + 8));
      value |= mask;
    }
    return value;
  }

  FORCE_INLINE u16 GetVCC() const {
    u16 value = 0;
    for (int i = 0; i < 8; i++) {
      bool h = vcc.h.element[7 - i] != 0;
      bool l = vcc.l.element[7 - i] != 0;
      u32 mask = (l << i) | (h << (i + 8));
      value |= mask;
    }
    return value;
  }

  FORCE_INLINE u8 GetVCE() const {
    u8 value = 0;
    for(int i = 0; i < 8; i++) {
      bool l = vce.element[ELEMENT_INDEX(i)] != 0;
      value |= (l << i);
    }
    return value;
  }

  FORCE_INLINE u32 ReadWord(u32 addr) {
    addr &= 0xfff;
    return GET_RSP_WORD(addr);
  }

  FORCE_INLINE void WriteWord(u32 addr, u32 val) {
    addr &= 0xfff;
    SET_RSP_WORD(addr, val);
  }

  FORCE_INLINE u16 ReadHalf(u32 addr) {
    addr &= 0xfff;
    return GET_RSP_HALF(addr);
  }

  FORCE_INLINE void WriteHalf(u32 addr, u16 val) {
    addr &= 0xfff;
    SET_RSP_HALF(addr, val);
  }

  FORCE_INLINE u8 ReadByte(u32 addr) {
    addr &= 0xfff;
    return RSP_BYTE(addr);
  }

  FORCE_INLINE void WriteByte(u32 addr, u8 val) {
    addr &= 0xfff;
    RSP_BYTE(addr) = val;
  }

  FORCE_INLINE bool AcquireSemaphore() {
    if(semaphore) {
      return true;
    } else {
      semaphore = true;
      return false;
    }
  }

  FORCE_INLINE void ReleaseSemaphore() {
    semaphore = false;
  }

  void add(u32 instr);
  void addi(u32 instr);
  void and_(u32 instr);
  void andi(u32 instr);
  void b(u32 instr, bool cond);
  void blink(u32 instr, bool cond);
  void cfc2(u32 instr);
  void ctc2(u32 instr);
  void lb(u32 instr);
  void lh(u32 instr);
  void lw(u32 instr);
  void lbu(u32 instr);
  void lhu(u32 instr);
  void lui(u32 instr);
  void luv(u32 instr);
  void lbv(u32 instr);
  void ldv(u32 instr);
  void lsv(u32 instr);
  void llv(u32 instr);
  void lrv(u32 instr);
  void lqv(u32 instr);
  void lfv(u32 instr);
  void lhv(u32 instr);
  void ltv(u32 instr);
  void lpv(u32 instr);
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
  void swv(u32 instr);
  void sub(u32 instr);
  void sbv(u32 instr);
  void sdv(u32 instr);
  void stv(u32 instr);
  void sqv(u32 instr);
  void ssv(u32 instr);
  void suv(u32 instr);
  void slv(u32 instr);
  void shv(u32 instr);
  void sfv(u32 instr);
  void srv(u32 instr);
  void spv(u32 instr);
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
  void vadd(u32 instr);
  void vaddc(u32 instr);
  void vand(u32 instr);
  void vnand(u32 instr);
  void vch(u32 instr);
  void vcr(u32 instr);
  void vcl(u32 instr);
  void vmacf(u32 instr);
  void vmacu(u32 instr);
  void vmacq(u32 instr);
  void vmadh(u32 instr);
  void vmadl(u32 instr);
  void vmadm(u32 instr);
  void vmadn(u32 instr);
  void vmov(u32 instr);
  void vmulf(u32 instr);
  void vmulu(u32 instr);
  void vmulq(u32 instr);
  void vmudl(u32 instr);
  void vmudh(u32 instr);
  void vmudm(u32 instr);
  void vmudn(u32 instr);
  void vmrg(u32 instr);
  void vlt(u32 instr);
  void veq(u32 instr);
  void vne(u32 instr);
  void vge(u32 instr);
  void vrcp(u32 instr);
  void vrsq(u32 instr);
  void vrcpl(u32 instr);
  void vrsql(u32 instr);
  void vrndp(u32 instr);
  void vrndn(u32 instr);
  void vrcph(u32 instr);
  void vsar(u32 instr);
  void vsub(u32 instr);
  void vsubc(u32 instr);
  void vxor(u32 instr);
  void vnxor(u32 instr);
  void vor(u32 instr);
  void vnor(u32 instr);
  void vzero(u32 instr);
  void mfc0(RDP& rdp, u32 instr);
  void mtc0(Registers& regs, Mem& mem, u32 instr);
  void mfc2(u32 instr);
  void mtc2(u32 instr);

  template <bool isDRAMdest>
  FORCE_INLINE void DMA(SPDMALen len, u8* rdram, RSP& rsp, bool bank) {
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

      dram_address += (length + skip);
      dram_address &= 0xFFFFF8;
      mem_address += length;
      mem_address &= 0xFF8;
    }

    rsp.lastSuccessfulSPAddr.address = mem_address;
    rsp.lastSuccessfulSPAddr.bank = bank;
    rsp.lastSuccessfulDRAMAddr.address = dram_address;
    rsp.spDMALen.raw = 0xFF8 | (rsp.spDMALen.skip << 20);
  }

  void WriteStatus(MI& mi, Registers& regs, u32 value);
private:
  FORCE_INLINE void branch(u16 address, bool cond) {
    if(cond) {
      nextPC = address & 0xFFC;
    }
  }

  FORCE_INLINE void branch_likely(u16 address, bool cond) {
    if(cond) {
      nextPC = address & 0xFFC;
    } else {
      pc = nextPC & 0xFFC;
      nextPC = pc + 4;
    }
  }
};
}