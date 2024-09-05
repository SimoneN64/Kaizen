#pragma once
#include <backend/Core.hpp>
#include <rdp_device.hpp>
#include <wsi.hpp>

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

  void Init(const std::shared_ptr<Vulkan::InstanceFactory> &, const std::shared_ptr<Vulkan::WSIPlatform> &,
            const std::shared_ptr<WindowInfo> &, const u8 *);
  ParallelRDP() = default;

  void UpdateScreen(n64::VI &);
  void EnqueueCommand(int, u32 *);
  void OnFullSync();
  bool IsFramerateUnlocked();
  void SetFramerateUnlocked(bool);

private:
  void LoadWSIPlatform(const std::shared_ptr<Vulkan::InstanceFactory> &, const std::shared_ptr<Vulkan::WSIPlatform> &,
                       const std::shared_ptr<WindowInfo> &);
  void DrawFullscreenTexturedQuad(Util::IntrusivePtr<Vulkan::Image>, Util::IntrusivePtr<Vulkan::CommandBuffer>);
  void UpdateScreen(Util::IntrusivePtr<Vulkan::Image>);

  std::shared_ptr<Vulkan::WSI> wsi;
  std::shared_ptr<RDP::CommandProcessor> command_processor;
  std::shared_ptr<WindowInfo> windowInfo;
};
