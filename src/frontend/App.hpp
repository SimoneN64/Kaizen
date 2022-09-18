#pragma once
#include <imgui/Window.hpp>

struct App {
  App() : window(core) {};
  ~App() = default;
  void Run();
  inline void LoadROM(const std::string& path) {
    window.LoadROM(core, path);
  }
private:
  n64::Core core;
  Window window;
};
