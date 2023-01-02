#pragma once
#include <common.hpp>

namespace n64 {
#define STATUS_MASK 0xFF57FFFF
#define CONFIG_MASK 0x0F00800F
#define COP0_REG_INDEX 0
#define COP0_REG_RANDOM 1
#define COP0_REG_ENTRYLO0 2
#define COP0_REG_ENTRYLO1 3
#define COP0_REG_CONTEXT 4
#define COP0_REG_PAGEMASK 5
#define COP0_REG_WIRED 6
#define COP0_REG_BADVADDR 8
#define COP0_REG_COUNT 9
#define COP0_REG_ENTRYHI 10
#define COP0_REG_COMPARE 11
#define COP0_REG_STATUS 12
#define COP0_REG_CAUSE 13
#define COP0_REG_EPC 14
#define COP0_REG_PRID 15
#define COP0_REG_CONFIG 16
#define COP0_REG_LLADDR 17
#define COP0_REG_WATCHLO 18
#define COP0_REG_WATCHHI 19
#define COP0_REG_XCONTEXT 20
#define COP0_REG_PARITY_ERR 26
#define COP0_REG_CACHE_ERR 27
#define COP0_REG_TAGLO 28
#define COP0_REG_TAGHI 29
#define COP0_REG_ERROREPC 30

#define ENTRY_LO_MASK 0x3FFFFFFF
#define ENTRY_HI_MASK 0xC00000FFFFFFE0FF
#define PAGEMASK_MASK 0x1FFE000

struct Cpu;
struct Registers;
struct Mem;

union Cop0Cause {
  u32 raw;
  struct {
    unsigned: 8;
    unsigned interruptPending: 8;
    unsigned: 16;
  } __attribute__((__packed__));
  struct {
    unsigned: 2;
    unsigned exceptionCode: 5;
    unsigned: 1;
    unsigned ip0: 1;
    unsigned ip1: 1;
    unsigned ip2: 1;
    unsigned ip3: 1;
    unsigned ip4: 1;
    unsigned ip5: 1;
    unsigned ip6: 1;
    unsigned ip7: 1;
    unsigned: 12;
    unsigned copError: 2;
    unsigned: 1;
    unsigned branchDelay: 1;
  } __attribute__((__packed__));
};

union Cop0Status {
  struct {
    unsigned ie: 1;
    unsigned exl: 1;
    unsigned erl: 1;
    unsigned ksu: 2;
    unsigned ux: 1;
    unsigned sx: 1;
    unsigned kx: 1;
    unsigned im: 8;
    unsigned ds: 9;
    unsigned re: 1;
    unsigned fr: 1;
    unsigned rp: 1;
    unsigned cu0: 1;
    unsigned cu1: 1;
    unsigned cu2: 1;
    unsigned cu3: 1;
  } __attribute__((__packed__));
  struct {
    unsigned: 16;
    unsigned de: 1;
    unsigned ce: 1;
    unsigned ch: 1;
    unsigned: 1;
    unsigned sr: 1;
    unsigned ts: 1;
    unsigned bev: 1;
    unsigned: 1;
    unsigned its: 1;
    unsigned: 7;
  } __attribute__((__packed__));
  u32 raw;
} __attribute__((__packed__));

union EntryLo {
  u32 raw;
  struct {
    unsigned g:1;
    unsigned v:1;
    unsigned d:1;
    unsigned c:3;
    unsigned pfn:20;
    unsigned:6;
  };
};

union EntryHi {
  u64 raw;
  struct {
    u64 asid:8;
    u64:5;
    u64 vpn2:27;
    u64 fill:22;
    u64 r:2;
  } __attribute__((__packed__));
};

union PageMask {
  u32 raw;
  struct {
    unsigned: 13;
    unsigned mask: 12;
    unsigned: 7;
  };
};

struct TLBEntry {
  bool initialized;
  union {
    u32 raw;
    struct {
      unsigned:1;
      unsigned v:1;
      unsigned d:1;
      unsigned c:3;
      unsigned pfn:20;
      unsigned:6;
    };
  } entryLo0, entryLo1;
  EntryHi entryHi;
  PageMask pageMask;

  bool global;
};

enum TLBError : u8 {
  NONE,
  MISS,
  INVALID,
  MODIFICATION,
  DISALLOWED_ADDRESS
};

enum class ExceptionCode : u8 {
  Interrupt,
  TLBModification,
  TLBLoad,
  TLBStore,
  AddressErrorLoad,
  AddressErrorStore,
  InstructionBusError,
  DataBusError,
  Syscall,
  Breakpoint,
  ReservedInstruction,
  CoprocessorUnusable,
  Overflow,
  Trap,
  FloatingPointError = 15,
  Watch = 23
};

void FireException(Registers& regs, ExceptionCode code, int cop, bool useOldPC);

union Cop0Context {
  u64 raw;
  struct {
    u64: 4;
    u64 badvpn2: 19;
    u64 ptebase: 41;
  };
};

union Cop0XContext {
  u64 raw;
  struct {
    u64: 4;
    u64 badvpn2: 27;
    u64 r: 2;
    u64 ptebase: 31;
  } __attribute__((__packed__));
};

struct Cop0 {
  Cop0();

  u32 GetReg32(u8);
  u64 GetReg64(u8);

  void SetReg32(u8, u32);
  void SetReg64(u8, u64);

  void Reset();

  bool kernel_mode;
  bool supervisor_mode;
  bool user_mode;
  bool is_64bit_addressing;
  bool llbit;

  PageMask pageMask{};
  EntryHi entryHi{};
  EntryLo entryLo0{}, entryLo1{};
  u32 index;
  Cop0Context context{};
  u32 wired, r7{};
  u64 badVaddr{}, count{};
  u32 compare{};
  Cop0Status status{};
  Cop0Cause cause{};
  s64 EPC;
  u32 PRId, Config, LLAddr{}, WatchLo{}, WatchHi{};
  Cop0XContext xcontext{};
  u32 r21{}, r22{}, r23{}, r24{}, r25{}, ParityError{}, CacheError{}, TagLo{}, TagHi{};
  s64 ErrorEPC;
  u32 r31{};
  TLBEntry tlb[32]{};
  TLBError tlbError = NONE;
  s64 openbus{};
  void decode(Registers&, Mem&, u32);
  inline u32 GetRandom() {
    int val = rand();
    int wired = GetWired();
    int lower, upper;
    if(wired > 31) {
      lower = 0;
      upper = 64;
    } else {
      lower = wired;
      upper = 32 - wired;
    }

    val = (val % upper) + lower;
    return val;
  }
private:
  inline u32 GetWired() { return wired & 0x3F; }
  inline u32 GetCount() { return u32(u64(count >> 1)); }

  void mtc0(n64::Registers&, u32);
  void dmtc0(n64::Registers&, u32);
  void mfc0(n64::Registers&, u32);
  void dmfc0(n64::Registers&, u32);
  void eret(n64::Registers&);

  void tlbr(n64::Registers&);
  void tlbw(int, n64::Registers&);
  void tlbp(n64::Registers&);
};

struct Registers;
enum class ExceptionCode : u8;

TLBEntry* TLBTryMatch(Registers& regs, u64 vaddr, int* match);
bool ProbeTLB(Registers& regs, TLBAccessType access_type, u64 vaddr, u32& paddr, int* match);
void HandleTLBException(Registers& regs, u64 vaddr);
ExceptionCode GetTLBExceptionCode(TLBError error, TLBAccessType access_type);
}