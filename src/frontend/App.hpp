#pragma once
#include <imgui/Window.hpp>
#include <Discord.hpp>

struct App {
  App();
  ~App() { Util::ClearRPC(); }
  void Run();
  inline void LoadROM(const std::string& path) {
    window.LoadROM(core, path);
  }
private:
  n64::Core core;
  Window window;
};
