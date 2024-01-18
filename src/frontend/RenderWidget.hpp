#pragma once
#undef signals
#include <ParallelRDPWrapper.hpp>
#include <QWidget>
#include <QWindow>
#include <QVulkanInstance>

class QtParallelRdpWindowInfo : public ParallelRdpWindowInfo {
public:
  QtParallelRdpWindowInfo(QWindow* window) : window(window) {}
  CoordinatePair get_window_size() {
    CoordinatePair{ static_cast<float>(window->width()), static_cast<float>(window->height()) };
  }
private:
  QWindow* window;
};

class QtWSIPlatform final : public Vulkan::WSIPlatform {
public:
  QtWSIPlatform(QWindow* window) : window(window) {}

  std::vector<const char*> get_instance_extensions() override {
    auto vec = std::vector<const char*>();

    for (const auto& ext : window->vulkanInstance()->extensions()) {
      vec.push_back(ext);
    }

    return vec;
  }

  VkSurfaceKHR create_surface(VkInstance, VkPhysicalDevice) override {
    return QVulkanInstance::surfaceForWindow(window);
  }

  uint32_t get_surface_width() override {
    return 640;
  }

  uint32_t get_surface_height() override {
    return 480;
  }

  bool alive(Vulkan::WSI&) override {
    return true;
  }

  void poll_input() override { }

  void event_frame_tick(double frame, double elapsed) override { }

  const VkApplicationInfo* get_application_info() override {
    return &appInfo;
  }

  VkApplicationInfo appInfo{
    .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
    .apiVersion = VK_API_VERSION_1_3
  };
private:
  QWindow* window;
};

class RenderWidget : public QWidget {
public:
  explicit RenderWidget(QWidget* parent);

  QPaintEngine* paintEngine() const override {
    return nullptr;
  }
private:
  std::unique_ptr<ParallelRdpWindowInfo> windowInfo;
  QtWSIPlatform* wsiPlatform;
  QVulkanInstance instance;
};