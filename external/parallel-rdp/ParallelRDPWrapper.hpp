#pragma once
#include <backend/Core.hpp>
#include <wsi.hpp>

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
  SDL_Window* window;
public:
  SDLParallelRdpWindowInfo(SDL_Window* window) : window(window) {}
  CoordinatePair get_window_size() {
    int width, height;
    SDL_GetWindowSize(window, &width, &height);
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
void LoadParallelRDP(const u8* rdram);
Vulkan::WSI* LoadWSIPlatform(Vulkan::WSIPlatform* wsi_platform, std::unique_ptr<ParallelRdpWindowInfo>&& newWindowInfo);
void UpdateScreenParallelRdp(n64::Core& core, n64::VI& vi);
void ParallelRdpEnqueueCommand(int command_length, u32* buffer);
void ParallelRdpOnFullSync();
void UpdateScreenParallelRdpNoGame(n64::Core& core);
bool IsFramerateUnlocked();
void SetFramerateUnlocked(bool unlocked);
