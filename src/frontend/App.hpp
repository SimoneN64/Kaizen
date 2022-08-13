#pragma once
#include <imgui/Window.hpp>

struct App {
  App() : window(core) {};
  void Run();
private:
  n64::Core core;
  Window window;
};
