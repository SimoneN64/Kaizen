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

static Vulkan::WSI* wsi;

void LoadParallelRDP(const u8* rdram);
Vulkan::WSI* LoadWSIPlatform(Vulkan::InstanceFactory*, std::unique_ptr<Vulkan::WSIPlatform>&& wsi_platform, std::unique_ptr<ParallelRdpWindowInfo>&& newWindowInfo);
void UpdateScreenParallelRdp(n64::VI& vi);
void ParallelRdpEnqueueCommand(int command_length, u32* buffer);
void ParallelRdpOnFullSync();
void UpdateScreenParallelRdpNoGame();
bool IsFramerateUnlocked();
void SetFramerateUnlocked(bool unlocked);
