#include <core/mmio/SI.hpp>
#include <core/Mem.hpp>
#include <Scheduler.hpp>

namespace n64 {
SI::SI() {
  Reset();
}

void SI::Reset() {
  status.raw = 0;
  dramAddr = 0;
  pifAddr = 0;
  pif.Reset();
}

auto SI::Read(MI& mi, u32 addr) const -> u32 {
  switch(addr) {
    case 0x04800000: return dramAddr;
    case 0x04800004: case 0x04800010: return pifAddr;
    case 0x0480000C: return 0;
    case 0x04800018: {
      u32 val = 0;
      val |= status.dmaBusy;
      val |= (0 << 1);
      val |= (0 << 3);
      val |= (mi.miIntr.si << 12);
      return val;
    }
    default:
      Util::panic("Unhandled SI[{:08X}] read", addr);
  }
}

void SI::DMA(Mem& mem, Registers& regs) const {
  SI& si = mem.mmio.si;
  si.status.dmaBusy = false;
  if (toDram) {
    si.pif.ProcessCommands(mem);
    for(int i = 0; i < 64; i++) {
      mem.mmio.rdp.rdram[BYTE_ADDRESS(si.dramAddr + i)] = si.pif.Read(si.pifAddr + i);
    }
    Util::trace("SI DMA from PIF RAM to RDRAM ({:08X} to {:08X})", si.pifAddr, si.dramAddr);
  } else {
    for(int i = 0; i < 64; i++) {
      si.pif.Write(si.pifAddr + i, mem.mmio.rdp.rdram[BYTE_ADDRESS(si.dramAddr + i)]);
    }
    Util::trace("SI DMA from RDRAM to PIF RAM ({:08X} to {:08X})", si.dramAddr, si.pifAddr);
    si.pif.ProcessCommands(mem);
  }
  InterruptRaise(mem.mmio.mi, regs, Interrupt::SI);
}

void SI::Write(Mem& mem, Registers& regs, u32 addr, u32 val) {
  switch(addr) {
    case 0x04800000:
      dramAddr = val & RDRAM_DSIZE;
      break;
    case 0x04800004: {
      pifAddr = val & 0x1FFFFFFF;
      status.dmaBusy = true;
      toDram = true;
      scheduler.enqueueRelative(SI_DMA_DELAY, SI_DMA);
    } break;
    case 0x04800010: {
      pifAddr = val & 0x1FFFFFFF;
      status.dmaBusy = true;
      toDram = false;
      scheduler.enqueueRelative(SI_DMA_DELAY, SI_DMA);
    } break;
    case 0x04800018:
      InterruptLower(mem.mmio.mi, regs, Interrupt::SI);
      break;
    default:
      Util::panic("Unhandled SI[{:08X}] write ({:08X})", addr, val);
  }
}
}