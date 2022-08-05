#pragma once
#include <BaseCore.hpp>
#include <imgui/Window.hpp>

struct App {
  App() : window(core) {};
  void Run();
private:
  std::shared_ptr<BaseCore> core;
  Window window;
};
