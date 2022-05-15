#pragma once
#include <SDL2/SDL.h>
#include <gb/Core.hpp>

namespace natsukashii::frontend {
struct App {
  ~App();
  App();
  void Run();
private:
  SDL_Window *window = nullptr;
  SDL_Renderer *renderer = nullptr;
  Uint32 id;
  bool quit = false;
  core::Core gb;
};
}
