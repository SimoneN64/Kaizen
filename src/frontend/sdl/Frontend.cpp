#include <Frontend.hpp>
#include <gb/Core.hpp>
#include <n64/Core.hpp>
#include <volk.h>
#include <ParallelRDPWrapper.hpp>

namespace natsukashii::frontend {
using namespace natsukashii;
App::~App() {
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
}

App::App(const std::string& rom, const std::string& selectedCore) {
  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_VIDEO_VULKAN);

  if(selectedCore == "gb") {
    window = SDL_CreateWindow("natukashii", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_RESIZABLE);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_RenderSetLogicalSize(renderer, 160, 144);
    id = SDL_GetWindowID(window);
    core = std::make_unique<gb::core::Core>(rom);
  } else if(selectedCore == "n64") {
    window = SDL_CreateWindow("natukashii", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN);
    id = SDL_GetWindowID(window);
    if(volkInitialize() != VK_SUCCESS) {
      util::panic("Failed to initialize Volk\n");
    }
    core = std::make_unique<n64::core::Core>(rom);
  } else {
    util::panic("Unimplemented core!");
  }
}

void App::Run() {
  while(!core->ShouldQuit()) {
    core->Run();
    core->PollInputs(id);
  }
}
}
