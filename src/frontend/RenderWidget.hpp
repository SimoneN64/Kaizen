#pragma once
#undef signals
#include <ParallelRDPWrapper.hpp>
#include <QVulkanWindow>
#include <QWidget>
#include <QWindow>
#include <SDL3/SDL_gamepad.h>

namespace n64 {
struct Core;
}

struct QtInstanceFactory : Vulkan::InstanceFactory {
  VkInstance create_instance(const VkInstanceCreateInfo *info) override {
    handle.setApiVersion({1, 3, 0});
    QByteArrayList exts;
    for (int i = 0; i < info->enabledExtensionCount; i++) {
      exts.push_back(QByteArray::fromStdString(info->ppEnabledExtensionNames[i]));
    }
    QByteArrayList layers;
    for (int i = 0; i < info->enabledLayerCount; i++) {
      layers.push_back(QByteArray::fromStdString(info->ppEnabledLayerNames[i]));
    }
    handle.setExtensions(exts);
    handle.setLayers(layers);
    handle.setApiVersion({1, 3, 0});
    handle.create();

    return handle.vkInstance();
  }

  QVulkanInstance handle;
};

class QtParallelRdpWindowInfo : public ParallelRDP::WindowInfo {
public:
  explicit QtParallelRdpWindowInfo(QWindow *window) : window(window) {}
  CoordinatePair get_window_size() override {
    return CoordinatePair{static_cast<float>(window->width()), static_cast<float>(window->height())};
  }

private:
  std::shared_ptr<QWindow> window{};
};

class QtWSIPlatform final : public Vulkan::WSIPlatform {
public:
  explicit QtWSIPlatform(const std::shared_ptr<n64::Core> &core, QWindow *window) : window(window), core(core) {}

  std::vector<const char *> get_instance_extensions() override {
    auto vec = std::vector<const char *>();

    for (const auto &ext : window->vulkanInstance()->supportedExtensions()) {
      vec.push_back(ext.name);
    }

    return vec;
  }

  VkSurfaceKHR create_surface(VkInstance, VkPhysicalDevice) override {
    return QVulkanInstance::surfaceForWindow(window.get());
  }

  void destroy_surface(VkInstance, VkSurfaceKHR) override {}

  uint32_t get_surface_width() override { return 640; }

  uint32_t get_surface_height() override { return 480; }

  bool alive(Vulkan::WSI &) override { return true; }

  void poll_input() override;
  void poll_input_async(Granite::InputTrackerHandler *handler) override {}

  void event_frame_tick(double frame, double elapsed) override {}

  const VkApplicationInfo *get_application_info() override { return &appInfo; }

  VkApplicationInfo appInfo{.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO, .apiVersion = VK_API_VERSION_1_3};

  std::shared_ptr<QWindow> window{};

private:
  std::shared_ptr<n64::Core> core;
  SDL_Gamepad *gamepad{};
  bool gamepadConnected = false;
};

class RenderWidget : public QWidget {
public:
  [[nodiscard]] VkInstance instance() const { return qtVkInstanceFactory->handle.vkInstance(); }
  explicit RenderWidget(const std::shared_ptr<n64::Core> &);

  [[nodiscard]] QPaintEngine *paintEngine() const override { return nullptr; }
  std::shared_ptr<ParallelRDP::WindowInfo> windowInfo;
  std::shared_ptr<Vulkan::WSIPlatform> wsiPlatform;
  std::shared_ptr<QtInstanceFactory> qtVkInstanceFactory;
Q_SIGNALS:
  void Show() { show(); }
  void Hide() { hide(); }
};
