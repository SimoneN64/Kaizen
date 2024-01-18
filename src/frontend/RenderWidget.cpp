#include <RenderWidget.hpp>
#include <KaizenQt.hpp>

RenderWidget::RenderWidget(QWidget* parent) : QWidget(parent) {
  if (volkInitialize() != VK_SUCCESS) {
    Util::panic("Could not initialize Vulkan loader!");
  }

  create();

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

  instance.create();
  windowHandle()->setVulkanInstance(&instance);

  wsiPlatform = new QtWSIPlatform(windowHandle());
  windowInfo = std::make_unique<QtParallelRdpWindowInfo>(windowHandle());

  LoadWSIPlatform(wsiPlatform, std::move(windowInfo));
}