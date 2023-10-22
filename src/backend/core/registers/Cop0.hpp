#pragma once
#include <common.hpp>
#include "log.hpp"

namespace n64 {
#define STATUS_MASK 0xFF57FFFF
#define CONFIG_MASK 0x0F00800F
#define INDEX_MASK 0x8000003F
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

struct JIT;
struct Interpreter;
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

union Index {
  u32 raw;
  struct {
    unsigned i:6;
    unsigned:25;
    unsigned p:1;
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
  Interrupt = 0,
  TLBModification = 1,
  TLBLoad = 2,
  TLBStore = 3,
  AddressErrorLoad = 4,
  AddressErrorStore = 5,
  InstructionBusError = 6,
  DataBusError = 7,
  Syscall = 8,
  Breakpoint = 9,
  ReservedInstruction = 10,
  CoprocessorUnusable = 11,
  Overflow = 12,
  Trap = 13,
  FloatingPointError = 15,
  Watch = 23
};

void FireException(Registers& regs, ExceptionCode code, int cop, s64 pc);

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
  u64 GetReg64(u8) const;

  void SetReg32(u8, u32);
  void SetReg64(u8, u64);

  void Reset();

  bool kernel_mode{true};
  bool supervisor_mode{false};
  bool user_mode{false};
  bool is_64bit_addressing{false};
  bool llbit{};

  PageMask pageMask{};
  EntryHi entryHi{};
  EntryLo entryLo0{}, entryLo1{};
  Index index{};
  Cop0Context context{};
  u32 wired{}, r7{};
  u64 badVaddr{}, count{};
  u32 compare{};
  Cop0Status status{};
  Cop0Cause cause{};
  s64 EPC{};
  u32 PRId{}, Config{}, LLAddr{}, WatchLo{}, WatchHi{};
  Cop0XContext xcontext{};
  u32 r21{}, r22{}, r23{}, r24{}, r25{}, ParityError{}, CacheError{}, TagLo{}, TagHi{};
  s64 ErrorEPC{};
  u32 r31{};
  TLBEntry tlb[32]{};
  TLBError tlbError = NONE;
  s64 openbus{};
  template <class T>
  void decode(T&, u32);
  FORCE_INLINE u32 GetRandom() {
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

  FORCE_INLINE void Update() {
    bool exception = status.exl || status.erl;

    kernel_mode     =  exception || status.ksu == 0;
    supervisor_mode = !exception && status.ksu == 1;
    user_mode       = !exception && status.ksu == 2;
    is_64bit_addressing =
      (kernel_mode && status.kx)
      || (supervisor_mode && status.sx)
      || (user_mode && status.ux);
  }
private:
  FORCE_INLINE u32 GetWired() { return wired & 0x3F; }
  FORCE_INLINE u32 GetCount() { return u32(u64(count >> 1)); }

  void decodeInterp(Registers&, u32);
  void decodeJIT(JIT&, u32);
  void mtc0(Registers&, u32);
  void dmtc0(Registers&, u32);
  void mfc0(Registers&, u32);
  void dmfc0(Registers&, u32);
  void eret(Registers&);

  void tlbr();
  void tlbw(int);
  void tlbp(n64::Registers&);
};

struct Registers;
enum class ExceptionCode : u8;

enum TLBAccessType {
  LOAD, STORE
};

bool ProbeTLB(Registers& regs, TLBAccessType access_type, u64 vaddr, u32& paddr, int* match);

bool MapVAddr(Registers& regs, TLBAccessType accessType, u64 vaddr, u32& paddr);
bool UserMapVAddr32(Registers& regs, TLBAccessType accessType, u64 vaddr, u32& paddr);
bool MapVAddr32(Registers& regs, TLBAccessType accessType, u64 vaddr, u32& paddr);
bool UserMapVAddr64(Registers& regs, TLBAccessType accessType, u64 vaddr, u32& paddr);
bool MapVAddr64(Registers& regs, TLBAccessType accessType, u64 vaddr, u32& paddr);

TLBEntry* TLBTryMatch(Registers& regs, u64 vaddr, int* match);
void HandleTLBException(Registers& regs, u64 vaddr);
ExceptionCode GetTLBExceptionCode(TLBError error, TLBAccessType access_type);
}