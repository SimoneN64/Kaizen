#pragma once
#include <n64/Core.hpp>
#include <wsi.hpp>

VkQueue GetGraphicsQueue();
VkInstance GetVkInstance();
VkPhysicalDevice GetVkPhysicalDevice();
VkDevice GetVkDevice();
uint32_t GetVkGraphicsQueueFamily();
VkFormat GetVkFormat();
VkCommandBuffer GetVkCommandBuffer();
void SubmitRequestedVkCommandBuffer();
void LoadParallelRdp();
void UpdateScreenParallelRdp();
void ParallelRdpEnqueueCommand(int command_length, u32* buffer);
void ParallelRdpOnFullSync();
void UpdateScreenParallelRdpNoGame();
bool IsFramerateUnlocked();
void SetFramerateUnlocked(bool unlocked);