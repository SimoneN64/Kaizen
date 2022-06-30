#include <Core.hpp>
#include <SDL2/SDL_events.h>
#include <ParallelRDPWrapper.hpp>

namespace natsukashii::n64::core {
Core::Core(Platform platform, const std::string& rom) {
  mem.LoadROM(rom);
  LoadParallelRDP(platform, mem.GetRDRAM());
}

void Core::Run() {

}

void Core::PollInputs(u32 windowID) {
  SDL_Event event;
  SDL_PollEvent(&event);
  ShouldQuit() = event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == windowID;
}
}
