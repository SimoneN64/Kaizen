#pragma once
#include <ParallelRDPWrapper.hpp>
#include <imgui.h>
#include <imgui_impl_sdl.h>
#include <imgui_impl_vulkan.h>
#include <SDL2/SDL.h>
#include <backend/Core.hpp>
#include <vector>
#include <frontend/imgui/Settings.hpp>
#include <frontend/imgui/GameList.hpp>

struct DrawData {
  ImDrawData* first;
  float second;
};

struct Window {
  explicit Window(n64::Core& core);
  ~Window();
  DrawData Present(n64::Core& core);

  [[nodiscard]] bool gotClosed(SDL_Event event);
  ImFont *uiFont{}, *codeFont{};
  u32 windowID{};
  Settings settings;
  GameList gameList;
  void LoadROM(n64::Core& core, const std::string& path);
private:
  bool renderGameList = true;
  SDL_Window* window{};
  std::string windowTitle{"Gadolinium"};
  std::string shadowWindowTitle{windowTitle};
  std::string gameName{};
  void InitSDL();
  void InitImgui();
  float Render(n64::Core& core);

  VkPhysicalDevice physicalDevice{};
  VkDevice device{};
  uint32_t queueFamily{uint32_t(-1)};
  VkQueue queue{};
  VkPipelineCache pipelineCache{};
  VkDescriptorPool descriptorPool{};
  VkAllocationCallbacks* allocator{};

  u32 minImageCount = 2;
};
