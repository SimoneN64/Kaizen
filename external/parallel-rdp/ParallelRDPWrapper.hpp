#pragma once
#include <backend/Core.hpp>
#include <wsi.hpp>

struct Window;
static SDL_Window* g_Window;

class ParallelRdpWindowInfo {
public:
  struct CoordinatePair {
    float x;
    float y;
  };
  virtual CoordinatePair get_window_size() = 0;
  virtual ~ParallelRdpWindowInfo() = default;
};

class SDLParallelRdpWindowInfo : public ParallelRdpWindowInfo {
  CoordinatePair get_window_size() {
    int width, height;
    SDL_GetWindowSize(g_Window, &width, &height);
    return CoordinatePair{ static_cast<float>(width), static_cast<float>(height) };
  }
};

VkRenderPass GetVkRenderPass();
VkQueue GetGraphicsQueue();
VkInstance GetVkInstance();
VkPhysicalDevice GetVkPhysicalDevice();
VkDevice GetVkDevice();
uint32_t GetVkGraphicsQueueFamily();
VkFormat GetVkFormat();
VkCommandBuffer GetVkCommandBuffer();
void SubmitRequestedVkCommandBuffer();
void InitParallelRDP(const u8* rdram, SDL_Window* window);
void UpdateScreenParallelRdp(n64::Core& core, Window& imguiWindow, n64::VI& vi);
void ParallelRdpEnqueueCommand(int command_length, u32* buffer);
void ParallelRdpOnFullSync();
void UpdateScreenParallelRdpNoGame(n64::Core& core, Window& imguiWindow);
bool IsFramerateUnlocked();
void SetFramerateUnlocked(bool unlocked);
