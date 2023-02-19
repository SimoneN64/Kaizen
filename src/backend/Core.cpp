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
  Stop();
}

void Core::Stop() {
  CpuReset();
  mem.Reset();
  pause = true;
  romLoaded = false;
}

CartInfo Core::LoadROM(const std::string& rom_) {
  rom = rom_;
  CpuReset();
  mem.Reset();
  pause = false;
  romLoaded = true;
  
  CartInfo cartInfo = mem.LoadROM(rom);
  isPAL = cartInfo.isPAL;
  mem.mmio.si.pif.ExecutePIF(mem, CpuGetRegs(), cartInfo);

  return cartInfo;
}

void Core::Run(Window& window, float volumeL, float volumeR) {
  MMIO& mmio = mem.mmio;
  Registers& regs = CpuGetRegs();

  for(int field = 0; field < mmio.vi.numFields; field++) {
    int frameCycles = 0;
    if(!pause && romLoaded) {
      for (int i = 0; i < mmio.vi.numHalflines; i++) {
        mmio.vi.current = (i << 1) + field;

        if ((mmio.vi.current & 0x3FE) == mmio.vi.intr) {
          InterruptRaise(mmio.mi, regs, Interrupt::VI);
        }

        for(;cycles <= mmio.vi.cyclesPerHalfline; cycles++, frameCycles++) {
          int cpuCount = CpuStep(*this);
          int oldCpuCount = cpuCount;
          while(cpuCount--) {
            if (!mmio.rsp.spStatus.halt) {
              regs.steps++;
              if (regs.steps > 2) {
                mmio.rsp.steps += 2;
                regs.steps -= 3;
              }

              while (mmio.rsp.steps > 0) {
                mmio.rsp.steps--;
                mmio.rsp.Step(regs, mem);
              }
            }
          }

          mmio.ai.Step(mem, regs, oldCpuCount, volumeL, volumeR);
          scheduler.tick(1, mem, regs);
        }

        cycles -= mmio.vi.cyclesPerHalfline;
      }

      if ((mmio.vi.current & 0x3FE) == mmio.vi.intr) {
        InterruptRaise(mmio.mi, regs, Interrupt::VI);
      }

      UpdateScreenParallelRdp(*this, window, GetVI());

      int missedCycles = N64_CYCLES_PER_FRAME(isPAL) - frameCycles;
      mmio.ai.Step(mem, regs, missedCycles, volumeL, volumeR);
    } else if(pause && romLoaded) {
      UpdateScreenParallelRdp(*this, window, GetVI());
    } else if(pause && !romLoaded) {
      UpdateScreenParallelRdpNoGame(*this, window);
    }
  }
}
}
