#pragma once
#undef signals
#include <ParallelRDPWrapper.hpp>
#include <QWidget>
#include <QWindow>
#include <QVulkanInstance>

struct QtInstanceFactory : Vulkan::InstanceFactory {
  VkInstance create_instance(const VkInstanceCreateInfo *info) override {
    instance.setApiVersion({1,3,0});
    QByteArrayList exts;
    for(int i = 0; i < info->enabledExtensionCount; i++) {
      exts.push_back(QByteArray::fromStdString(info->ppEnabledExtensionNames[i]));
    }
    QByteArrayList layers;
    for(int i = 0; i < info->enabledLayerCount; i++) {
      layers.push_back(QByteArray::fromStdString(info->ppEnabledLayerNames[i]));
    }
    instance.setExtensions(exts);
    instance.setLayers(layers);
    instance.setApiVersion({1,3,0});
    instance.create();

    return instance.vkInstance();
  }

  QVulkanInstance& get_qvkinstance() { return instance; }
private:
  QVulkanInstance instance;
};

class QtParallelRdpWindowInfo : public ParallelRdpWindowInfo {
public:
  explicit QtParallelRdpWindowInfo(QWindow* window) : window(window) {}
  CoordinatePair get_window_size() override {
    return CoordinatePair{ static_cast<float>(window->width()), static_cast<float>(window->height()) };
  }
private:
  QWindow* window;
};

class QtWSIPlatform final : public Vulkan::WSIPlatform {
public:
  explicit QtWSIPlatform(QWindow* window) : window(window) {}

  std::vector<const char*> get_instance_extensions() override {
    auto vec = std::vector<const char*>();

    for (const auto& ext : window->vulkanInstance()->supportedExtensions()) {
      vec.push_back(ext.name);
    }

    return vec;
  }

  VkSurfaceKHR create_surface(VkInstance instance, VkPhysicalDevice) override {
    window->show();
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

  [[nodiscard]] QPaintEngine* paintEngine() const override {
    return nullptr;
  }
private:
  std::unique_ptr<ParallelRdpWindowInfo> windowInfo;
  std::unique_ptr<Vulkan::WSIPlatform> wsiPlatform;
  QtInstanceFactory instance;
};