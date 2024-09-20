#include <ImGuiWidget.hpp>
#include <log.hpp>
#include <QWindow>
#include <qevent.h>
#include <SDL3/SDL_vulkan.h>

static void check_vk_result(VkResult err) {
  if (err == 0)
    return;
  fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
  if (err < 0)
    abort();
}

#ifdef APP_USE_VULKAN_DEBUG_REPORT
static VKAPI_ATTR VkBool32 VKAPI_CALL debug_report(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType,
                                                   uint64_t object, size_t location, int32_t messageCode,
                                                   const char *pLayerPrefix, const char *pMessage, void *pUserData) {
  (void)flags;
  (void)object;
  (void)location;
  (void)messageCode;
  (void)pUserData;
  (void)pLayerPrefix; // Unused arguments
  fprintf(stderr, "[vulkan] Debug report from ObjectType: %i\nMessage: %s\n\n", objectType, pMessage);
  return VK_FALSE;
}
#endif // APP_USE_VULKAN_DEBUG_REPORT

static bool IsExtensionAvailable(const std::vector<VkExtensionProperties> &properties, const char *extension) {
  for (const VkExtensionProperties &p : properties)
    if (strcmp(p.extensionName, extension) == 0)
      return true;
  return false;
}

void ImGuiWidget::SelectGpu() {
  uint32_t gpu_count;
  VkResult err = vkEnumeratePhysicalDevices(instance, &gpu_count, nullptr);
  check_vk_result(err);
  IM_ASSERT(gpu_count > 0);

  std::vector<VkPhysicalDevice> gpus;
  gpus.resize(gpu_count);
  err = vkEnumeratePhysicalDevices(instance, &gpu_count, gpus.data());
  check_vk_result(err);

  // If a number >1 of GPUs got reported, find discrete GPU if present, or use first one available. This covers
  // most common cases (multi-gpu/integrated+dedicated graphics). Handling more complicated setups (multiple
  // dedicated GPUs) is out of scope of this sample.
  for (VkPhysicalDevice &device : gpus) {
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(device, &properties);
    if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
      gpu = device;
      return;
    }
  }

  // Use first GPU (Integrated) if a Discrete one is not available.
  if (gpu_count > 0) {
    gpu = gpus[0];
    return;
  }

  Util::panic("[ImGui Widget] Could not find a suitable Vulkan GPU!");
}

