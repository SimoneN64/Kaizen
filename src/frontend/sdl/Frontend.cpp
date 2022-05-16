#include <Frontend.hpp>

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
    gb.Run();

    SDL_Event event;
    SDL_PollEvent(&event);
    quit = event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == id;
  }
}
}
