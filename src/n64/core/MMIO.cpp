#include <n64/core/MMIO.hpp>
#include <util.hpp>
#include <n64/core/Mem.hpp>
#include <n64/core/cpu/Registers.hpp>

namespace n64 {
MMIO::MMIO () {
  Reset();
}

void MMIO::Reset() {
  rsp.Reset();
  rdp.Reset();
  mi.Reset();
  vi.Reset();
  ai.Reset();
  pi.Reset();
  ri.Reset();
  si.Reset();
}

template <bool crashOnUnimplemented>
u32 MMIO::Read(u32 addr) {
  switch (addr) {
    case 0x04040000 ... 0x040FFFFF: return rsp.Read<crashOnUnimplemented>(addr);
    case 0x04100000 ... 0x041FFFFF: return rdp.Read<crashOnUnimplemented>(addr);
    case 0x04300000 ... 0x043FFFFF: return mi.Read<crashOnUnimplemented>(addr);
    case 0x04400000 ...	0x044FFFFF: return vi.Read<crashOnUnimplemented>(addr);
    case 0x04500000 ... 0x045FFFFF: return ai.Read(addr);
    case 0x04600000 ... 0x046FFFFF: return pi.Read<crashOnUnimplemented>(mi, addr);
    case 0x04700000 ... 0x047FFFFF: return ri.Read<crashOnUnimplemented>(addr);
    case 0x04800000 ... 0x048FFFFF: return si.Read<crashOnUnimplemented>(mi, addr);
    default:
      if constexpr (crashOnUnimplemented) {
        util::panic("Unhandled mmio read at addr {:08X}\n", addr);
      }
      return 0;
  }
}

template u32 MMIO::Read<true>(u32);
template u32 MMIO::Read<false>(u32);

template <bool crashOnUnimplemented>
void MMIO::Write(Mem& mem, Registers& regs, u32 addr, u32 val) {
  switch (addr) {
    case 0x04040000 ... 0x040FFFFF: rsp.Write<crashOnUnimplemented>(mem, regs, addr, val); break;
    case 0x04100000 ... 0x041FFFFF: rdp.Write<crashOnUnimplemented>(mi, regs, rsp, addr, val); break;
    case 0x04300000 ... 0x043FFFFF: mi.Write<crashOnUnimplemented>(regs, addr, val); break;
    case 0x04400000 ...	0x044FFFFF: vi.Write<crashOnUnimplemented>(mi, regs, addr, val); break;
    case 0x04500000 ... 0x045FFFFF: ai.Write<crashOnUnimplemented>(mem, regs, addr, val); break;
    case 0x04600000 ... 0x046FFFFF: pi.Write<crashOnUnimplemented>(mem, regs, addr, val); break;
    case 0x04700000 ... 0x047FFFFF: ri.Write<crashOnUnimplemented>(addr, val); break;
    case 0x04800000 ... 0x048FFFFF: si.Write<crashOnUnimplemented>(mem, regs, addr, val); break;
    default:
      if constexpr (crashOnUnimplemented) {
        util::panic("Unhandled mmio write at addr {:08X} with val {:08X}\n", addr, val);
      }
  }
}

template void MMIO::Write<true>(Mem&, Registers&, u32, u32);
template void MMIO::Write<false>(Mem&, Registers&, u32, u32);
}
