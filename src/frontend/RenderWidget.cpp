#include <KaizenQt.hpp>
#include <RenderWidget.hpp>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_vulkan.h>

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

  instance = std::make_unique<QtInstanceFactory>();
  windowHandle()->setVulkanInstance(&instance->qVkInstance);
  windowHandle()->create();

  wsiPlatform = std::make_unique<QtWSIPlatform>(windowHandle());
  windowInfo = std::make_unique<QtParallelRdpWindowInfo>(windowHandle());

  auto winPtr = reinterpret_cast<void *>(winId());
  auto props = SDL_CreateProperties();
#ifdef SDL_PLATFORM_LINUX
  SDL_SetPointerProperty(props, SDL_PROP_WINDOW_CREATE_WAYLAND_WL_SURFACE_POINTER, winPtr);
  SDL_SetNumberProperty(props, SDL_PROP_WINDOW_X11_WINDOW_NUMBER, static_cast<s64> winPtr);
#elif SDL_PLATFORM_WINDOWS
  SDL_SetPointerProperty(props, SDL_PROP_WINDOW_CREATE_WIN32_HWND_POINTER, winPtr);
#else
  SDL_SetPointerProperty(props, SDL_PROP_WINDOW_CREATE_COCOA_WINDOW_POINTER, winPtr);
#endif
  sdlWindow = SDL_CreateWindowWithProperties(props);

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

  ImGui::StyleColorsDark();

  ImGui_ImplSDL3_InitForVulkan(sdlWindow);

  connect(&timer, &QTimer::timeout, this, &RenderWidget::UpdateEvents);
  timer.setInterval(16);
  timer.start();
}

void RenderWidget::UpdateEvents() {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    ImGui_ImplSDL3_ProcessEvent(&event);
  }
}
