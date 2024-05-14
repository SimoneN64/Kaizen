#pragma once
#include <backend/Core.hpp>
#include <wsi.hpp>
#include <rdp_device.hpp>

class ParallelRDP {
public:
  class WindowInfo {
  public:
    struct CoordinatePair {
      float x;
      float y;
    };
    virtual CoordinatePair get_window_size() = 0;
    virtual ~WindowInfo() = default;
  };

  void Init(Vulkan::InstanceFactory*, std::unique_ptr<Vulkan::WSIPlatform>&&, std::unique_ptr<WindowInfo>&&, const u8*);
  ParallelRDP() = default;
  ~ParallelRDP() {
    delete wsi;
    delete command_processor;
  }

  void UpdateScreen(n64::VI&, bool = false);
  void EnqueueCommand(int, u32*);
  void OnFullSync();
  bool IsFramerateUnlocked();
  void SetFramerateUnlocked(bool);
private:
  void LoadWSIPlatform(Vulkan::InstanceFactory*, std::unique_ptr<Vulkan::WSIPlatform>&&, std::unique_ptr<WindowInfo>&&);
  void DrawFullscreenTexturedQuad(Util::IntrusivePtr<Vulkan::Image>, Util::IntrusivePtr<Vulkan::CommandBuffer>);
  void UpdateScreen(Util::IntrusivePtr<Vulkan::Image>);

  Vulkan::WSI* wsi = nullptr;
  RDP::CommandProcessor* command_processor;
  std::unique_ptr<WindowInfo> windowInfo;
};
