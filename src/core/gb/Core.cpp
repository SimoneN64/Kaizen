#include <Core.hpp>
#include <SDL2/SDL_events.h>

namespace natsukashii::gb::core {
Core::Core(const std::string& rom) {
  mem.LoadROM(rom);
}

void Core::Run() {
  while(true) {
    cpu.Step(mem);
  }
}

void Core::PollInputs(u32 windowID) {
  SDL_Event event;
  SDL_PollEvent(&event);
  ShouldQuit() = event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == windowID;
}
}
