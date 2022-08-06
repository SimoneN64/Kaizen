#pragma once
#include <imgui.h>
#include <imgui_impl_sdl.h>
#define VK_NO_PROTOTYPES
#include <imgui_impl_vulkan.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <vulkan/vulkan.h>
#include <Core.hpp>
#include <vector>

struct Window {
  explicit Window(n64::Core& core);
  ~Window();
  ImDrawData* Present();

  [[nodiscard]] bool gotClosed(SDL_Event event) {
    return event.type == SDL_WINDOWEVENT
        && event.window.event == SDL_WINDOWEVENT_CLOSE
        && event.window.windowID == SDL_GetWindowID(window);
  }
private:
  n64::Core core;
  void InitSDL();
  void InitImgui();
  void Render();

  SDL_Window* window{};

  VkInstance instance{};
  VkPhysicalDevice physicalDevice{};
  VkDevice device{};
  uint32_t queueFamily{uint32_t(-1)};
  VkQueue queue{};
  VkPipelineCache pipelineCache{};
  VkDescriptorPool descriptorPool{};
  VkAllocationCallbacks* allocator{};

  u32 minImageCount = 2;
  bool rebuildSwapchain = false;
};
