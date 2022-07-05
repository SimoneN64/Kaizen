#include <Core.hpp>
#include <SDL2/SDL_events.h>
#include <ParallelRDPWrapper.hpp>

namespace natsukashii::n64::core {
Core::Core(Platform platform, const std::string& rom) {
  mem.LoadROM(rom);
  LoadParallelRDP(platform, mem.GetRDRAM());
}

void Core::Run() {
  MMIO& mmio = mem.mmio;
  for(mmio.vi.current = 0; mmio.vi.current < 262; mmio.vi.current++) {
    for(int i = 0; i < 6000; i++) {
      cpu.Step(mem);
      mmio.rsp.Step(mmio.mi, cpu.regs, mmio.rdp);
      mmio.rsp.Step(mmio.mi, cpu.regs, mmio.rdp);
      mmio.rsp.Step(mmio.mi, cpu.regs, mmio.rdp);
      mmio.rsp.Step(mmio.mi, cpu.regs, mmio.rdp);
    }

    if((mmio.vi.current & 0x3FE) == mmio.vi.intr) {
      InterruptRaise(mmio.mi, cpu.regs, Interrupt::VI);
    }

    UpdateScreenParallelRdp(mmio.vi);
  }
}

void Core::PollInputs(u32 windowID) {
  SDL_Event event;
  SDL_PollEvent(&event);
  ShouldQuit() = event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == windowID;
}
}
