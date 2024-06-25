#include <core/mmio/SI.hpp>
#include <core/Mem.hpp>
#include <Scheduler.hpp>

namespace n64 {
SI::SI(Mem& mem, Registers& regs) : mem(mem), regs(regs), pif(mem, regs) {
  Reset();
}

void SI::Reset() {
  status.raw = 0;
  dramAddr = 0;
  pifAddr = 0;
  toDram = false;
  pif.Reset();
}

auto SI::Read(u32 addr) const -> u32 {
  switch(addr) {
    case 0x04800000: return dramAddr;
    case 0x04800004: case 0x04800010: return pifAddr;
    case 0x0480000C: return 0;
    case 0x04800018: {
      u32 val = 0;
      val |= status.dmaBusy;
      val |= (0 << 1);
      val |= (0 << 3);
      val |= (mem.mmio.mi.miIntr.si << 12);
      return val;
    }
    default:
      Util::panic("Unhandled SI[{:08X}] read", addr);
  }
}

void SI::DMA() {
  status.dmaBusy = false;
  if (toDram) {
    pif.ProcessCommands(mem);
    for(int i = 0; i < 64; i++) {
      u32 addr = dramAddr + i;
      if(addr < RDRAM_SIZE) {
        mem.mmio.rdp.rdram[BYTE_ADDRESS(addr)] = pif.Read(pifAddr + i);
      }
    }
    Util::trace("SI DMA from PIF RAM to RDRAM ({:08X} to {:08X})", pifAddr, dramAddr);
  } else {
    for(int i = 0; i < 64; i++) {
      u32 addr = dramAddr + i;
      if(addr < RDRAM_SIZE) {
        pif.Write(pifAddr + i, mem.mmio.rdp.rdram[BYTE_ADDRESS(addr)]);
      } else {
        pif.Write(pifAddr + i, 0);
      }
    }
    Util::trace("SI DMA from RDRAM to PIF RAM ({:08X} to {:08X})", dramAddr, pifAddr);
    pif.ProcessCommands(mem);
  }
  mem.mmio.mi.InterruptRaise(MI::Interrupt::SI);
}

void SI::Write(u32 addr, u32 val) {
  switch(addr) {
    case 0x04800000:
      dramAddr = val & RDRAM_DSIZE;
      break;
    case 0x04800004: {
      pifAddr = val & 0x1FFFFFFF;
      status.dmaBusy = true;
      toDram = true;
      scheduler.EnqueueRelative(SI_DMA_DELAY, SI_DMA);
    } break;
    case 0x04800010: {
      pifAddr = val & 0x1FFFFFFF;
      status.dmaBusy = true;
      toDram = false;
      scheduler.EnqueueRelative(SI_DMA_DELAY, SI_DMA);
    } break;
    case 0x04800018:
      mem.mmio.mi.InterruptLower(MI::Interrupt::SI);
      break;
    default:
      Util::panic("Unhandled SI[{:08X}] write ({:08X})", addr, val);
  }
}
}
