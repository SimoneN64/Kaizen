#pragma once
#include <common.hpp>

namespace n64 {
#define STATUS_MASK 0xFF77FFFF

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

enum TLBAccessType {
  LOAD, STORE
};

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

  template<class T>
  T GetReg(u8);

  template<class T>
  void SetReg(u8, T);

  PageMask pageMask{};
  EntryHi entryHi{};
  EntryLo entryLo0{}, entryLo1{};
  u32 index, random;
  Cop0Context context{};
  u32 wired, r7{};
  u64 badVaddr{}, count{};
  u32 compare{};
  Cop0Status status{};
  Cop0Cause cause{};
  u64 EPC;
  u32 PRId, Config, LLAddr{}, WatchLo{}, WatchHi{};
  Cop0XContext xcontext{};
  u32 r21{}, r22{}, r23{}, r24{}, r25{}, ParityError{}, CacheError{}, TagLo{}, TagHi{};
  u64 ErrorEPC;
  u32 r31{};
  TLBEntry tlb[32]{};
  TLBError tlbError = NONE;
  void decode(Registers&, Mem&, u32);
private:
  void mtc0(Registers&, u32);
  void dmtc0(Registers&, u32);
  void mfc0(Registers&, u32);
  void dmfc0(Registers&, u32);
  void eret(Registers&);

  void tlbr(Registers&);
  void tlbwi(Registers&);
  void tlbp(Registers&);
};

struct Registers;
enum class ExceptionCode : u8;

TLBEntry* TLBTryMatch(Registers& regs, u32 vaddr, int* match);
bool ProbeTLB(Registers& regs, TLBAccessType access_type, u32 vaddr, u32& paddr, int* match);
void HandleTLBException(Registers& regs, u64 vaddr);
ExceptionCode GetTLBExceptionCode(TLBError error, TLBAccessType access_type);
}