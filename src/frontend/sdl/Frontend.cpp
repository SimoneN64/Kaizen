#include <Frontend.hpp>
#include <n64/Core.hpp>
#include <volk.h>
#include "../ParallelRDPWrapper.hpp"

namespace natsukashii::frontend {
using namespace natsukashii;
App::~App() {
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
}

App::App(const std::string& rom, const std::string& selectedCore) {
  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_VIDEO_VULKAN);

  if(selectedCore == "n64") {
    window = SDL_CreateWindow("natukashii", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN);
    windowID = SDL_GetWindowID(window);
    if(volkInitialize() != VK_SUCCESS) {
      util::panic("Failed to initialize Volk\n");
    }
    g_Core = std::make_unique<n64::core::Core>(Platform::SDL, rom);
  } else {
    util::panic("Unimplemented core \"{}\"!", selectedCore);
  }
}

void App::Run() {
  while(!g_Core->ShouldQuit()) {
    g_Core->Run();
  }
}
}
