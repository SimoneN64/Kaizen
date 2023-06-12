#pragma once
#include <imgui/Window.hpp>
#include <Discord.hpp>

struct App {
  App();
  ~App() { Util::ClearRPC(); }
  void Run();
  n64::Core core;
  Window window;
};
