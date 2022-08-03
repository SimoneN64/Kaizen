#include <Window.hpp>
#include <util.hpp>
#include <nfd.hpp>
#include <n64/Core.hpp>

Window::Window() {
  InitSDL();
  u32 extCount;
  SDL_Vulkan_GetInstanceExtensions(window, &extCount, nullptr);
  std::vector<const char*> extensions;
  extensions.resize(extCount);
  SDL_Vulkan_GetInstanceExtensions(window, &extCount, extensions.data());
  InitVulkan(extensions, extCount);
  InitVulkanWindow();
  InitImgui();
}

void Window::InitSDL() {
  SDL_Init(SDL_INIT_EVERYTHING);
  window = SDL_CreateWindow(
    "natsukashii",
    SDL_WINDOWPOS_CENTERED,
    SDL_WINDOWPOS_CENTERED,
    1280, 720,
    SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI
  );
}

static void check_vk_result(VkResult err) {
  if (err) {
    util::panic("[vulkan] Error: VkResult = {}", err);
  }
}

void Window::InitVulkan(const std::vector<const char*>& extensions, u32 extCount) {
  VkResult err{};

  // Create Vulkan Instance
  {
    VkInstanceCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.enabledExtensionCount = extCount;
    create_info.ppEnabledExtensionNames = extensions.data();

    err = vkCreateInstance(&create_info, nullptr, &instance);
    check_vk_result(err);
  }

  // Select GPU
  {
    uint32_t gpu_count;
    err = vkEnumeratePhysicalDevices(instance, &gpu_count, nullptr);
    check_vk_result(err);
    IM_ASSERT(gpu_count > 0);

    auto* gpus = (VkPhysicalDevice*)malloc(sizeof(VkPhysicalDevice) * gpu_count);
    err = vkEnumeratePhysicalDevices(instance, &gpu_count, gpus);
    check_vk_result(err);

    // If a number >1 of GPUs got reported, find discrete GPU if present, or use first one available. This covers
    // most common cases (multi-gpu/integrated+dedicated graphics). Handling more complicated setups (multiple
    // dedicated GPUs) is out of scope of this sample.
    int use_gpu = 0;
    for (int i = 0; i < (int)gpu_count; i++) {
      VkPhysicalDeviceProperties properties;
      vkGetPhysicalDeviceProperties(gpus[i], &properties);
      if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
        use_gpu = i;
        break;
      }
    }

    physicalDevice = gpus[use_gpu];
    free(gpus);
  }

  // Select graphics queue family
  {
    uint32_t count;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count, nullptr);
    auto* queues = (VkQueueFamilyProperties*)malloc(sizeof(VkQueueFamilyProperties) * count);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count, queues);
    for (uint32_t i = 0; i < count; i++) {
      if (queues[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
        queueFamily = i;
        break;
      }
    }
    free(queues);
    IM_ASSERT(queueFamily != (uint32_t)-1);
  }

  // Create Logical Device (with 1 queue)
  {
    int device_extension_count = 1;
    const char* device_extensions[] = { "VK_KHR_swapchain" };
    const float queue_priority[] = { 1.0f };
    VkDeviceQueueCreateInfo queue_info[1] = {};
    queue_info[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_info[0].queueFamilyIndex = queueFamily;
    queue_info[0].queueCount = 1;
    queue_info[0].pQueuePriorities = queue_priority;
    VkDeviceCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    create_info.queueCreateInfoCount = sizeof(queue_info) / sizeof(queue_info[0]);
    create_info.pQueueCreateInfos = queue_info;
    create_info.enabledExtensionCount = device_extension_count;
    create_info.ppEnabledExtensionNames = device_extensions;
    err = vkCreateDevice(physicalDevice, &create_info, nullptr, &device);
    check_vk_result(err);
    vkGetDeviceQueue(device, queueFamily, 0, &queue);
  }

  // Create Descriptor Pool
  {
    VkDescriptorPoolSize pool_sizes[] = {
      { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
      { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
      { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
      { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
      { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
      { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
      { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
      { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
      { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
      { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
      { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
    };
    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
    pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
    pool_info.pPoolSizes = pool_sizes;
    err = vkCreateDescriptorPool(device, &pool_info, nullptr, &descriptorPool);
    check_vk_result(err);
  }
}

void Window::InitVulkanWindow() {
  VkSurfaceKHR surface;
  if (!SDL_Vulkan_CreateSurface(window, instance, &surface)) {
    util::panic("Failed to create Vulkan surface.");
  }

  // Create Framebuffers
  int w, h;
  SDL_GetWindowSize(window, &w, &h);
  windowData.Surface = surface;

  // Check for WSI support
  VkBool32 res;
  vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, queueFamily, windowData.Surface, &res);
  if (res != VK_TRUE) {
    fprintf(stderr, "Error no WSI support on physical device 0\n");
    exit(-1);
  }

  // Select Surface Format
  const VkFormat requestSurfaceImageFormat[] = { VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_R8G8B8_UNORM };
  const VkColorSpaceKHR requestSurfaceColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
  windowData.SurfaceFormat = ImGui_ImplVulkanH_SelectSurfaceFormat(physicalDevice, windowData.Surface, requestSurfaceImageFormat, (size_t)IM_ARRAYSIZE(requestSurfaceImageFormat), requestSurfaceColorSpace);

  // Select Present Mode
  VkPresentModeKHR present_modes[] = { VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_IMMEDIATE_KHR, VK_PRESENT_MODE_FIFO_KHR };

  windowData.PresentMode = ImGui_ImplVulkanH_SelectPresentMode(physicalDevice, windowData.Surface, &present_modes[0], IM_ARRAYSIZE(present_modes));

  IM_ASSERT(minImageCount >= 2);
  ImGui_ImplVulkanH_CreateOrResizeWindow(instance, physicalDevice, device, &windowData, queueFamily, nullptr, w, h, minImageCount);
}

void Window::InitImgui() {
  VkResult err;
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  (void)io;
  //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
  //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();
  //ImGui::StyleColorsLight();

  // Setup Platform/Renderer backends
  ImGui_ImplSDL2_InitForVulkan(window);
  ImGui_ImplVulkan_InitInfo init_info = {};
  init_info.Instance = instance;
  init_info.PhysicalDevice = physicalDevice;
  init_info.Device = device;
  init_info.QueueFamily = queueFamily;
  init_info.Queue = queue;
  init_info.PipelineCache = pipelineCache;
  init_info.DescriptorPool = descriptorPool;
  init_info.Subpass = 0;
  init_info.MinImageCount = minImageCount;
  init_info.ImageCount = windowData.ImageCount;
  init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
  init_info.Allocator = nullptr;
  init_info.CheckVkResultFn = check_vk_result;
  ImGui_ImplVulkan_Init(&init_info, windowData.RenderPass);

  // Load Fonts
  // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
  // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
  // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
  // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
  // - Read 'docs/FONTS.md' for more instructions and details.
  // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
  //io.Fonts->AddFontDefault();
  //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
  //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
  //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
  //io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
  //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
  //IM_ASSERT(font != NULL);

  // Upload Fonts
  {
    // Use any command queue
    VkCommandPool command_pool = windowData.Frames[windowData.FrameIndex].CommandPool;
    VkCommandBuffer command_buffer = windowData.Frames[windowData.FrameIndex].CommandBuffer;

    err = vkResetCommandPool(device, command_pool, 0);
    check_vk_result(err);
    VkCommandBufferBeginInfo begin_info = {};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    err = vkBeginCommandBuffer(command_buffer, &begin_info);
    check_vk_result(err);

    ImGui_ImplVulkan_CreateFontsTexture(command_buffer);

    VkSubmitInfo end_info = {};
    end_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    end_info.commandBufferCount = 1;
    end_info.pCommandBuffers = &command_buffer;
    err = vkEndCommandBuffer(command_buffer);
    check_vk_result(err);
    err = vkQueueSubmit(queue, 1, &end_info, VK_NULL_HANDLE);
    check_vk_result(err);

    err = vkDeviceWaitIdle(device);
    check_vk_result(err);
    ImGui_ImplVulkan_DestroyFontUploadObjects();
  }
}

Window::~Window() {
  VkResult err = vkDeviceWaitIdle(device);
  check_vk_result(err);
  ImGui_ImplVulkan_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();
  ImGui_ImplVulkanH_DestroyWindow(instance, device, &windowData, nullptr);
  vkDestroyDescriptorPool(device, descriptorPool, nullptr);
  vkDestroyDevice(device, nullptr);
  vkDestroyInstance(instance, nullptr);
}

void Window::Render(ImDrawData* drawData) {
  VkResult err;

  VkSemaphore image_acquired_semaphore  = windowData.FrameSemaphores[windowData.SemaphoreIndex].ImageAcquiredSemaphore;
  VkSemaphore render_complete_semaphore = windowData.FrameSemaphores[windowData.SemaphoreIndex].RenderCompleteSemaphore;
  err = vkAcquireNextImageKHR(device, windowData.Swapchain, UINT64_MAX, image_acquired_semaphore, VK_NULL_HANDLE, &windowData.FrameIndex);
  if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR) {
    rebuildSwapchain = true;
    return;
  }
  check_vk_result(err);

  ImGui_ImplVulkanH_Frame* fd = &windowData.Frames[windowData.FrameIndex];
  {
    err = vkWaitForFences(device, 1, &fd->Fence, VK_TRUE, UINT64_MAX);    // wait indefinitely instead of periodically checking
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
    info.renderPass = windowData.RenderPass;
    info.framebuffer = fd->Framebuffer;
    info.renderArea.extent.width = windowData.Width;
    info.renderArea.extent.height = windowData.Height;
    info.clearValueCount = 1;
    info.pClearValues = &windowData.ClearValue;
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

void Window::Present() {
  if (rebuildSwapchain) {
    return;
  }
  VkSemaphore render_complete_semaphore = windowData.FrameSemaphores[windowData.SemaphoreIndex].RenderCompleteSemaphore;
  VkPresentInfoKHR info = {};
  info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  info.waitSemaphoreCount = 1;
  info.pWaitSemaphores = &render_complete_semaphore;
  info.swapchainCount = 1;
  info.pSwapchains = &windowData.Swapchain;
  info.pImageIndices = &windowData.FrameIndex;
  VkResult err = vkQueuePresentKHR(queue, &info);
  if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR) {
    rebuildSwapchain = true;
    return;
  }
  check_vk_result(err);
  windowData.SemaphoreIndex = (windowData.SemaphoreIndex + 1) % windowData.ImageCount;
}

void Window::Update(std::unique_ptr<BaseCore>& core) {
  if (rebuildSwapchain) {
    int width, height;
    SDL_GetWindowSize(window, &width, &height);
    if (width > 0 && height > 0) {
      ImGui_ImplVulkan_SetMinImageCount(minImageCount);
      ImGui_ImplVulkanH_CreateOrResizeWindow(instance, physicalDevice, device, &windowData, queueFamily, nullptr, width, height, minImageCount);
      windowData.FrameIndex = 0;
      rebuildSwapchain = false;
    }
  }

  // Start the Dear ImGui frame
  ImGui_ImplVulkan_NewFrame();
  ImGui_ImplSDL2_NewFrame();
  ImGui::NewFrame();

  ImGui::BeginMainMenuBar();
  if(ImGui::BeginMenu("File")) {
    if(ImGui::BeginMenu("Open")) {
      if(ImGui::MenuItem("Nintendo 64")) {
        nfdchar_t* outpath;
        const nfdnfilteritem_t filter {"Nintendo 64 roms", "n64,z64,v64,N64,Z64,V64"};
        nfdresult_t result = NFD_OpenDialog(&outpath, &filter, 1, nullptr);
        if(result == NFD_OKAY) {
          core = std::make_unique<n64::Core>(outpath);
          core->initialized = true;
          NFD_FreePath(outpath);
        }
      }
      if(ImGui::MenuItem("Game Boy")) {
        if(ImGui::BeginPopup("##unimplemented_Core")) {
          ImGui::TextColored({1.0, 0.0, 0.0, 0.7}, "Unimplemented core 'Game Boy'!");
          ImGui::EndPopup();
        }
      }
      ImGui::EndMenu();
    }
    if(ImGui::BeginMenu("Exit")) {

      ImGui::EndMenu();
    }
    ImGui::EndMenu();
  }
  ImGui::EndMainMenuBar();

  // Rendering
  ImGui::Render();
  ImDrawData* draw_data = ImGui::GetDrawData();
  const bool is_minimized = (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f);
  if (!is_minimized) {
    windowData.ClearValue.color.float32[0] = 0;
    windowData.ClearValue.color.float32[1] = 0;
    windowData.ClearValue.color.float32[2] = 0;
    windowData.ClearValue.color.float32[3] = 0;
    Render(draw_data);
    Present();
  }
}
