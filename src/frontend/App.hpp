#pragma once
#include <imgui.h>
#include <imgui_impl_sdl.h>
#include <imgui_impl_vulkan.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <vulkan/vulkan.h>
#include <common.hpp>
#include <vector>
#include <n64/Core.hpp>

struct App {
  App();
  ~App();
  void Run();
private:
  void InitSDL();
  void InitVulkan(const std::vector<const char*>&, u32);
  void InitVulkanWindow();
  void InitImgui();
  void Render(ImDrawData*);
  void Present();
  SDL_Window* window{};

  VkInstance instance{};
  VkPhysicalDevice physicalDevice{};
  VkDevice device{};
  uint32_t queueFamily{uint32_t(-1)};
  VkQueue queue{};
  VkPipelineCache pipelineCache{};
  VkDescriptorPool descriptorPool{};

  ImGui_ImplVulkanH_Window windowData;
  u32 minImageCount = 2;
  bool rebuildSwapchain = false;
};
