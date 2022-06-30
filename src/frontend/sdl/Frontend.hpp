#pragma once
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <BaseCore.hpp>
#include <string>
#include <memory>
#include <util.hpp>

namespace natsukashii::frontend {
using namespace natsukashii::core;

struct App {
  ~App();
  App(const std::string&, const std::string&);
  void Run();
private:
  SDL_Renderer *renderer = nullptr;
};
}
