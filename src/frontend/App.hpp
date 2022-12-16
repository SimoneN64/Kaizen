#pragma once
#include <imgui/Window.hpp>
#include <util.hpp>

struct App {
  App();
  ~App() { util::ClearRPC(); }
  void Run();
  inline void LoadROM(const std::string& path) {
    window.LoadROM(core, path);
  }
private:
  n64::Core core;
  Window window;
};