void ImGuiWidget::InitVulkan(char const *const *instanceExtensions, uint32_t instanceExtensionsCount) {
  VkResult err;
#ifdef IMGUI_IMPL_VULKAN_USE_VOLK
  volkInitialize();
#endif

  // Create Vulkan Instance
  {
    VkInstanceCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

    // Enumerate available extensions
    uint32_t properties_count;
    std::vector<VkExtensionProperties> properties;
    vkEnumerateInstanceExtensionProperties(nullptr, &properties_count, nullptr);
    properties.resize(properties_count);
    err = vkEnumerateInstanceExtensionProperties(nullptr, &properties_count, properties.data());
    check_vk_result(err);

    std::vector<const char *> extensions;
    extensions.resize(instanceExtensionsCount);
    memcpy(extensions.data(), instanceExtensions, instanceExtensionsCount * sizeof(const char *));

    // Enable required extensions
    if (IsExtensionAvailable(properties, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME))
      extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
#ifdef VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME
    if (IsExtensionAvailable(properties, VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME)) {
      extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
      create_info.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    }
#endif

    // Enabling validation layers
#ifdef APP_USE_VULKAN_DEBUG_REPORT
    const char *layers[] = {"VK_LAYER_KHRONOS_validation"};
    create_info.enabledLayerCount = 1;
    create_info.ppEnabledLayerNames = layers;
    instance_extensions.push_back("VK_EXT_debug_report");
#endif

    // Create Vulkan Instance
    create_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    create_info.ppEnabledExtensionNames = extensions.data();
    err = vkCreateInstance(&create_info, nullptr, &instance);
    check_vk_result(err);
#ifdef IMGUI_IMPL_VULKAN_USE_VOLK
    volkLoadInstance(instance);
#endif

    // Setup the debug report callback
#ifdef APP_USE_VULKAN_DEBUG_REPORT
    auto f_vkCreateDebugReportCallbackEXT =
      (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
    IM_ASSERT(f_vkCreateDebugReportCallbackEXT != nullptr);
    VkDebugReportCallbackCreateInfoEXT debug_report_ci = {};
    debug_report_ci.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
    debug_report_ci.flags =
      VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
    debug_report_ci.pfnCallback = debug_report;
    debug_report_ci.pUserData = nullptr;
    err = f_vkCreateDebugReportCallbackEXT(instance, &debug_report_ci, nullptr, &g_DebugReport);
    check_vk_result(err);
#endif
  }

  // Select Physical Device (GPU)
  SelectGpu();

  // Select graphics queue family
  {
    uint32_t count;
    vkGetPhysicalDeviceQueueFamilyProperties(gpu, &count, nullptr);
    VkQueueFamilyProperties *queues = (VkQueueFamilyProperties *)malloc(sizeof(VkQueueFamilyProperties) * count);
    vkGetPhysicalDeviceQueueFamilyProperties(gpu, &count, queues);
    for (uint32_t i = 0; i < count; i++)
      if (queues[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
        queueFamily = i;
        break;
      }
    free(queues);
    IM_ASSERT(queueFamily != (uint32_t)-1);
  }

  // Create Logical Device (with 1 queue)
  {
    std::vector<const char *> device_extensions;
    device_extensions.push_back("VK_KHR_swapchain");

    // Enumerate physical device extension
    uint32_t properties_count;
    std::vector<VkExtensionProperties> properties;
    vkEnumerateDeviceExtensionProperties(gpu, nullptr, &properties_count, nullptr);
    properties.resize(properties_count);
    vkEnumerateDeviceExtensionProperties(gpu, nullptr, &properties_count, properties.data());
#ifdef VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME
    if (IsExtensionAvailable(properties, VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME))
      device_extensions.push_back(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
#endif

    const float queue_priority[] = {1.0f};
    VkDeviceQueueCreateInfo queue_info[1] = {};
    queue_info[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_info[0].queueFamilyIndex = queueFamily;
    queue_info[0].queueCount = 1;
    queue_info[0].pQueuePriorities = queue_priority;
    VkDeviceCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    create_info.queueCreateInfoCount = sizeof(queue_info) / sizeof(queue_info[0]);
    create_info.pQueueCreateInfos = queue_info;
    create_info.enabledExtensionCount = (uint32_t)device_extensions.size();
    create_info.ppEnabledExtensionNames = device_extensions.data();
    err = vkCreateDevice(gpu, &create_info, nullptr, &device);
    check_vk_result(err);
    vkGetDeviceQueue(device, queueFamily, 0, &queue);
  }

  // Create Descriptor Pool
  // The example only requires a single combined image sampler descriptor for the font image and only uses one
  // descriptor set (for that) If you wish to load e.g. additional textures you may need to alter pools sizes.
  {
    VkDescriptorPoolSize pool_sizes[] = {
      {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1},
    };
    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1;
    pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
    pool_info.pPoolSizes = pool_sizes;
    err = vkCreateDescriptorPool(device, &pool_info, nullptr, &descriptorPool);
    check_vk_result(err);
  }
}

void ImGuiWidget::InitWindow() {
  mainWindowData.Surface = surface;

  // Check for WSI support
  VkBool32 res;
  vkGetPhysicalDeviceSurfaceSupportKHR(gpu, queueFamily, mainWindowData.Surface, &res);
  if (res != VK_TRUE) {
    fprintf(stderr, "Error no WSI support on physical device 0\n");
    exit(-1);
  }

  // Select Surface Format
  const VkFormat requestSurfaceImageFormat[] = {VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM,
                                                VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_R8G8B8_UNORM};
  const VkColorSpaceKHR requestSurfaceColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
  mainWindowData.SurfaceFormat =
    ImGui_ImplVulkanH_SelectSurfaceFormat(gpu, mainWindowData.Surface, requestSurfaceImageFormat,
                                          (size_t)IM_ARRAYSIZE(requestSurfaceImageFormat), requestSurfaceColorSpace);

  // Select Present Mode
#ifdef APP_UNLIMITED_FRAME_RATE
  VkPresentModeKHR present_modes[] = {VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_IMMEDIATE_KHR,
                                      VK_PRESENT_MODE_FIFO_KHR};
#else
  VkPresentModeKHR present_modes[] = {VK_PRESENT_MODE_FIFO_KHR};
#endif
  mainWindowData.PresentMode =
    ImGui_ImplVulkanH_SelectPresentMode(gpu, mainWindowData.Surface, &present_modes[0], IM_ARRAYSIZE(present_modes));
  // printf("[vulkan] Selected PresentMode = %d\n", mainWindowData.PresentMode);

  // Create SwapChain, RenderPass, Framebuffer, etc.
  IM_ASSERT(minImageCount >= 2);
  ImGui_ImplVulkanH_CreateOrResizeWindow(instance, gpu, device, &mainWindowData, queueFamily, nullptr, width(),
                                         height(), minImageCount);
}

ImGuiWidget::ImGuiWidget(QWidget *parent) : QWidget(parent) {
  setAttribute(Qt::WA_OpaquePaintEvent);
  setAttribute(Qt::WA_NoSystemBackground);
  setFocusPolicy(Qt::StrongFocus);
  // HOWEVER, this important attribute stops the "paintEvent" slot from being called,
  // thus we'll need to write our own method that paints to the screen every frame.
  setAttribute(Qt::WA_PaintOnScreen);
  windowHandle()->setSurfaceType(QWindow::VulkanSurface);

  SDL_InitSubSystem(SDL_INIT_VIDEO);
#ifdef SDL_HINT_IME_SHOW_UI
  SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif

  auto winPtr = reinterpret_cast<void *>(winId());

  auto props = SDL_CreateProperties();
#ifdef SDL_PLATFORM_LINUX
  SDL_SetPointerProperty(props, SDL_PROP_WINDOW_CREATE_WAYLAND_WL_SURFACE_POINTER, winPtr);
  SDL_SetNumberProperty(props, SDL_PROP_WINDOW_X11_WINDOW_NUMBER, reinterpret_cast<s64>(winPtr));
#elif SDL_PLATFORM_WINDOWS
  SDL_SetPointerProperty(props, SDL_PROP_WINDOW_CREATE_WIN32_HWND_POINTER, winPtr);
#else
  SDL_SetPointerProperty(props, SDL_PROP_WINDOW_CREATE_COCOA_WINDOW_POINTER, winPtr);
#endif
  SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_VULKAN_BOOLEAN, true);
  SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_RESIZABLE_BOOLEAN, true);
  SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_HIGH_PIXEL_DENSITY_BOOLEAN, true);

  window = SDL_CreateWindowWithProperties(props);

  uint32_t extensionCount = 0;
  auto extensions = SDL_Vulkan_GetInstanceExtensions(&extensionCount);
  InitVulkan(extensions, extensionCount);

  if (!SDL_Vulkan_CreateSurface(window, instance, nullptr, &surface)) {
    Util::panic("[ImGui Widget]: Failed to create Vulkan surface.\n");
  }

  InitWindow();

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable Docking
  io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport / Platform Windows
  // io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoTaskBarIcons;
  // io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoMerge;

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();
  // ImGui::StyleColorsLight();

  // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
  ImGuiStyle &style = ImGui::GetStyle();
  if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
    style.WindowRounding = 0.0f;
    style.Colors[ImGuiCol_WindowBg].w = 1.0f;
  }

  // Setup Platform/Renderer backends
  ImGui_ImplSDL3_InitForVulkan(window);
  ImGui_ImplVulkan_InitInfo init_info = {};
  init_info.Instance = instance;
  init_info.PhysicalDevice = gpu;
  init_info.Device = device;
  init_info.QueueFamily = queueFamily;
  init_info.Queue = queue;
  init_info.PipelineCache = pipelineCache;
  init_info.DescriptorPool = descriptorPool;
  init_info.RenderPass = mainWindowData.RenderPass;
  init_info.Subpass = 0;
  init_info.MinImageCount = minImageCount;
  init_info.ImageCount = mainWindowData.ImageCount;
  init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
  init_info.Allocator = nullptr;
  init_info.CheckVkResultFn = check_vk_result;
  ImGui_ImplVulkan_Init(&init_info);
}

