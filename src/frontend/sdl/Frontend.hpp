#pragma once
#include <SDL2/SDL.h>
#include <BaseCore.hpp>
#include <string>
#include <memory>
#include <util.hpp>

namespace natsukashii::frontend {
using namespace natsukashii::core;
struct App {
  ~App();
  App(const std::string&);
  void Run();
private:
  SDL_Window *window = nullptr;
  SDL_Renderer *renderer = nullptr;
  Uint32 id;
  bool quit = false;
  std::unique_ptr<BaseCore> core;
};
}
