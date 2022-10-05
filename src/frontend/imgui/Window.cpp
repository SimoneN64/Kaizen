#include <Window.hpp>
#include <util.hpp>
#include <nfd.hpp>
#include <Core.hpp>
#include <utility>
#include <Audio.hpp>
#include <nlohmann/json.hpp>
#include <filesystem>
#include <SDL.h>

VkInstance instance{};
using json = nlohmann::json;

Window::Window(n64::Core& core) {
  InitSDL();
  InitParallelRDP(core.mem.GetRDRAM(), window);
  InitImgui();
  NFD::Init();
}

[[nodiscard]] bool Window::gotClosed(SDL_Event event) {
  return event.window.event == SDL_WINDOWEVENT_CLOSE
      && event.window.windowID == SDL_GetWindowID(window);
}

void Window::InitSDL() {
  SDL_Init(SDL_INIT_EVERYTHING);
  n64::InitAudio();

  window = SDL_CreateWindow(
    "natsukashii",
    SDL_WINDOWPOS_CENTERED,
    SDL_WINDOWPOS_CENTERED,
    800, 600,
    SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI
  );

  windowID = SDL_GetWindowID(window);

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

  uiFont = io.Fonts->AddFontFromFileTTF("resources/OpenSans.ttf", 15.f);
  codeFont = io.Fonts->AddFontFromFileTTF("resources/Sweet16.ttf", 15.f);

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
    n64::CartInfo cartInfo = core.LoadROM(path);
    std::ifstream gameDbFile("resources/game_db.json");
    json gameDb = json::parse(gameDbFile);
    auto entry = gameDb[fmt::format("{:08x}", cartInfo.crc)]["name"];
    std::string name{};
    if(!entry.empty()) {
      name = entry.get<std::string>();
    } else {
      name = std::filesystem::path(path).stem().string();
    }

    windowTitle = "natsukashii - " + name;

    SDL_SetWindowTitle(window, windowTitle.c_str());
  }
}

void Window::Render(n64::Core& core) {
  ImGui::PushFont(uiFont);
  static bool showSettings = false;
  static std::string windowTitleCache = windowTitle;
  bool showMainMenuBar = windowID == SDL_GetWindowID(SDL_GetMouseFocus());
  if(showMainMenuBar) {
    ImGui::BeginMainMenuBar();
    if (ImGui::BeginMenu("File")) {
      if (ImGui::MenuItem("Open", "O")) {
        nfdchar_t *outpath;
        const nfdu8filteritem_t filter{"Nintendo 64 roms", "n64,z64,v64,N64,Z64,V64"};
        nfdresult_t result = NFD_OpenDialog(&outpath, &filter, 1, nullptr);
        if (result == NFD_OKAY) {
          LoadROM(core, outpath);
          NFD_FreePath(outpath);
        }
      }
      if (ImGui::MenuItem("Exit")) {
        core.done = true;
      }
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Emulation")) {
      if (ImGui::MenuItem("Reset")) {
        LoadROM(core, core.rom);
      }
      if (ImGui::MenuItem("Stop")) {
        windowTitle = "natsukashii";
        SDL_SetWindowTitle(window, windowTitle.c_str());
        core.Stop();
      }
      if (ImGui::MenuItem(core.pause ? "Resume" : "Pause", nullptr, false, core.romLoaded)) {
        core.TogglePause();
        std::string paused = "  | Paused";
        if(core.pause) {
          windowTitleCache = windowTitle;
          windowTitle += paused;
        } else {
          windowTitle = windowTitleCache;
        }
        SDL_SetWindowTitle(window, windowTitle.c_str());
      }
      if (ImGui::MenuItem("Settings")) {
        showSettings = true;
      }
      ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();
  }

  if(showSettings) {
    ImGui::OpenPopup("Settings");
    if(ImGui::BeginPopupModal("Settings", &showSettings)) {
      ImGui::Checkbox("Lock channels", &lockVolume);
      ImGui::SliderFloat("Volume L", &volumeL, 0, 1, "%.2f", ImGuiSliderFlags_NoInput);
      if (!lockVolume) {
        ImGui::SliderFloat("Volume R", &volumeR, 0, 1, "%.2f", ImGuiSliderFlags_NoInput);
      } else {
        ImGui::BeginDisabled();
        ImGui::SliderFloat("Volume R", &volumeR, 0, 1, "%.2f", ImGuiSliderFlags_NoInput);
        ImGui::EndDisabled();
        volumeR = volumeL;
      }
      ImGui::EndPopup();
    }
  }
  ImGui::PopFont();
}
