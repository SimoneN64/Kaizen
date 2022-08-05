#include <Window.hpp>
#include <util.hpp>
#include <nfd.hpp>
#include <n64/Core.hpp>
#include <parallel-rdp/parallel-rdp-standalone/volk/volk.h>
#include <parallel-rdp/ParallelRDPWrapper.hpp>
#include <utility>

Window::Window(std::shared_ptr<BaseCore> core) : core(std::move(core)) {
  InitSDL();
  LoadWSIPlatform();
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

  if(volkInitialize() != VK_SUCCESS) {
    util::panic("Failed to load Volk!");
  }
}

static void check_vk_result(VkResult err) {
  if (err) {
    util::panic("[vulkan] Error: VkResult = {}", err);
  }
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

  instance = GetVkInstance();
  physicalDevice = GetVkPhysicalDevice();
  device = GetVkDevice();
  queueFamily = GetVkGraphicsQueueFamily();
  queue = GetGraphicsQueue();
  pipelineCache = nullptr;
  descriptorPool = nullptr;
  allocator = nullptr;
  minImageCount = 2;

  {
    VkDescriptorPoolSize poolSizes[] = {
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

    VkDescriptorPoolCreateInfo poolInfo{};

    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    poolInfo.maxSets = 1000 * IM_ARRAYSIZE(poolSizes);
    poolInfo.poolSizeCount = (uint32_t)IM_ARRAYSIZE(poolSizes);
    poolInfo.pPoolSizes = poolSizes;
    err = vkCreateDescriptorPool(device, &poolInfo, allocator, &descriptorPool);
    check_vk_result(err);
  }

  VkRenderPass renderPass;
  {
    VkAttachmentDescription attachment = {};
    attachment.format = GetVkFormat();
    attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    VkAttachmentReference colorAttachment = {};
    colorAttachment.attachment = 0;
    colorAttachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachment;
    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    VkRenderPassCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    info.attachmentCount = 1;
    info.pAttachments = &attachment;
    info.subpassCount = 1;
    info.pSubpasses = &subpass;
    info.dependencyCount = 1;
    info.pDependencies = &dependency;
    err = vkCreateRenderPass(device, &info, allocator, &renderPass);
    check_vk_result(err);
  }

  // Setup Platform/Renderer backends
  ImGui_ImplSDL2_InitForVulkan(window);
  ImGui_ImplVulkan_InitInfo initInfo = {};
  initInfo.Instance = instance;
  initInfo.PhysicalDevice = physicalDevice;
  initInfo.Device = device;
  initInfo.QueueFamily = queueFamily;
  initInfo.Queue = queue;
  initInfo.PipelineCache = pipelineCache;
  initInfo.DescriptorPool = descriptorPool;
  initInfo.Allocator = allocator;
  initInfo.MinImageCount = minImageCount;
  initInfo.ImageCount = 2;
  initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
  initInfo.CheckVkResultFn = check_vk_result;
  ImGui_ImplVulkan_Init(&initInfo, renderPass);

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
    VkCommandBuffer commandBuffer = GetVkCommandBuffer();
    ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
    SubmitRequestedVkCommandBuffer();
  }
}

Window::~Window() {
  VkResult err = vkDeviceWaitIdle(device);
  check_vk_result(err);
  ImGui_ImplVulkan_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();
  vkDestroyDescriptorPool(device, descriptorPool, nullptr);
  vkDestroyDevice(device, nullptr);
  vkDestroyInstance(instance, nullptr);
}

ImDrawData* Window::Present() {
  ImGui_ImplVulkan_NewFrame();
  ImGui_ImplSDL2_NewFrame(window);
  ImGui::NewFrame();

  Render();

  ImGui::Render();
  return ImGui::GetDrawData();
}

void Window::Render() {
  ImGui::BeginMainMenuBar();
  if(ImGui::BeginMenu("File")) {
    if(ImGui::BeginMenu("Open")) {
      if(ImGui::MenuItem("Nintendo 64")) {
        nfdchar_t* outpath;
        const nfdu8filteritem_t filter {"Nintendo 64 roms", "n64,z64,v64,N64,Z64,V64"};
        nfdresult_t result = NFD_OpenDialog(&outpath, &filter, 1, nullptr);
        if(result == NFD_OKAY) {
          core = std::make_shared<n64::Core>(outpath);
          core->initialized = true;
          core->system = System::Nintendo64;
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
}
