#include <Scheduler.hpp>
#include <core/Mem.hpp>
#include <core/mmio/SI.hpp>

namespace n64 {
SI::SI(Mem &mem, Registers &regs) : mem(mem), regs(regs), pif(mem, regs) { Reset(); }

void SI::Reset() {
  status.raw = 0;
  dramAddr = 0;
  pifAddr = 0;
  toDram = false;
  pif.Reset();
}

auto SI::Read(u32 addr) const -> u32 {
  switch (addr) {
  case 0x04800000:
    return dramAddr;
  case 0x04800004:
  case 0x04800010:
    return pifAddr;
  case 0x0480000C:
    return 0;
  case 0x04800018:
    u32 val = 0;
    val |= status.dmaBusy;
    val |= (0 << 1);
    val |= (0 << 3);
    val |= (mem.mmio.mi.miIntr.si << 12);
    return val;
  default:
    Util::panic("Unhandled SI[{:08X}] read", addr);
  }
}

// pif -> rdram
template <>
void SI::DMA<true>() {
  pif.ProcessCommands(mem);
  for (int i = 0; i < 64; i++) {
    mem.mmio.rdp.WriteRDRAM<u8>(dramAddr + i, pif.Read(pifAddr + i));
  }
  Util::trace("SI DMA from PIF RAM to RDRAM ({:08X} to {:08X})", pifAddr, dramAddr);
}

// rdram -> pif
template <>
void SI::DMA<false>() {
  for (int i = 0; i < 64; i++) {
    pif.Write(pifAddr + i, mem.mmio.rdp.ReadRDRAM<u8>(dramAddr + i));
  }
  Util::trace("SI DMA from RDRAM to PIF RAM ({:08X} to {:08X})", dramAddr, pifAddr);
}

void SI::DMA() {
  status.dmaBusy = false;
  if (toDram)
    DMA<true>();
  else
    DMA<false>();
  mem.mmio.mi.InterruptRaise(MI::Interrupt::SI);
}

void SI::Write(u32 addr, u32 val) {
  switch (addr) {
  case 0x04800000:
    dramAddr = val & RDRAM_DSIZE;
    break;
  case 0x04800004:
    pifAddr = val & 0x1FFFFFFF;
    status.dmaBusy = true;
    toDram = true;
    scheduler.EnqueueRelative(SI_DMA_DELAY, SI_DMA);
    break;
  case 0x04800010:
    pifAddr = val & 0x1FFFFFFF;
    status.dmaBusy = true;
    toDram = false;
    scheduler.EnqueueRelative(SI_DMA_DELAY, SI_DMA);
    break;
  case 0x04800018:
    mem.mmio.mi.InterruptLower(MI::Interrupt::SI);
    break;
  default:
    Util::panic("Unhandled SI[{:08X}] write ({:08X})", addr, val);
  }
}
} // namespace n64
