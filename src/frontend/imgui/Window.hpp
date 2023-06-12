#pragma once
#include <ParallelRDPWrapper.hpp>
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_vulkan.h>
#include <SDL2/SDL.h>
#include <vector>
#include <frontend/imgui/Settings.hpp>
#include <nfd.hpp>
#include "Discord.hpp"

struct Window {
  explicit Window(n64::Core& core);
  ~Window();
  ImDrawData* Present(n64::Core& core);

  void onClose(SDL_Event event);
  ImFont *uiFont{};
  Settings settings;
  void LoadROM(n64::Core& core, const std::string& path);
  bool done = false;
  std::string gameName{};
private:
  bool showSettings = false;
  SDL_Window* window{};
  std::string windowTitle{"Kaizen"};
  std::string shadowWindowTitle{windowTitle};
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

static void FORCE_INLINE OpenROMDialog(Window& window, n64::Core& core) {
  nfdchar_t *outpath;
  const nfdu8filteritem_t filter{"Nintendo 64 roms/archives", "n64,z64,v64,N64,Z64,V64,zip,tar,rar,7z"};
  nfdresult_t result = NFD_OpenDialog(&outpath, &filter, 1, nullptr);
  if (result == NFD_OKAY) {
    window.LoadROM(core, outpath);
    NFD_FreePath(outpath);
  }
}