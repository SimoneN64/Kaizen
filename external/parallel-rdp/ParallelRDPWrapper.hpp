#pragma once
#include <n64/Core.hpp>
#include <wsi.hpp>
#include <SDL2/SDL.h>
#include <n64/core/mmio/VI.hpp>

struct Window;

static SDL_Window* window;
static u32 windowID;
VkQueue GetGraphicsQueue();
VkInstance GetVkInstance();
VkPhysicalDevice GetVkPhysicalDevice();
VkDevice GetVkDevice();
uint32_t GetVkGraphicsQueueFamily();
VkFormat GetVkFormat();
VkCommandBuffer GetVkCommandBuffer();
void SubmitRequestedVkCommandBuffer();
void LoadParallelRDP(const u8* rdram);
void UpdateScreenParallelRdp(Window& imguiWindow, const n64::VI& vi);
void ParallelRdpEnqueueCommand(int command_length, u32* buffer);
void ParallelRdpOnFullSync();
void UpdateScreenParallelRdpNoGame(Window& imguiWindow);
bool IsFramerateUnlocked();
void SetFramerateUnlocked(bool unlocked);