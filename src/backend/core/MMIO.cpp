#include <core/MMIO.hpp>
#include <log.hpp>
#include <core/Mem.hpp>
#include <core/registers/Registers.hpp>

namespace n64 {
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

u32 MMIO::Read(u32 addr) {
  switch (addr) {
    case RSP_REGION: return rsp.Read(addr);
    case RDP_REGION: return rdp.Read(addr);
    case MI_REGION: return mi.Read(addr);
    case VI_REGION: return vi.Read(addr);
    case AI_REGION: return ai.Read(addr);
    case PI_REGION: return pi.Read(mi, addr);
    case RI_REGION: return ri.Read(addr);
    case SI_REGION: return si.Read(mi, addr);
    default:
      Util::panic("Unhandled mmio read at addr {:08X}", addr);
  }
}

void MMIO::Write(Mem& mem, Registers& regs, u32 addr, u32 val) {
  switch (addr) {
    case RSP_REGION: rsp.Write(mem, regs, addr, val); break;
    case RDP_REGION: rdp.Write(mi, regs, rsp, addr, val); break;
    case MI_REGION: mi.Write(regs, addr, val); break;
    case VI_REGION: vi.Write(mi, regs, addr, val); break;
    case AI_REGION: ai.Write(mem, regs, addr, val); break;
    case PI_REGION: pi.Write(mem, regs, addr, val); break;
    case RI_REGION: ri.Write(addr, val); break;
    case SI_REGION: si.Write(mem, regs, addr, val); break;
    default:
      Util::panic("Unhandled mmio write at addr {:08X} with val {:08X}", addr, val);
  }
}
}
