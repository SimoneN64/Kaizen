#include <Cop0.hpp>
#include <util.hpp>
#include <n64/core/cpu/Registers.hpp>
#include <n64/core/Cpu.hpp>

namespace natsukashii::n64::core {
Cop0::Cop0() {
  cause.raw = 0xB000007C;
  random = 0x0000001F;
  status.raw = 0x241000E0;
  wired = 64;
  index = 64;
  PRId = 0x00000B00;
  Config = 0x7006E463;
  EPC = 0xFFFFFFFFFFFFFFFF;
  ErrorEPC = 0xFFFFFFFFFFFFFFFF;
}

template<class T>
T Cop0::GetReg(u8 addr) {
  switch(addr) {
    case 0: return index & 0x8000001F;
    case 1: return random & 0x1F;
    case 2: return entryLo0.raw & 0x3FFFFFFF;
    case 3: return entryLo1.raw & 0x3FFFFFFF;
    case 4: return context.raw;
    case 5: return pageMask.raw;
    case 6: return wired & 0x3F;
    case 7: return r7;
    case 8: return badVaddr;
    case 9: return count >> 1;
    case 10: return entryHi.raw & 0xFFFFFFFFFFFFE0FF;
    case 11: return compare;
    case 12: return status.raw & STATUS_MASK;
    case 13: return cause.raw;
    case 14: return EPC;
    case 15: return PRId & 0xFFFF;
    case 16: return Config;
    case 17: return LLAddr;
    case 18: return WatchLo;
    case 19: return WatchHi;
    case 20: return xcontext.raw & 0xFFFFFFF0;
    case 21: return r21;
    case 22: return r22;
    case 23: return r23;
    case 24: return r24;
    case 25: return r25;
    case 26: return ParityError;
    case 27: return CacheError;
    case 28: return TagLo & 0xFFFFFFF;
    case 29: return TagHi;
    case 30: return ErrorEPC;
    case 31: return r31;
    default:
      util::panic("Unsupported word read from COP0 register {}\n", index);
  }
}

template<class T>
void Cop0::SetReg(u8 addr, T value) {
  switch(addr) {
    case 0: index = value & 0x8000001F; break;
    case 1: random = value & 0x1F; break;
    case 2: entryLo0.raw = value & 0x3FFFFFFF; break;
    case 3: entryLo1.raw = value & 0x3FFFFFFF; break;
    case 4: context.raw = value; break;
    case 5: pageMask.raw = value; break;
    case 6: wired = value & 0x3F; break;
    case 7: r7 = value; break;
    case 9: count = value << 1; break;
    case 10: entryHi.raw = value & 0xFFFFFFFFFFFFE0FF; break;
    case 11: {
      cause.ip7 = 0;
      compare = value;
    } break;
    case 12: status.raw = value & STATUS_MASK; break;
    case 13: {
      Cop0Cause tmp{};
      tmp.raw = value;
      cause.ip0 = tmp.ip0;
      cause.ip1 = tmp.ip1;
    } break;
    case 14: EPC = value; break;
    case 15: PRId = value & 0xFFFF; break;
    case 16: Config = value; break;
    case 17: LLAddr = value; break;
    case 18: WatchLo = value; break;
    case 19: WatchHi = value; break;
    case 21: r21 = value; break;
    case 22: r22 = value; break;
    case 23: r23 = value; break;
    case 24: r24 = value; break;
    case 25: r25 = value; break;
    case 26: ParityError = value; break;
    case 27: CacheError = value; break;
    case 28: TagLo = value & 0xFFFFFFF; break;
    case 29: TagHi = value; break;
    case 30: ErrorEPC = value; break;
    case 31: r31 = value; break;
    default:
      util::panic("Unsupported word write to COP0 register {}\n", index);
  }
}

#define vpn(addr, PageMask) (((((addr) & 0xFFFFFFFFFF) | (((addr) >> 22) & 0x30000000000)) & ~((PageMask) | 0x1FFF)))

TLBEntry* TLBTryMatch(Registers& regs, u32 vaddr, int* match) {
  for(int i = 0; i < 32; i++) {
    TLBEntry *entry = &regs.cop0.tlb[i];
    u64 entry_vpn = vpn(entry->entryHi.raw, entry->pageMask.raw);
    u64 vaddr_vpn = vpn(vaddr, entry->pageMask.raw);

    bool vpn_match = entry_vpn == vaddr_vpn;
    bool asid_match = entry->global || (regs.cop0.entryHi.asid == entry->entryHi.asid);

    if(vpn_match && asid_match) {
      if(match) {
        *match = i;
      }
      return entry;
    }
  }

  return nullptr;
}

bool ProbeTLB(Registers& regs, TLBAccessType access_type, u32 vaddr, u32& paddr, int* match) {
  TLBEntry* entry = TLBTryMatch(regs, vaddr, match);
  if(!entry) {
    regs.cop0.tlbError = MISS;
    return false;
  }

  u32 mask = (entry->pageMask.mask << 12) | 0xFFF;
  u32 odd = vaddr & (mask + 1);
  u32 pfn;

  if(!odd) {
    if(!(entry->entryLo0.v)) {
      regs.cop0.tlbError = INVALID;
      return false;
    }

    if(access_type == STORE && !(entry->entryLo0.d)) {
      regs.cop0.tlbError = MODIFICATION;
      return false;
    }

    pfn = entry->entryLo0.pfn;
  } else {
    if(!(entry->entryLo1.v)) {
      regs.cop0.tlbError = INVALID;
      return false;
    }

    if(access_type == STORE && !(entry->entryLo1.d)) {
      regs.cop0.tlbError = MODIFICATION;
      return false;
    }

    pfn = entry->entryLo1.pfn;
  }

  paddr = (pfn << 12) | (vaddr & mask);

  return true;
}

void HandleTLBException(Registers& regs, u64 vaddr) {
  u64 vpn2 = (vaddr >> 13) & 0x7FFFF;
  u64 xvpn2 = (vaddr >> 13) & 0x7FFFFFF;
  regs.cop0.badVaddr = vaddr;
  regs.cop0.context.badvpn2 = vpn2;
  regs.cop0.xcontext.badvpn2 = xvpn2;
  regs.cop0.xcontext.r = (vaddr >> 62) & 3;
  regs.cop0.entryHi.vpn2 = xvpn2;
  regs.cop0.entryHi.r = (vaddr >> 62) & 3;
}

ExceptionCode GetTLBExceptionCode(TLBError error, TLBAccessType accessType) {
  switch(error) {
    case NONE: util::panic("Getting TLB exception with error NONE\n");
    case INVALID: case MISS:
      return accessType == LOAD ?
        ExceptionCode::TLBLoad : ExceptionCode::TLBStore;
    case MODIFICATION:
      return ExceptionCode::TLBModification;
    case DISALLOWED_ADDRESS:
      return accessType == LOAD ?
        ExceptionCode::AddressErrorLoad : ExceptionCode::AddressErrorStore;
    default:
      util::panic("Getting TLB exception for unknown error code! ({})\n", error);
  }
}

void Cop0::decode(Registers& regs, Mem& mem, u32 instr) {
  u8 mask_cop = (instr >> 21) & 0x1F;
  u8 mask_cop2 = instr & 0x3F;
  switch(mask_cop) {
    case 0x00: mfc0(regs, instr); break;
    case 0x01: dmfc0(regs, instr); break;
    case 0x04: mtc0(regs, instr); break;
    case 0x05: dmtc0(regs, instr); break;
    case 0x10 ... 0x1F:
      switch(mask_cop2) {
        case 0x01: tlbr(regs); break;
        case 0x02: tlbwi(regs); break;
        case 0x08: tlbp(regs); break;
        case 0x18: eret(regs); break;
        default: util::panic("Unimplemented COP0 function {} {} ({:08X}) ({:016lX})", mask_cop2 >> 3, mask_cop2 & 7, instr, regs.oldPC);
      }
      break;
    default: util::panic("Unimplemented COP0 instruction {} {}", mask_cop >> 4, mask_cop & 7);
  }
}
}