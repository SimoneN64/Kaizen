#pragma once
#include <imgui.h>
#include <imgui_impl_sdl.h>
#define VK_NO_PROTOTYPES
#include <imgui_impl_vulkan.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <vulkan/vulkan.h>
#include <BaseCore.hpp>
#include <vector>

struct Window {
  explicit Window(std::shared_ptr<BaseCore> core);
  ~Window();
  ImDrawData* Present();

  [[nodiscard]] bool gotClosed(SDL_Event event) {
    return event.type == SDL_WINDOWEVENT
        && event.window.event == SDL_WINDOWEVENT_CLOSE
        && event.window.windowID == SDL_GetWindowID(window);
  }
private:
  std::shared_ptr<BaseCore> core;
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