void ImGuiWidget::resizeEvent(QResizeEvent *event) {
  if (event->size().width() > 0 && event->size().height() > 0 &&
      (swapChainRebuild || mainWindowData.Width != event->size().width() ||
       mainWindowData.Height != event->size().height())) {
    ImGui_ImplVulkan_SetMinImageCount(minImageCount);
    ImGui_ImplVulkanH_CreateOrResizeWindow(instance, gpu, device, &mainWindowData, queueFamily, nullptr,
                                           event->size().width(), event->size().height(), minImageCount);
    mainWindowData.FrameIndex = 0;
    swapChainRebuild = false;
  }
}

void ImGuiWidget::showEvent(QShowEvent *) {
  connect(&timer, &QTimer::timeout, this, &ImGuiWidget::Repaint);
  timer.setInterval(16);
  timer.start();
}

void ImGuiWidget::Repaint() {
  ImGui_ImplVulkan_NewFrame();
  ImGui_ImplSDL3_NewFrame();
  ImGui::NewFrame();

  ImGui::ShowDemoWindow();
  ImGui::Render();

  FrameRender(ImGui::GetDrawData());

  ImGui::UpdatePlatformWindows();
  ImGui::RenderPlatformWindowsDefault();

  FramePresent();
}

void ImGuiWidget::FrameRender(ImDrawData *drawData) {
  VkResult err;

  VkSemaphore image_acquired_semaphore =
    mainWindowData.FrameSemaphores[mainWindowData.SemaphoreIndex].ImageAcquiredSemaphore;
  VkSemaphore render_complete_semaphore =
    mainWindowData.FrameSemaphores[mainWindowData.SemaphoreIndex].RenderCompleteSemaphore;
  err = vkAcquireNextImageKHR(device, mainWindowData.Swapchain, UINT64_MAX, image_acquired_semaphore, VK_NULL_HANDLE,
                              &mainWindowData.FrameIndex);
  if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR) {
    swapChainRebuild = true;
    return;
  }
  check_vk_result(err);

  ImGui_ImplVulkanH_Frame *fd = &mainWindowData.Frames[mainWindowData.FrameIndex];
  {
    err = vkWaitForFences(device, 1, &fd->Fence, VK_TRUE,
                          UINT64_MAX); // wait indefinitely instead of periodically checking
    check_vk_result(err);

    err = vkResetFences(device, 1, &fd->Fence);
    check_vk_result(err);
  }
  {
    err = vkResetCommandPool(device, fd->CommandPool, 0);
    check_vk_result(err);
    VkCommandBufferBeginInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    err = vkBeginCommandBuffer(fd->CommandBuffer, &info);
    check_vk_result(err);
  }
  {
    VkRenderPassBeginInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    info.renderPass = mainWindowData.RenderPass;
    info.framebuffer = fd->Framebuffer;
    info.renderArea.extent.width = mainWindowData.Width;
    info.renderArea.extent.height = mainWindowData.Height;
    info.clearValueCount = 1;
    info.pClearValues = &mainWindowData.ClearValue;
    vkCmdBeginRenderPass(fd->CommandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);
  }

  // Record dear imgui primitives into command buffer
  ImGui_ImplVulkan_RenderDrawData(drawData, fd->CommandBuffer);

  // Submit command buffer
  vkCmdEndRenderPass(fd->CommandBuffer);
  {
    VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    info.waitSemaphoreCount = 1;
    info.pWaitSemaphores = &image_acquired_semaphore;
    info.pWaitDstStageMask = &wait_stage;
    info.commandBufferCount = 1;
    info.pCommandBuffers = &fd->CommandBuffer;
    info.signalSemaphoreCount = 1;
    info.pSignalSemaphores = &render_complete_semaphore;

    err = vkEndCommandBuffer(fd->CommandBuffer);
    check_vk_result(err);
    err = vkQueueSubmit(queue, 1, &info, fd->Fence);
    check_vk_result(err);
  }
}

void ImGuiWidget::FramePresent() {
  if (swapChainRebuild)
    return;
  VkSemaphore render_complete_semaphore =
    mainWindowData.FrameSemaphores[mainWindowData.SemaphoreIndex].RenderCompleteSemaphore;
  VkPresentInfoKHR info = {};
  info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  info.waitSemaphoreCount = 1;
  info.pWaitSemaphores = &render_complete_semaphore;
  info.swapchainCount = 1;
  info.pSwapchains = &mainWindowData.Swapchain;
  info.pImageIndices = &mainWindowData.FrameIndex;
  VkResult err = vkQueuePresentKHR(queue, &info);
  if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR) {
    swapChainRebuild = true;
    return;
  }
  check_vk_result(err);
  mainWindowData.SemaphoreIndex =
    (mainWindowData.SemaphoreIndex + 1) % mainWindowData.SemaphoreCount; // Now we can use the next set of semaphores
}
