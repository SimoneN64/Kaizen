#include <KaizenQt.hpp>
#include <RenderWidget.hpp>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_vulkan.h>

static void check_vk_result(VkResult err) {
  if (err == 0)
    return;
  fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
  if (err < 0)
    abort();
}

void RenderWidget::InitImgui(std::shared_ptr<Vulkan::WSI>& wsi) {
  this->wsi = wsi;
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

  ImGui::StyleColorsDark();
  ImGuiStyle &style = ImGui::GetStyle();
  style.WindowRounding = 0.0f;
  style.Colors[ImGuiCol_WindowBg].w = 1.0f;

  ImGui_ImplSDL3_InitForVulkan(sdlWindow);

  auto myVkInstance = instance->qVkInstance.vkInstance();

  volkInitialize();
  volkLoadInstance(myVkInstance);

  ImGui_ImplVulkan_LoadFunctions(
    [](const char *function_name, void *instance) {
      return vkGetInstanceProcAddr(static_cast<VkInstance>(instance), function_name);
    },
    myVkInstance);

  ImGui_ImplVulkan_InitInfo init_info = {};
  init_info.Instance = myVkInstance;
  init_info.PhysicalDevice = wsi->get_device().get_physical_device();
  init_info.Device = wsi->get_device().get_device();
  init_info.QueueFamily = wsi->get_context().get_queue_info().family_indices[Vulkan::QUEUE_INDEX_GRAPHICS];
  init_info.Queue = wsi->get_context().get_queue_info().queues[Vulkan::QUEUE_INDEX_GRAPHICS];
  init_info.PipelineCache = nullptr;
  {
    VkDescriptorPoolSize pool_sizes[] = {
      {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1},
    };
    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1;
    pool_info.poolSizeCount = IM_ARRAYSIZE(pool_sizes);
    pool_info.pPoolSizes = pool_sizes;
    auto err = wsi->get_device().get_device_table().vkCreateDescriptorPool(wsi->get_device().get_device(), &pool_info,
                                                                           nullptr, &init_info.DescriptorPool);
    check_vk_result(err);
  }

  init_info.RenderPass =
    wsi->get_device()
      .request_render_pass(wsi->get_device().get_swapchain_render_pass(Vulkan::SwapchainRenderPass::ColorOnly), false)
      .get_render_pass();
  init_info.Subpass = 0;
  init_info.MinImageCount = 2;
  init_info.ImageCount = 2;
  init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
  init_info.Allocator = nullptr;
  init_info.CheckVkResultFn = check_vk_result;
  ImGui_ImplVulkan_Init(&init_info);

  connect(&timer, &QTimer::timeout, this, &RenderWidget::UpdateEvents);
  timer.setInterval(16);
  timer.start();
}

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

  instance = std::make_shared<QtInstanceFactory>();
  windowHandle()->setVulkanInstance(&instance->qVkInstance);
  windowHandle()->create();

  wsiPlatform = std::make_shared<QtWSIPlatform>(windowHandle());
  windowInfo = std::make_shared<QtParallelRdpWindowInfo>(windowHandle());

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
}

void RenderWidget::UpdateEvents() {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    ImGui_ImplSDL3_ProcessEvent(&event);
  }

  Util::IntrusivePtr<Vulkan::CommandBuffer> cmd = wsi->get_device().request_command_buffer();

  ImGui_ImplVulkan_NewFrame();
  ImGui_ImplSDL3_NewFrame();
  ImGui::NewFrame();

  ImGui::ShowDemoWindow();

  ImGui::Render();
  ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd->get_command_buffer());

  ImGui::UpdatePlatformWindows();
  ImGui::RenderPlatformWindowsDefault();
}
