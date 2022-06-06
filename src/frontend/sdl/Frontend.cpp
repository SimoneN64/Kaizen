#include <Frontend.hpp>
#include <algorithm>
#include <cctype>
#include <gb/Core.hpp>
#include <n64/Core.hpp>

namespace natsukashii::frontend {
App::~App() {
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
}

App::App(const std::string& rom) {
  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
  window = SDL_CreateWindow("natukashii", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_SHOWN);
  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  id = SDL_GetWindowID(window);

  std::string ext{rom.begin() + rom.find_first_of('.') + 1, rom.end()};
  std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c){
    return std::tolower(c);
  });

  if(ext == "gb") core = std::make_unique<gb::core::Core>(rom);
  else if(ext == "n64") core = std::make_unique<n64::core::Core>(rom);
  else util::panic("Unimplemented core!");
}

void App::Run() {
  while(!quit) {
    core->Run();

    SDL_Event event;
    SDL_PollEvent(&event);
    quit = event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == id;
  }
}
}
