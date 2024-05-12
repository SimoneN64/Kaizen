#include <log.hpp>
#include <core/registers/Registers.hpp>
#include <core/Interpreter.hpp>

namespace n64 {
Cop0::Cop0(Registers& regs) : regs(regs) {
  Reset();
}

void Cop0::Reset() {
  cause.raw = 0xB000007C;
  status.raw = 0;
  status.cu0 = 1;
  status.cu1 = 1;
  status.fr = 1;
  PRId = 0x00000B22;
  Config = 0x7006E463;
  EPC = 0xFFFFFFFFFFFFFFFFll;
  ErrorEPC = 0xFFFFFFFFFFFFFFFFll;
  wired = 0;
  index.raw = 63;
  badVaddr = 0xFFFFFFFFFFFFFFFF;
}

u32 Cop0::GetReg32(u8 addr) {
  switch(addr) {
    case COP0_REG_INDEX: return index.raw & INDEX_MASK;
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
      Util::panic("Unsupported word read from COP0 register {}", addr);
  }
}

u64 Cop0::GetReg64(u8 addr) const {
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
      Util::panic("Unsupported dword read from COP0 register {}", addr);
  }
}

void Cop0::SetReg32(u8 addr, u32 value) {
  openbus = value & 0xFFFFFFFF;
  switch(addr) {
    case COP0_REG_INDEX: index.raw = value & INDEX_MASK; break;
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
      Update();
      break;
    case COP0_REG_CAUSE: {
      Cop0Cause tmp{};
      tmp.raw = value;
      cause.ip0 = tmp.ip0;
      cause.ip1 = tmp.ip1;
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
      Util::panic("Unsupported word write from COP0 register {}", addr);
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
      Util::panic("Unsupported dword write to COP0 register {}", addr);
  }
}

static FORCE_INLINE u64 getVPN(u64 addr, u64 pageMask) {
  u64 mask = pageMask | 0x1fff;
  u64 vpn = (addr & 0xFFFFFFFFFF) | ((addr >> 22) & 0x30000000000);

  return vpn & ~mask;
}

TLBEntry* Cop0::TLBTryMatch(u64 vaddr, int* match) {
  for(int i = 0; i < 32; i++) {
    TLBEntry *entry = &regs.cop0.tlb[i];
    if(entry->initialized) {
      u64 entry_vpn = getVPN(entry->entryHi.raw, entry->pageMask.raw);
      u64 vaddr_vpn = getVPN(vaddr, entry->pageMask.raw);

      bool vpn_match = entry_vpn == vaddr_vpn;
      bool asid_match = entry->global || (regs.cop0.entryHi.asid == entry->entryHi.asid);

      if (vpn_match && asid_match) {
        if (match) {
          *match = i;
        }
        return entry;
      }
    }
  }

  return nullptr;
}

bool Cop0::ProbeTLB(TLBAccessType access_type, u64 vaddr, u32& paddr, int* match) {
  TLBEntry* entry = TLBTryMatch(vaddr, match);
  if(!entry) {
    regs.cop0.tlbError = MISS;
    return false;
  }

  u32 mask = (entry->pageMask.mask << 12) | 0xFFF;
  u32 odd = vaddr & (mask + 1);
  u32 pfn;

  if(!odd) {
    if(!entry->entryLo0.v) {
      regs.cop0.tlbError = INVALID;
      return false;
    }

    if(access_type == STORE && !entry->entryLo0.d) {
      regs.cop0.tlbError = MODIFICATION;
      return false;
    }

    pfn = entry->entryLo0.pfn;
  } else {
    if(!entry->entryLo1.v) {
      regs.cop0.tlbError = INVALID;
      return false;
    }

    if(access_type == STORE && !entry->entryLo1.d) {
      regs.cop0.tlbError = MODIFICATION;
      return false;
    }

    pfn = entry->entryLo1.pfn;
  }

  paddr = (pfn << 12) | (vaddr & mask);

  return true;
}

FORCE_INLINE bool Is64BitAddressing(Cop0& cp0, u64 addr) {
  u8 region = (addr >> 62) & 3;
  switch(region) {
    case 0b00: return cp0.status.ux;
    case 0b01: return cp0.status.sx;
    case 0b11: return cp0.status.kx;
    default: return false;
  }
}

void Cop0::FireException(ExceptionCode code, int cop, s64 pc) {
  bool old_exl = regs.cop0.status.exl;

  if(!regs.cop0.status.exl) {
    if((regs.cop0.cause.branchDelay = regs.prevDelaySlot)) {
      pc -= 4;
    }

    regs.cop0.status.exl = true;
    regs.cop0.EPC = pc;
  }

  regs.cop0.cause.copError = cop;
  regs.cop0.cause.exceptionCode = static_cast<u8>(code);

  if(regs.cop0.status.bev) {
    Util::panic("BEV bit set!");
  } else {
    switch(code) {
      case ExceptionCode::Interrupt: case ExceptionCode::TLBModification:
      case ExceptionCode::AddressErrorLoad: case ExceptionCode::AddressErrorStore:
      case ExceptionCode::InstructionBusError: case ExceptionCode::DataBusError:
      case ExceptionCode::Syscall: case ExceptionCode::Breakpoint:
      case ExceptionCode::ReservedInstruction: case ExceptionCode::CoprocessorUnusable:
      case ExceptionCode::Overflow: case ExceptionCode::Trap:
      case ExceptionCode::FloatingPointError: case ExceptionCode::Watch:
        regs.SetPC32(s32(0x80000180));
        break;
      case ExceptionCode::TLBLoad: case ExceptionCode::TLBStore:
        if(old_exl || regs.cop0.tlbError == INVALID) {
          regs.SetPC32(s32(0x80000180));
        } else if(Is64BitAddressing(regs.cop0, regs.cop0.badVaddr)) {
          regs.SetPC32(s32(0x80000080));
        } else {
          regs.SetPC32(s32(0x80000000));
        }
        break;
      default: Util::panic("Unhandled exception! {}", static_cast<u8>(code));
    }
  }

  regs.cop0.Update();
}

void Cop0::HandleTLBException(u64 vaddr) {
  u64 vpn2 = (vaddr >> 13) & 0x7FFFF;
  u64 xvpn2 = (vaddr >> 13) & 0x7FFFFFF;
  regs.cop0.badVaddr = vaddr;
  regs.cop0.context.badvpn2 = vpn2;
  regs.cop0.xcontext.badvpn2 = xvpn2;
  regs.cop0.xcontext.r = (vaddr >> 62) & 3;
  regs.cop0.entryHi.vpn2 = xvpn2;
  regs.cop0.entryHi.r = (vaddr >> 62) & 3;
}

ExceptionCode Cop0::GetTLBExceptionCode(TLBError error, TLBAccessType accessType) {
  switch(error) {
    case NONE: Util::panic("Getting TLB exception with error NONE");
    case INVALID: case MISS:
      return accessType == LOAD ?
        ExceptionCode::TLBLoad : ExceptionCode::TLBStore;
    case MODIFICATION:
      return ExceptionCode::TLBModification;
    case DISALLOWED_ADDRESS:
      return accessType == LOAD ?
        ExceptionCode::AddressErrorLoad : ExceptionCode::AddressErrorStore;
    default:
      Util::panic("Getting TLB exception for unknown error code! ({})", static_cast<u8>(error));
  }
}

template<class T>
void Cop0::decode(T& cpu, u32 instr) {
  if constexpr (std::is_same_v<decltype(cpu), Interpreter&>) {
    decodeInterp(instr);
  } else if constexpr (std::is_same_v<decltype(cpu), JIT&>) {
    decodeJIT(cpu, instr);
  } else {
    Util::panic("What the fuck did you just give me?!!");
  }
}

template void Cop0::decode<Interpreter>(Interpreter&, u32);
template void Cop0::decode<JIT>(JIT&, u32);

void Cop0::decodeJIT(JIT& cpu, u32 instr) {

}

void Cop0::decodeInterp(u32 instr) {
  u8 mask_cop = (instr >> 21) & 0x1F;
  u8 mask_cop2 = instr & 0x3F;
  switch(mask_cop) {
    case 0x00: mfc0(instr); break;
    case 0x01: dmfc0(instr); break;
    case 0x04: mtc0(instr); break;
    case 0x05: dmtc0(instr); break;
    case 0x10 ... 0x1F:
      switch(mask_cop2) {
        case 0x01: tlbr(); break;
        case 0x02: tlbw(index.i); break;
        case 0x06: tlbw(GetRandom()); break;
        case 0x08: tlbp(); break;
        case 0x18: eret(); break;
        default: Util::panic("Unimplemented COP0 function {} {} ({:08X}) ({:016lX})", mask_cop2 >> 3, mask_cop2 & 7, instr, regs.oldPC);
      }
      break;
    default: Util::panic("Unimplemented COP0 instruction {} {}", mask_cop >> 4, mask_cop & 7);
  }
}

bool Cop0::MapVAddr(TLBAccessType accessType, u64 vaddr, u32& paddr) {
  if(regs.cop0.is64BitAddressing) [[unlikely]] {
    if (regs.cop0.kernelMode) [[likely]] {
      return MapVAddr64(accessType, vaddr, paddr);
    } else if (regs.cop0.userMode) {
      return UserMapVAddr64(accessType, vaddr, paddr);
    } else if (regs.cop0.supervisorMode) {
      Util::panic("Supervisor mode memory access, 64 bit mode");
    } else {
      Util::panic("Unknown mode! This should never happen!");
    }
  } else {
    if (regs.cop0.kernelMode) [[likely]] {
      return MapVAddr32(accessType, vaddr, paddr);
    } else if (regs.cop0.userMode) {
      return UserMapVAddr32(accessType, vaddr, paddr);
    } else if (regs.cop0.supervisorMode) {
      Util::panic("Supervisor mode memory access, 32 bit mode");
    } else {
      Util::panic("Unknown mode! This should never happen!");
    }
  }
}

bool Cop0::UserMapVAddr32(TLBAccessType accessType, u64 vaddr, u32& paddr) {
  switch (vaddr) {
    case VREGION_KUSEG:
      return ProbeTLB(accessType, s64(s32(vaddr)), paddr, nullptr);
    default:
      regs.cop0.tlbError = DISALLOWED_ADDRESS;
      return false;
  }
}

bool Cop0::MapVAddr32(TLBAccessType accessType, u64 vaddr, u32& paddr) {
  switch((u32(vaddr) >> 29) & 7) {
    case 0 ... 3: case 7:
      return ProbeTLB(accessType, s64(s32(vaddr)), paddr, nullptr);
    case 4 ... 5:
      paddr = vaddr & 0x1FFFFFFF;
      return true;
    case 6: Util::panic("Unimplemented virtual mapping in KSSEG! ({:08X})", vaddr);
    default:
      Util::panic("Should never end up in default case in map_vaddr! ({:08X})", vaddr);
  }

  return false;
}

bool Cop0::UserMapVAddr64(TLBAccessType accessType, u64 vaddr, u32& paddr) {
  switch (vaddr) {
    case VREGION_XKUSEG:
      return ProbeTLB(accessType, vaddr, paddr, nullptr);
    default:
      regs.cop0.tlbError = DISALLOWED_ADDRESS;
      return false;
  }
}

bool Cop0::MapVAddr64(TLBAccessType accessType, u64 vaddr, u32& paddr) {
  switch (vaddr) {
    case VREGION_XKUSEG: case VREGION_XKSSEG:
      return ProbeTLB(accessType, vaddr, paddr, nullptr);
    case VREGION_XKPHYS: {
      if (!regs.cop0.kernelMode) {
        Util::panic("Access to XKPHYS address 0x{:016X} when outside kernel mode!", vaddr);
      }
      u8 high_two_bits = (vaddr >> 62) & 0b11;
      if (high_two_bits != 0b10) {
        Util::panic("Access to XKPHYS address 0x{:016X} with high two bits != 0b10!", vaddr);
      }
      u8 subsegment = (vaddr >> 59) & 0b11;
      bool cached = subsegment != 2; // do something with this eventually
      // If any bits in the range of 58:32 are set, the address is invalid.
      bool valid = (vaddr & 0x07FFFFFF00000000) == 0;
      if (!valid) {
        regs.cop0.tlbError = DISALLOWED_ADDRESS;
        return false;
      }
      paddr = vaddr & 0xFFFFFFFF;
      return true;
    }
    case VREGION_XKSEG:
      return ProbeTLB(accessType, vaddr, paddr, nullptr);
    case VREGION_CKSEG0:
      // Identical to kseg0 in 32 bit mode.
      // Unmapped translation. Subtract the base address of the space to get the physical address.
      paddr = vaddr - START_VREGION_KSEG0; // Implies cutting off the high 32 bits
      Util::trace("CKSEG0: Translated 0x{:016X} to 0x{:08X}", vaddr, paddr);
      return true;
    case VREGION_CKSEG1:
      // Identical to kseg1 in 32 bit mode.
      // Unmapped translation. Subtract the base address of the space to get the physical address.
      paddr = vaddr - START_VREGION_KSEG1; // Implies cutting off the high 32 bits
      Util::trace("KSEG1: Translated 0x{:016X} to 0x{:08X}", vaddr, paddr);
      return true;
    case VREGION_CKSSEG:
      Util::panic("Resolving virtual address 0x{:016X} (VREGION_CKSSEG) in 64 bit mode", vaddr);
    case VREGION_CKSEG3:
      return ProbeTLB(accessType, vaddr, paddr, nullptr);
    case VREGION_XBAD1:
    case VREGION_XBAD2:
    case VREGION_XBAD3:
      regs.cop0.tlbError = DISALLOWED_ADDRESS;
      return false;
    default:
      Util::panic("Resolving virtual address 0x{:016X} in 64 bit mode", vaddr);
  }

  return false;
}
}