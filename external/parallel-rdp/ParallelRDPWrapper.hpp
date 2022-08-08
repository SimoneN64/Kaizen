#pragma once
#include <Core.hpp>
#include <wsi.hpp>
#include <SDL2/SDL.h>
#include <core/mmio/VI.hpp>

struct Window;
static SDL_Window* g_Window;

class ParallelRdpWindowInfo {
public:
  struct CoordinatePair {
    int x;
    int y;
  };
  virtual CoordinatePair get_window_size() = 0;
  virtual ~ParallelRdpWindowInfo() = default;
};

class SDLParallelRdpWindowInfo : public ParallelRdpWindowInfo {
  CoordinatePair get_window_size() {
    int width, height;
    SDL_GetWindowSize(g_Window, &width, &height);
    return CoordinatePair{ width, height };
  }
};

static u32 windowID;
VkQueue GetGraphicsQueue();
VkInstance GetVkInstance();
VkPhysicalDevice GetVkPhysicalDevice();
VkDevice GetVkDevice();
uint32_t GetVkGraphicsQueueFamily();
VkFormat GetVkFormat();
VkCommandBuffer GetVkCommandBuffer();
void SubmitRequestedVkCommandBuffer();
void InitParallelRDP(const u8* rdram, SDL_Window*);
void UpdateScreenParallelRdp(Window& imguiWindow, const n64::VI& vi);
void ParallelRdpEnqueueCommand(int command_length, u32* buffer);
void ParallelRdpOnFullSync();
void UpdateScreenParallelRdpNoGame(Window& imguiWindow);
bool IsFramerateUnlocked();
void SetFramerateUnlocked(bool unlocked);
