#include <filesystem>
#include <Window.hpp>
#include <Core.hpp>
#include <Audio.hpp>
#include <SDL2/SDL.h>
#include <Discord.hpp>

VkInstance instance{};
namespace fs = std::filesystem;
#define GET_TRANSLATED_STRING(x) settings.languageStrings[(x)].c_str()

Window::Window(n64::Core& core) : settings(core) {
  InitSDL();
  InitParallelRDP(core.cpu->mem.GetRDRAM(), window);
  InitImgui();
  NFD::Init();
}

void Window::handleEvents(SDL_Event event, n64::Core& core) {
  done = event.window.event == SDL_WINDOWEVENT_CLOSE
      && event.window.windowID == SDL_GetWindowID(window);

  bool minimized = SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED;
  core.pause = event.window.event == SDL_WINDOWEVENT_FOCUS_LOST || minimized;
  core.render = !minimized;
}

static void check_vk_result(VkResult err) {
  if (err != VK_SUCCESS) {
    Util::panic("[vulkan] Error: VkResult = {}", static_cast<int>(err));
  }
}

void Window::InitSDL() {
  SDL_Init(SDL_INIT_EVERYTHING);
  n64::InitAudio();

  windowTitle = "Kaizen";
  window = SDL_CreateWindow(
    "Kaizen",
    SDL_WINDOWPOS_CENTERED,
    SDL_WINDOWPOS_CENTERED,
    1024, 768,
    SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI
  );

  if(!window) {
    Util::panic("Could not create SDL window: {}", SDL_GetError());
  }

  check_vk_result(volkInitialize());
}

void Window::InitImgui() {
  VkResult err;

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  (void)io;

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();

  instance = GetVkInstance();
  physicalDevice = GetVkPhysicalDevice();
  device = GetVkDevice();
  queueFamily = GetVkGraphicsQueueFamily();
  queue = GetGraphicsQueue();
  pipelineCache = nullptr;
  descriptorPool = nullptr;
  allocator = nullptr;
  minImageCount = 2;

  ImGui_ImplVulkan_LoadFunctions([](const char* function_name, void*) { return vkGetInstanceProcAddr(instance, function_name); });

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
  ImGui_ImplVulkan_Init(&initInfo, GetVkRenderPass());

  int displayIndex = SDL_GetWindowDisplayIndex(window);
  float ddpi, hdpi, vdpi;
  SDL_GetDisplayDPI(displayIndex, &ddpi, &hdpi, &vdpi);

  ddpi /= 96.f;

  uiFont = io.Fonts->AddFontFromFileTTF("resources/OpenSans.ttf", 16.f * ddpi);

  ImGui::GetStyle().ScaleAllSizes(ddpi);

  {
    VkCommandBuffer commandBuffer = GetVkCommandBuffer();
    ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
    SubmitRequestedVkCommandBuffer();
  }
}

Window::~Window() {
  auto err = vkDeviceWaitIdle(device);
  check_vk_result(err);
  ImGui_ImplVulkan_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();

  SDL_DestroyWindow(window);
  SDL_DestroyWindow(g_Window);
  SDL_Quit();
}

ImDrawData* Window::Present(n64::Core& core) {
  ImGui_ImplVulkan_NewFrame();
  ImGui_ImplSDL2_NewFrame(window);
  ImGui::NewFrame();

  Render(core);

  ImGui::Render();
  return ImGui::GetDrawData();
}

void Window::LoadROM(n64::Core& core, const std::string &path) {
  if(!path.empty()) {
    core.LoadROM(path);
    gameName = core.cpu->mem.rom.gameNameDB;

    if(gameName.empty()) {
      gameName = fs::path(path).stem().string();
    }

    Util::UpdateRPC(Util::Playing, gameName);
    windowTitle = "Kaizen - " + gameName;
    shadowWindowTitle = windowTitle;

    SDL_SetWindowTitle(window, windowTitle.c_str());
  }
}

void Window::RenderMainMenuBar(n64::Core &core) {
  ImGui::BeginMainMenuBar();

  if (ImGui::BeginMenu(GET_TRANSLATED_STRING(Language::MENU_FILE))) {
    if (ImGui::MenuItem(GET_TRANSLATED_STRING(Language::FILE_ITEM_OPEN), "O")) {
      OpenROMDialog(*this, core);
    }
    if (ImGui::MenuItem("Dump RDRAM")) {
      core.cpu->mem.DumpRDRAM();
    }
    if (ImGui::MenuItem("Dump IMEM")) {
      core.cpu->mem.DumpIMEM();
    }
    if (ImGui::MenuItem("Dump DMEM")) {
      core.cpu->mem.DumpDMEM();
    }
    if (ImGui::MenuItem(GET_TRANSLATED_STRING(Language::FILE_ITEM_EXIT))) {
      done = true;
    }
    ImGui::EndMenu();
  }
  if (ImGui::BeginMenu(GET_TRANSLATED_STRING(Language::MENU_EMULATION))) {
    if (ImGui::MenuItem(GET_TRANSLATED_STRING(Language::EMULATION_ITEM_RESET))) {
      LoadROM(core, core.rom);
    }
    if (ImGui::MenuItem(GET_TRANSLATED_STRING(Language::EMULATION_ITEM_STOP))) {
      windowTitle = "Kaizen";
      core.rom.clear();
      Util::UpdateRPC(Util::Idling);
      SDL_SetWindowTitle(window, windowTitle.c_str());
      core.Stop();
    }
    if (ImGui::MenuItem(core.pause ? GET_TRANSLATED_STRING(Language::EMULATION_ITEM_RESUME)
                                        : GET_TRANSLATED_STRING(Language::EMULATION_ITEM_PAUSE), nullptr, false, core.romLoaded)) {
      core.TogglePause();
      if(core.pause) {
        shadowWindowTitle = windowTitle;
        windowTitle += "  | Paused";
        Util::UpdateRPC(Util::Paused, gameName);
      } else {
        windowTitle = shadowWindowTitle;
        Util::UpdateRPC(Util::Playing, gameName);
      }
      SDL_SetWindowTitle(window, windowTitle.c_str());
    }
    if (ImGui::MenuItem(GET_TRANSLATED_STRING(Language::EMULATION_ITEM_SETTINGS))) {
      showSettings = true;
    }
    ImGui::EndMenu();
  }
  ImGui::EndMainMenuBar();
}

void Window::Render(n64::Core& core) {
  ImGui::PushFont(uiFont);

  u32 ticks = SDL_GetTicks();
  static u32 lastFrame = 0;
  if(!core.pause && core.romLoaded && lastFrame < ticks - 1000) {
    lastFrame = ticks;
    windowTitle += fmt::format("  | {:02d} VI/s", core.cpu->mem.mmio.vi.swaps);
    core.cpu->mem.mmio.vi.swaps = 0;
    SDL_SetWindowTitle(window, windowTitle.c_str());
    windowTitle = shadowWindowTitle;
  }

  if(SDL_GetMouseFocus()) {
    RenderMainMenuBar(core);
  }

  settings.RenderWidget(showSettings);

  ImGui::PopFont();
}
