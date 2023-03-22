#include <Core.hpp>
#include <ParallelRDPWrapper.hpp>
#include <Window.hpp>
#include <Scheduler.hpp>

namespace n64 {
Core::Core() {
  if(SDL_GameControllerAddMappingsFromFile("resources/gamecontrollerdb.txt") < 0) {
    Util::warn("Failed to load game controller DB");
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
  cpu->mem.mmio.si.pif.InitDevices(cpu->mem.saveType);
  cpu->mem.mmio.si.pif.LoadMempak(rom_);
  cpu->mem.mmio.si.pif.LoadEeprom(cpu->mem.saveType, rom_);
  isPAL = cpu->mem.IsROMPAL();
  cpu->mem.mmio.si.pif.ExecutePIF(cpu->mem, cpu->regs);
}

void Core::Run(Window& window, float volumeL, float volumeR) {
  MMIO& mmio = cpu->mem.mmio;

  for (int field = 0; field < mmio.vi.numFields; field++) {
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

    mmio.ai.Step(cpu->mem, cpu->regs, frameCycles, volumeL, volumeR);
    scheduler.tick(frameCycles, cpu->mem, cpu->regs);
  }
}
}
