#include <Core.hpp>

namespace n64 {
Core::Core() {
  Reset();
}

void Core::Reset() {
  cpu.Reset();
  mem.Reset();
  romLoaded = false;
}

void Core::LoadROM(const std::string& rom) {
  Reset();
  mem.LoadROM(rom);
  romLoaded = true;
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
  }
}

void Core::PollInputs(SDL_Event e) {

}
}
