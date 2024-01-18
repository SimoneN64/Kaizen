#include <RenderWidget.hpp>
#include <KaizenQt.hpp>

RenderWidget::RenderWidget(QWidget* parent) : QWidget(parent) {
  setAttribute(Qt::WA_NativeWindow);
  setAttribute(Qt::WA_PaintOnScreen);
  if (GetOSCompositorCategory() == CompositorCategory::Wayland) {
    setAttribute(Qt::WA_DontCreateNativeAncestors);
  }

  if (GetOSCompositorCategory() == CompositorCategory::MacOS) {
    windowHandle()->setSurfaceType(QWindow::MetalSurface);
  }
  else {
    windowHandle()->setSurfaceType(QWindow::VulkanSurface);
  }

  if(!Vulkan::Context::init_loader(nullptr)) {
    Util::panic("Could not initialize Vulkan ICD");
  }

  instance = std::make_unique<QtInstanceFactory>();
  windowHandle()->setVulkanInstance(&instance->qVkInstance);
  windowHandle()->create();

  wsiPlatform = std::make_unique<QtWSIPlatform>(windowHandle());
  windowInfo = std::make_unique<QtParallelRdpWindowInfo>(windowHandle());
}