#include <RenderWidget.hpp>
#include <KaizenQt.hpp>

RenderWidget::RenderWidget(QWidget* parent) : QWidget(parent) {
  setAttribute(Qt::WA_NativeWindow);
  setAttribute(Qt::WA_PaintOnScreen);
  if (GetOSCompositorCategory() == CompositorCategory::Wayland) {
    setAttribute(Qt::WA_DontCreateNativeAncestors);
  }

  create();

  if (GetOSCompositorCategory() == CompositorCategory::MacOS) {
    windowHandle()->setSurfaceType(QWindow::MetalSurface);
  }
  else {
    windowHandle()->setSurfaceType(QWindow::VulkanSurface);
  }

  if(!Vulkan::Context::init_loader(nullptr)) {
    Util::panic("Could not initialize Vulkan ICD");
  }

  windowHandle()->setVulkanInstance(&instance.get_qvkinstance());

  wsiPlatform = new QtWSIPlatform(windowHandle());
  windowInfo = std::make_unique<QtParallelRdpWindowInfo>(windowHandle());

  LoadWSIPlatform(&instance, wsiPlatform, std::move(windowInfo));
}