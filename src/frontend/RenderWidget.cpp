#include <KaizenQt.hpp>
#include <RenderWidget.hpp>

RenderWidget::RenderWidget(QWidget *parent) : QWidget(parent) {
  setAttribute(Qt::WA_NativeWindow);
  setAttribute(Qt::WA_PaintOnScreen);
  if (GetOSCompositorCategory() == CompositorCategory::Wayland) {
    setAttribute(Qt::WA_DontCreateNativeAncestors);
  }

  if (GetOSCompositorCategory() == CompositorCategory::MacOS) {
    windowHandle()->setSurfaceType(QWindow::MetalSurface);
  } else {
    windowHandle()->setSurfaceType(QWindow::VulkanSurface);
  }

  if (!Vulkan::Context::init_loader(nullptr)) {
    Util::panic("Could not initialize Vulkan ICD");
  }

  qtVkInstanceFactory = std::make_shared<QtInstanceFactory>();
  windowHandle()->setVulkanInstance(&qtVkInstanceFactory->handle);
  windowHandle()->create();

  wsiPlatform = std::make_shared<QtWSIPlatform>(windowHandle());
  windowInfo = std::make_shared<QtParallelRdpWindowInfo>(windowHandle());
}
