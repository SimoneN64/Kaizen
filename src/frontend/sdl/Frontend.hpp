#pragma once
#include <SDL2/SDL.h>

namespace natsukashii::frontend {
struct App {
  ~App();
  App();
private:
  SDL_Window *window = nullptr;
  SDL_Renderer *renderer = nullptr;
};
}