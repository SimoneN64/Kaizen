#include <Cop0.hpp>
#include <util.hpp>
#include <n64/core/cpu/Registers.hpp>
#include <n64/core/Cpu.hpp>

namespace n64 {
Cop0::Cop0() {
  Reset();
}

void Cop0::Reset() {
  cause.raw = 0xB000007C;
  status.raw = 0x241000E0;
  PRId = 0x00000B22;
  Config = 0x7006E463;
  EPC = 0xFFFFFFFFFFFFFFFF;
  ErrorEPC = 0xFFFFFFFFFFFFFFFF;
}

u32 Cop0::GetReg32(u8 addr) {
  switch(addr) {
    case COP0_REG_INDEX: return index & 0x8000003F;
    case COP0_REG_RANDOM: return GetRandom();
    case COP0_REG_ENTRYLO0: return entryLo0.raw;
    case COP0_REG_ENTRYLO1: return entryLo1.raw;
    case COP0_REG_CONTEXT: return context.raw;
    case COP0_REG_PAGEMASK: return pageMask.raw;
    case COP0_REG_WIRED: return wired;
    case COP0_REG_BADVADDR: return badVaddr;
    case COP0_REG_COUNT: return GetCount();
    case COP0_REG_ENTRYHI: return entryHi.raw;
    case COP0_REG_COMPARE: return compare;
    case COP0_REG_STATUS: return status.raw;
    case COP0_REG_CAUSE: return cause.raw;
    case COP0_REG_EPC: return EPC;
    case COP0_REG_PRID: return PRId;
    case COP0_REG_CONFIG: return Config;
    case COP0_REG_LLADDR: return LLAddr;
    case COP0_REG_WATCHLO: return WatchLo;
    case COP0_REG_WATCHHI: return WatchHi;
    case COP0_REG_XCONTEXT: return xcontext.raw;
    case COP0_REG_PARITY_ERR: return ParityError;
    case COP0_REG_CACHE_ERR: return CacheError;
    case COP0_REG_TAGLO: return TagLo;
    case COP0_REG_TAGHI: return TagHi;
    case COP0_REG_ERROREPC: return ErrorEPC;
    case  7: case 21: case 22:
    case 23: case 24: case 25:
    case 31: return openbus;
    default:
      util::panic("Unsupported word read from COP0 register {}\n", index);
  }
}

u64 Cop0::GetReg64(u8 addr) {
  switch(addr) {
    case COP0_REG_ENTRYLO0: return entryLo0.raw;
    case COP0_REG_ENTRYLO1: return entryLo1.raw;
    case COP0_REG_CONTEXT: return context.raw;
    case COP0_REG_BADVADDR: return badVaddr;
    case COP0_REG_ENTRYHI: return entryHi.raw;
    case COP0_REG_STATUS: return status.raw;
    case COP0_REG_EPC: return EPC;
    case COP0_REG_PRID: return PRId;
    case COP0_REG_LLADDR: return LLAddr;
    case COP0_REG_XCONTEXT: return xcontext.raw & 0xFFFFFFFFFFFFFFF0;
    case COP0_REG_ERROREPC: return ErrorEPC;
    case  7: case 21: case 22:
    case 23: case 24: case 25:
    case 31: return openbus;
    default:
      util::panic("Unsupported word read from COP0 register {}\n", index);
  }
}

void Cop0::SetReg32(u8 addr, u32 value) {
  openbus = value & 0xFFFFFFFF;
  switch(addr) {
    case COP0_REG_INDEX: index = value; break;
    case COP0_REG_RANDOM: break;
    case COP0_REG_ENTRYLO0:
      entryLo0.raw = value & ENTRY_LO_MASK;
      break;
    case COP0_REG_ENTRYLO1:
      entryLo1.raw = value & ENTRY_LO_MASK;
      break;
    case COP0_REG_CONTEXT:
      context.raw = (s64(s32(value)) & 0xFFFFFFFFFF800000) | (context.raw & 0x7FFFFF);
      break;
    case COP0_REG_PAGEMASK: pageMask.raw = value & PAGEMASK_MASK; break;
    case COP0_REG_WIRED: wired = value & 63; break;
    case COP0_REG_BADVADDR: break;
    case COP0_REG_COUNT: count = (u64)value << 1; break;
    case COP0_REG_ENTRYHI:
      entryHi.raw = s64(s32(value)) & ENTRY_HI_MASK;
      break;
    case COP0_REG_COMPARE:
      compare = value;
      cause.ip7 = false;
      break;
    case COP0_REG_STATUS:
      status.raw &= ~STATUS_MASK;
      status.raw |= (value & STATUS_MASK);
      break;
    case COP0_REG_CAUSE: {
      Cop0Cause temp{};
      temp.raw = value;
      cause.ip0 = temp.ip0;
      cause.ip1 = temp.ip1;
    } break;
    case COP0_REG_EPC:
      EPC = s64(s32(value));
      break;
    case COP0_REG_PRID: break;
    case COP0_REG_CONFIG: {
      Config &= ~CONFIG_MASK;
      Config |= (value & CONFIG_MASK);
    } break;
    case COP0_REG_LLADDR: LLAddr = value; break;
    case COP0_REG_WATCHLO: WatchLo = value; break;
    case COP0_REG_WATCHHI: WatchHi = value; break;
    case COP0_REG_XCONTEXT:
      xcontext.raw = (s64(s32(value)) & 0xFFFFFFFE00000000) | (xcontext.raw & 0x1FFFFFFFF);
      break;
    case COP0_REG_PARITY_ERR: ParityError = value & 0xff; break;
    case COP0_REG_CACHE_ERR: break;
    case COP0_REG_TAGLO: TagLo = value; break;
    case COP0_REG_TAGHI: TagHi = value; break;
    case COP0_REG_ERROREPC: ErrorEPC = s64(s32(value)); break;
    case  7: case 21: case 22:
    case 23: case 24: case 25:
    case 31: break;
    default:
      util::panic("Unsupported word read from COP0 register {}\n", index);
  }
}

void Cop0::SetReg64(u8 addr, u64 value) {
  openbus = value;
  switch(addr) {
    case COP0_REG_ENTRYLO0: entryLo0.raw = value & ENTRY_LO_MASK; break;
    case COP0_REG_ENTRYLO1: entryLo1.raw = value & ENTRY_LO_MASK; break;
    case COP0_REG_CONTEXT:
      context.raw = (value & 0xFFFFFFFFFF800000) | (context.raw & 0x7FFFFF);
      break;
    case COP0_REG_XCONTEXT:
      xcontext.raw = (value & 0xFFFFFFFE00000000) | (xcontext.raw & 0x1FFFFFFFF);
      break;
    case COP0_REG_ENTRYHI: entryHi.raw = value & ENTRY_HI_MASK; break;
    case COP0_REG_STATUS: status.raw = value; break;
    case COP0_REG_CAUSE: {
      Cop0Cause tmp{};
      tmp.raw = value;
      cause.ip0 = tmp.ip0;
      cause.ip1 = tmp.ip1;
    } break;
    case COP0_REG_BADVADDR: break;
    case COP0_REG_EPC: EPC = (s64)value; break;
    case COP0_REG_LLADDR: LLAddr = value; break;
    case COP0_REG_ERROREPC: ErrorEPC = (s64)value; break;
    default:
      util::panic("Unsupported word write to COP0 register {}\n", addr);
  }
}

#define vpn(addr, PageMask) (((((addr) & 0xFFFFFFFFFF) | (((addr) >> 22) & 0x30000000000)) & ~((PageMask) | 0x1FFF)))

TLBEntry* TLBTryMatch(Registers& regs, s64 vaddr, int* match) {
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

bool ProbeTLB(Registers& regs, TLBAccessType access_type, s64 vaddr, u32& paddr, int* match) {
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