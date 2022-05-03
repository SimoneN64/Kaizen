#include <Frontend.hpp>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_video.h>

namespace natsukashii::frontend {
App::~App() {
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
}

App::App() {
  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
  window = SDL_CreateWindow("natukashii", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_SHOWN);
  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  id = SDL_GetWindowID(window);
}

void App::Run() {
  while(!quit) {
    SDL_Event e;
    SDL_PollEvent(&e);
    quit = e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_CLOSE && e.window.windowID == id;
  }
}
}
