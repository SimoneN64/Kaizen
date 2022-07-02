#pragma once
#include <n64/Core.hpp>
#include <parallel-rdp-standalone/vulkan/wsi.hpp>
#include <SDL2/SDL.h>
#include <n64/core/mmio/VI.hpp>
#include <BaseCore.hpp>

enum class Platform : bool {
  SDL, Qt
};

using namespace natsukashii::n64;

static SDL_Window* window;
static u32 windowID;
static std::unique_ptr<natsukashii::core::BaseCore> g_Core;
VkQueue GetGraphicsQueue();
VkInstance GetVkInstance();
VkPhysicalDevice GetVkPhysicalDevice();
VkDevice GetVkDevice();
uint32_t GetVkGraphicsQueueFamily();
VkFormat GetVkFormat();
VkCommandBuffer GetVkCommandBuffer();
void SubmitRequestedVkCommandBuffer();
void LoadParallelRDP(Platform platform, const u8* rdram);
void UpdateScreenParallelRdp(core::VI& vi);
void ParallelRdpEnqueueCommand(int command_length, u32* buffer);
void ParallelRdpOnFullSync();
void UpdateScreenParallelRdpNoGame();
bool IsFramerateUnlocked();
void SetFramerateUnlocked(bool unlocked);