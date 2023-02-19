#include <Core.hpp>
#include <ParallelRDPWrapper.hpp>
#include <Window.hpp>
#include <algorithm>
#include <Scheduler.hpp>

namespace n64 {
Core::Core() {
  if(SDL_GameControllerAddMappingsFromFile("resources/gamecontrollerdb.txt") < 0) {
    Util::print("Failed to load game controller DB\n");
  }
}

void Core::Stop() {
  cpu->Reset();
  cpu->mem.Reset();
  pause = true;
  romLoaded = false;
}

CartInfo Core::LoadROM(const std::string& rom_) {
  rom = rom_;
  cpu->Reset();
  cpu->mem.Reset();
  pause = false;
  romLoaded = true;
  
  CartInfo cartInfo = cpu->mem.LoadROM(rom);
  isPAL = cartInfo.isPAL;
  cpu->mem.mmio.si.pif.ExecutePIF(cpu->mem, cpu->regs, cartInfo);

  return cartInfo;
}

void Core::Run(Window& window, float volumeL, float volumeR) {
  MMIO& mmio = cpu->mem.mmio;

  for (int field = 0; field < mmio.vi.numFields; field++) {
    int frameCycles = 0;
    if (!pause && romLoaded) {
      for (int i = 0; i < mmio.vi.numHalflines; i++) {
        mmio.vi.current = (i << 1) + field;

        if ((mmio.vi.current & 0x3FE) == mmio.vi.intr) {
          InterruptRaise(mmio.mi, cpu->regs, Interrupt::VI);
        }

        int cpuCount = cpu->Run();
        frameCycles += cpuCount;
        mmio.rsp.Run(cpuCount, cpu->regs, cpu->mem);
        mmio.ai.Step(cpu->mem, cpu->regs, cpuCount, volumeL, volumeR);
        scheduler.tick(cpuCount, cpu->mem, cpu->regs);
      }

      if ((mmio.vi.current & 0x3FE) == mmio.vi.intr) {
        InterruptRaise(mmio.mi, cpu->regs, Interrupt::VI);
      }

      UpdateScreenParallelRdp(*this, window, GetVI());

      int missedCycles = N64_CYCLES_PER_FRAME(isPAL) - frameCycles;
      mmio.ai.Step(cpu->mem, cpu->regs, missedCycles, volumeL, volumeR);
    } else if (pause && romLoaded) {
      UpdateScreenParallelRdp(*this, window, GetVI());
    } else if (pause && !romLoaded) {
      UpdateScreenParallelRdpNoGame(*this, window);
    }
  }
}
}
