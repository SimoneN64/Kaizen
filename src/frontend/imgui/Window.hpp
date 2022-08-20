#pragma once
#include <parallel-rdp/ParallelRDPWrapper.hpp>
#include <imgui.h>
#include <imgui_impl_sdl.h>
#include <imgui_impl_vulkan.h>
#include <SDL2/SDL_video.h>
#include <Core.hpp>
#include <vector>

struct Window {
  explicit Window(n64::Core& core);
  ~Window();
  ImDrawData* Present(n64::Core& core);

  [[nodiscard]] bool gotClosed(SDL_Event event);
private:
  SDL_Window* window;
  u32 windowID;
  void InitSDL();
  void InitImgui();
  void Render(n64::Core& core);

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
