#include <MMIO.hpp>
#include <util.hpp>
#include <n64/core/Mem.hpp>
#include <n64/core/cpu/Registers.hpp>

namespace natsukashii::n64::core {
u32 MMIO::Read(u32 addr) {
  switch (addr) {
    case 0x04040000 ... 0x040FFFFF: return rsp.Read(addr);
    case 0x04100000 ... 0x041FFFFF: return rdp.Read(addr);
    case 0x04300000 ... 0x043FFFFF: return mi.Read(addr);
    case 0x04400000 ...	0x044FFFFF: return vi.Read(addr);
    case 0x04500000 ... 0x045FFFFF: return ai.Read(addr);
    case 0x04600000 ... 0x046FFFFF: return pi.Read(mi, addr);
    case 0x04700000 ... 0x047FFFFF: return ri.Read(addr);
    case 0x04800000 ... 0x048FFFFF: return si.Read(mi, addr);
    default: util::panic("Unhandled mmio read at addr {:08X}\n", addr);
  }
}

void MMIO::Write(Mem& mem, Registers& regs, u32 addr, u32 val) {
  switch (addr) {
    case 0x04040000 ... 0x040FFFFF: rsp.Write(mem, regs, addr, val); break;
    case 0x04100000 ... 0x041FFFFF: rdp.Write(addr, val); break;
    case 0x04300000 ... 0x043FFFFF: mi.Write(regs, addr, val); break;
    case 0x04400000 ...	0x044FFFFF: vi.Write(mi, regs, addr, val); break;
    case 0x04500000 ... 0x045FFFFF: ai.Write(mem, regs, addr, val); break;
    case 0x04600000 ... 0x046FFFFF: pi.Write(mem, regs, addr, val); break;
    case 0x04700000 ... 0x047FFFFF: ri.Write(addr, val); break;
    case 0x04800000 ... 0x048FFFFF: si.Write(mem, regs, addr, val); break;
    default: util::panic("Unhandled mmio write at addr {:08X} with val {:08X}\n", addr, val);
  }
}
}
