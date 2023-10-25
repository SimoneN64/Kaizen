#include <Core.hpp>
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
  pause = false;
  romLoaded = true;

  auto extension = fs::path(rom).extension().string();
  bool isArchive = false;
  for(const auto i : ARCHIVE_TYPES) {
    if(extension == i) {
      isArchive = true;
      break;
    }
  }

  cpu->mem.LoadROM(isArchive, rom);
  GameDB::match(cpu->mem);
  cpu->mem.mmio.vi.isPal = cpu->mem.IsROMPAL();
  cpu->mem.mmio.si.pif.InitDevices(cpu->mem.saveType);
  cpu->mem.mmio.si.pif.mempakPath = rom;
  cpu->mem.mmio.si.pif.LoadEeprom(cpu->mem.saveType, rom);
  cpu->mem.flash.Load(cpu->mem.saveType, rom);
  cpu->mem.LoadSRAM(cpu->mem.saveType, rom);
  PIF::ExecutePIF(cpu->mem, cpu->regs);
}

void Core::Run(float volumeL, float volumeR) {
  Mem& mem = cpu->mem;
  MMIO& mmio = mem.mmio;
  Registers& regs = cpu->regs;

  for (int field = 0; field < mmio.vi.numFields; field++) {
    int frameCycles = 0;
    for (int i = 0; i < mmio.vi.numHalflines; i++) {
      mmio.vi.current = (i << 1) + field;

      if ((mmio.vi.current & 0x3FE) == mmio.vi.intr) {
        InterruptRaise(mmio.mi, regs, Interrupt::VI);
      }

      for(; cycles < mem.mmio.vi.cyclesPerHalfline; cycles++, frameCycles++) {
        int taken = cpu->Step();
        static int cpuSteps = 0;
        cpuSteps += taken;
        if(mmio.rsp.spStatus.halt) {
          cpuSteps = 0;
          mmio.rsp.steps = 0;
        } else {
          while(cpuSteps > 2) {
            mmio.rsp.steps += 2;
            cpuSteps -= 3;
          }

          while(mmio.rsp.steps > 0) {
            mmio.rsp.steps--;
            mmio.rsp.Step(regs, mem);
          }
        }

        cycles += taken;
        frameCycles += taken;
        scheduler.tick(taken, mem, regs);
      }

      cycles -= mmio.vi.cyclesPerHalfline;
    }

    if ((mmio.vi.current & 0x3FE) == mmio.vi.intr) {
      InterruptRaise(mmio.mi, regs, Interrupt::VI);
    }

    mmio.ai.Step(cpu->mem, regs, frameCycles, volumeL, volumeR);
    scheduler.tick(frameCycles, mem, regs);
  }
}
}
