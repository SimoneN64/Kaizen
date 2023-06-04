#pragma once
#include <ParallelRDPWrapper.hpp>
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_vulkan.h>
#include <SDL2/SDL.h>
#include <backend/Core.hpp>
#include <vector>
#include <frontend/imgui/Settings.hpp>

struct Window {
  explicit Window(n64::Core& core);
  ~Window();
  ImDrawData* Present(n64::Core& core);

  [[nodiscard]] bool gotClosed(SDL_Event event);
  ImFont *uiFont{};
  Settings settings;
  void LoadROM(n64::Core& core, const std::string& path);
private:
  bool showSettings = false;
  SDL_Window* window{};
  std::string windowTitle{"Kaizen"};
  std::string shadowWindowTitle{windowTitle};
  std::string gameName{};
  void InitSDL();
  void InitImgui();
  void Render(n64::Core& core);
  void RenderMainMenuBar(n64::Core& core);

  VkPhysicalDevice physicalDevice{};
  VkDevice device{};
  uint32_t queueFamily{uint32_t(-1)};
  VkQueue queue{};
  VkPipelineCache pipelineCache{};
  VkDescriptorPool descriptorPool{};
  VkAllocationCallbacks* allocator{};

  u32 minImageCount = 2;
};
