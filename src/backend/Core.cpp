#include <Core.hpp>
#include <ParallelRDPWrapper.hpp>
#include <Window.hpp>
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

void Core::LoadROM(const std::string& rom_) {
  rom = rom_;
  cpu->Reset();
  cpu->mem.Reset();
  pause = false;
  romLoaded = true;
  
  cpu->mem.LoadROM(rom);
  GameDB::match(cpu->mem);
  isPAL = cpu->mem.IsROMPAL();
  cpu->mem.mmio.si.pif.ExecutePIF(cpu->mem, cpu->regs);
}

void Core::Run(Window& window, float volumeL, float volumeR) {
  MMIO& mmio = cpu->mem.mmio;

  for (int field = 0; field < mmio.vi.numFields; field++) {
    if (!pause && romLoaded) {
      int frameCycles = 0;
      for (int i = 0; i < mmio.vi.numHalflines; i++) {
        mmio.vi.current = (i << 1) + field;

        if ((mmio.vi.current & 0x3FE) == mmio.vi.intr) {
          InterruptRaise(mmio.mi, cpu->regs, Interrupt::VI);
        }

        int cpuCount = cpu->Run();
        cpu->RunRSP(cpuCount);
        frameCycles += cpuCount;
      }

      if ((mmio.vi.current & 0x3FE) == mmio.vi.intr) {
        InterruptRaise(mmio.mi, cpu->regs, Interrupt::VI);
      }

      UpdateScreenParallelRdp(*this, window, GetVI());

      mmio.ai.Step(cpu->mem, cpu->regs, frameCycles, volumeL, volumeR);
      scheduler.tick(frameCycles, cpu->mem, cpu->regs);
    } else if (pause && romLoaded) {
      UpdateScreenParallelRdp(*this, window, GetVI());
    } else if (pause && !romLoaded) {
      UpdateScreenParallelRdpNoGame(*this, window);
    }
  }
}
}
