#pragma once
#include <parallel-rdp/ParallelRDPWrapper.hpp>
#include <imgui.h>
#include <imgui_impl_sdl.h>
#include <imgui_impl_vulkan.h>
#include <SDL.h>
#include <Core.hpp>
#include <vector>
#include <nlohmann/json.hpp>

using namespace nlohmann;

struct Window {
  explicit Window(n64::Core& core);
  ~Window();
  ImDrawData* Present(n64::Core& core);

  [[nodiscard]] bool gotClosed(SDL_Event event);
  ImFont *uiFont{}, *codeFont{};
  u32 windowID{};
  float volumeL = 0.0, volumeR = 0.0;
  void LoadROM(n64::Core& core, const std::string& path);
private:
  json settings;
  std::fstream settingsFile;
  bool lockVolume = true;
  SDL_Window* window{};
  std::string windowTitle{"Gadolinium"};
  std::string shadowWindowTitle{windowTitle};
  std::string gameName{};
  void InitSDL();
  void InitImgui();
  void Render(n64::Core& core);
  void LoadSettings(n64::Core&);

  VkPhysicalDevice physicalDevice{};
  VkDevice device{};
  uint32_t queueFamily{uint32_t(-1)};
  VkQueue queue{};
  VkPipelineCache pipelineCache{};
  VkDescriptorPool descriptorPool{};
  VkAllocationCallbacks* allocator{};

  u32 minImageCount = 2;
};
