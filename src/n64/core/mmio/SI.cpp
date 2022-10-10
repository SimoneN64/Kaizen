#include <n64/core/mmio/SI.hpp>
#include <n64/core/Mem.hpp>

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
    case 0x0480000C: return 0;
    case 0x04800018: {
      u32 val = 0;
      val |= status.dmaBusy;
      val |= (0 << 1);
      val |= (0 << 3);
      val |= (status.intr << 12);
      return val;
    }
    default: return 0xFFFFFFFF;
  }
}

void SI::Write(Mem& mem, Registers& regs, u32 addr, u32 val) {
  switch(addr) {
    case 0x04800000:
      dramAddr = val & RDRAM_DSIZE;
      break;
    case 0x04800004: {
      ProcessPIFCommands(mem.pifRam, controller, mem);

      for(int i = 0; i < 64; i++) {
        mem.mmio.rdp.dram[BYTE_ADDRESS(dramAddr + i)] = mem.pifRam[i];
      }
      InterruptRaise(mem.mmio.mi, regs, Interrupt::SI);
      status.intr = 1;
      util::logdebug("SI DMA from PIF RAM to RDRAM ({:08X} to {:08X})\n", val & 0x1FFFFFFF, dramAddr);
    } break;
    case 0x04800010: {
      for(int i = 0; i < 64; i++) {
        mem.pifRam[i] = mem.mmio.rdp.dram[BYTE_ADDRESS(dramAddr + i)];
      }
      ProcessPIFCommands(mem.pifRam, controller, mem);
      InterruptRaise(mem.mmio.mi, regs, Interrupt::SI);
      status.intr = 1;
      util::logdebug("SI DMA from RDRAM to PIF RAM ({:08X} to {:08X})\n", dramAddr, val & 0x1FFFFFFF);
    } break;
    case 0x04800018:
      InterruptLower(mem.mmio.mi, regs, Interrupt::SI);
      status.intr = 0;
      break;
    default: util::panic("Unhandled SI[%08X] write (%08X)\n", addr, val);
  }
}

}