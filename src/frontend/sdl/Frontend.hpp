#pragma once
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <string>
#include <util.hpp>

namespace natsukashii::frontend {
struct App {
  ~App();
  App(const std::string&, const std::string&);
  void Run();
private:
  SDL_Renderer *renderer = nullptr;
};
}
