#pragma once
#include <imgui.h>
#include <imgui_impl_sdl.h>
#include <imgui_impl_vulkan.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <vulkan/vulkan.h>
#include <BaseCore.hpp>
#include <vector>

struct Window {
  Window();
  ~Window();
  void Update(std::unique_ptr<BaseCore>&);

  [[nodiscard]] bool gotClosed(SDL_Event event) {
    return event.type == SDL_WINDOWEVENT
        && event.window.event == SDL_WINDOWEVENT_CLOSE
        && event.window.windowID == SDL_GetWindowID(window);
  }
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
