#pragma once
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_vulkan.h>
#include <SDL3/SDL.h>
#include <QTimer>
#include <QWidget>

class ImGuiWidget : public QWidget {
  QTimer timer;

  SDL_Window *window{};
  VkInstance instance = VK_NULL_HANDLE;
  VkPhysicalDevice gpu = VK_NULL_HANDLE;
  VkDevice device = VK_NULL_HANDLE;
  uint32_t queueFamily = static_cast<uint32_t>(-1);
  VkQueue queue = VK_NULL_HANDLE;
  VkDebugReportCallbackEXT debugReport = VK_NULL_HANDLE;
  VkPipelineCache pipelineCache = VK_NULL_HANDLE;
  VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
  VkSurfaceKHR surface;

  ImGui_ImplVulkanH_Window mainWindowData;
  uint32_t minImageCount = 2;
  bool swapChainRebuild = false;

  void SelectGpu();
  void InitVulkan(char const *const *, uint32_t);
  void InitWindow();
  void FrameRender(ImDrawData *);
  void FramePresent();

public slots:
  void Repaint();

public:
  [[nodiscard]] QPaintEngine *paintEngine() const override { return nullptr; }
  ImGuiWidget(QWidget* parent);


  void resizeEvent(QResizeEvent *event) override;
  void showEvent(QShowEvent *) override;
};
