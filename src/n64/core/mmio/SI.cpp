#include <n64/core/mmio/SI.hpp>
#include <n64/core/Mem.hpp>
#include <util.hpp>

namespace n64 {
SI::SI() {
  Reset();
}

void SI::Reset() {
  status.raw = 0;
  dramAddr = 0;
  controller.raw = 0;
}

auto SI::Read(MI& mi, u32 addr) const -> u32 {
  switch(addr) {
    case 0x04800000: return dramAddr;
    case 0x04800018: {
      u32 val = 0;
      val |= status.dmaBusy;
      val |= (0 << 1);
      val |= (0 << 3);
      val |= (status.intr << 12);
      return val;
    }
    default: return 0;
  }
}

void SI::Write(Mem& mem, Registers& regs, u32 addr, u32 val) {
  switch(addr) {
    case 0x04800000:
      dramAddr = val;
      break;
    case 0x04800004: {
      if(!(status.raw & 3)) {
        ProcessPIFCommands(mem.pifRam, controller, mem);

        pifAddr = (val & 0x7FC) & PIF_RAM_DSIZE;
        for(int i = 0; i < 64; i++) {
          mem.mmio.rdp.dram[BYTE_ADDRESS(dramAddr + i) & RDRAM_DSIZE] = mem.pifRam[pifAddr + i];
        }
        InterruptRaise(mem.mmio.mi, regs, Interrupt::SI);
        status.intr = 1;
        //util::logdebug("SI DMA from PIF RAM to RDP RAM ({:08X} to {:08X})\n", pifAddr, dramAddr);
      }
    } break;
    case 0x04800010: {
      if(!(status.raw & 3)) {
        pifAddr = (val & 0x7FC) & PIF_RAM_DSIZE;
        for(int i = 0; i < 64; i++) {
          mem.pifRam[pifAddr + i] = mem.mmio.rdp.dram[BYTE_ADDRESS(dramAddr + i) & RDRAM_DSIZE];
        }
        ProcessPIFCommands(mem.pifRam, controller, mem);
        InterruptRaise(mem.mmio.mi, regs, Interrupt::SI);
        status.intr = 1;
        //util::logdebug("SI DMA from RDP RAM to PIF RAM ({:08X} to {:08X})\n", dramAddr, pifAddr);
      }
    } break;
    case 0x04800018:
      InterruptLower(mem.mmio.mi, regs, Interrupt::SI);
      status.intr = 0;
      break;
    default: util::panic("Unhandled SI[%08X] write (%08X)\n", addr, val);
  }
}

}