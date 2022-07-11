#pragma once
#include <BaseCore.hpp>
#include <imgui/Window.hpp>

struct App {
  App() = default;
  void Run();
private:
  std::unique_ptr<BaseCore> core;
  Window window;
};
