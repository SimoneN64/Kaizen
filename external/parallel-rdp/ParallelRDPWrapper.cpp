#include <File.hpp>
#include <ParallelRDPWrapper.hpp>
#include <memory>
#include <rdp_device.hpp>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_vulkan.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

using namespace Vulkan;
using namespace RDP;

bool ParallelRDP::IsFramerateUnlocked() const { return wsi->get_present_mode() != PresentMode::SyncToVBlank; }

void ParallelRDP::SetFramerateUnlocked(bool unlocked) const {
  if (unlocked) {
    wsi->set_present_mode(PresentMode::UnlockedForceTearing);
  } else {
    wsi->set_present_mode(PresentMode::SyncToVBlank);
  }
}

Program *fullscreen_quad_program;

static void check_vk_result(VkResult err) {
  if (err == 0)
    return;
  fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
  if (err < 0)
    abort();
}

void ParallelRDP::LoadWSIPlatform(const std::shared_ptr<InstanceFactory> &instanceFactory,
                                  const std::shared_ptr<WSIPlatform> &wsi_platform,
                                  const std::shared_ptr<WindowInfo> &newWindowInfo, void *winPtr) {
  wsi = std::make_shared<WSI>();
  wsi->set_backbuffer_srgb(false);
  wsi->set_platform(wsi_platform.get());
  wsi->set_present_mode(PresentMode::SyncToVBlank);
  Context::SystemHandles handles;
  if (!wsi->init_simple(instanceFactory.get(), 1, handles)) {
    Util::panic("Failed to initialize WSI!");
  }

  windowInfo = newWindowInfo;

  auto props = SDL_CreateProperties();
#ifdef SDL_PLATFORM_LINUX
  SDL_SetPointerProperty(props, SDL_PROP_WINDOW_CREATE_WAYLAND_WL_SURFACE_POINTER, winPtr);
  SDL_SetPointerProperty(props, SDL_PROP_WINDOW_X11_DISPLAY_POINTER, winPtr);
#elif SDL_PLATFORM_WINDOWS
  SDL_SetPointerProperty(props, SDL_PROP_WINDOW_CREATE_WIN32_HWND_POINTER, winPtr);
#else
  SDL_SetPointerProperty(props, SDL_PROP_WINDOW_CREATE_COCOA_WINDOW_POINTER, winPtr);
#endif
  SDLWindow = SDL_CreateWindowWithProperties(props);

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

  ImGui::StyleColorsDark();
  ImGui_ImplSDL3_InitForVulkan(SDLWindow);

  ImGui_ImplVulkan_InitInfo init_info = {};
  init_info.Instance = wsi->get_context().get_instance();
  init_info.PhysicalDevice = wsi->get_device().get_physical_device();
  init_info.Device = wsi->get_device().get_device();
  init_info.QueueFamily = wsi->get_context().get_queue_info().family_indices[QUEUE_INDEX_GRAPHICS];
  init_info.Queue = wsi->get_context().get_queue_info().queues[QUEUE_INDEX_GRAPHICS];
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
      .request_render_pass(wsi->get_device().get_swapchain_render_pass(SwapchainRenderPass::ColorOnly), false)
      .get_render_pass();
  init_info.Subpass = 0;
  init_info.MinImageCount = 2;
  init_info.ImageCount = 2;
  init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
  init_info.Allocator = nullptr;
  init_info.CheckVkResultFn = check_vk_result;
  ImGui_ImplVulkan_Init(&init_info);
}

void ParallelRDP::Init(const std::shared_ptr<Vulkan::InstanceFactory> &factory,
                       const std::shared_ptr<Vulkan::WSIPlatform> &wsiPlatform,
                       const std::shared_ptr<WindowInfo> &newWindowInfo, const u8 *rdram, void *winPtr) {
  LoadWSIPlatform(factory, wsiPlatform, newWindowInfo, winPtr);

  ResourceLayout vertLayout;
  ResourceLayout fragLayout;

  vertLayout.input_mask = 1;
  vertLayout.output_mask = 1;

  fragLayout.input_mask = 1;
  fragLayout.output_mask = 1;
  fragLayout.spec_constant_mask = 1;
  fragLayout.push_constant_size = 4 * sizeof(float);

  fragLayout.sets[0].sampled_image_mask = 1;
  fragLayout.sets[0].fp_mask = 1;
  fragLayout.sets[0].array_size[0] = 1;

  auto fullscreenQuadVert = Util::ReadFileBinary("resources/vert.spv");
  auto fullscreenQuadFrag = Util::ReadFileBinary("resources/frag.spv");
  auto sizeVert = fullscreenQuadVert.size();
  auto sizeFrag = fullscreenQuadFrag.size();

  fullscreen_quad_program = wsi->get_device().request_program(
    reinterpret_cast<u32 *>(fullscreenQuadVert.data()), sizeVert, reinterpret_cast<u32 *>(fullscreenQuadFrag.data()),
    sizeFrag, &vertLayout, &fragLayout);

  auto aligned_rdram = reinterpret_cast<uintptr_t>(rdram);
  uintptr_t offset = 0;

  if (wsi->get_device().get_device_features().supports_external_memory_host) {
    size_t align = wsi->get_device().get_device_features().host_memory_properties.minImportedHostPointerAlignment;
    offset = aligned_rdram & (align - 1);
    aligned_rdram -= offset;
  }

  CommandProcessorFlags flags = 0;

  command_processor = std::make_shared<CommandProcessor>(wsi->get_device(), reinterpret_cast<void *>(aligned_rdram),
                                                         offset, 8 * 1024 * 1024, 4 * 1024 * 1024, flags);

  if (!command_processor->device_is_supported()) {
    Util::panic("This device probably does not support 8/16-bit storage. Make sure you're using up-to-date drivers!");
  }
}

void ParallelRDP::DrawFullscreenTexturedQuad(Util::IntrusivePtr<Image> image,
                                             Util::IntrusivePtr<CommandBuffer> cmd) const {
  cmd->set_texture(0, 0, image->get_view(), Vulkan::StockSampler::LinearClamp);
  cmd->set_program(fullscreen_quad_program);
  cmd->set_quad_state();
  auto data = static_cast<float *>(cmd->allocate_vertex_data(0, 6 * sizeof(float), 2 * sizeof(float)));
  data[0] = -1.0f;
  data[1] = -3.0f;
  data[2] = -1.0f;
  data[3] = +1.0f;
  data[4] = +3.0f;
  data[5] = +1.0f;

  auto windowSize = windowInfo->get_window_size();

  float zoom = std::min(windowSize.x / wsi->get_platform().get_surface_width(),
                        windowSize.y / wsi->get_platform().get_surface_height());

  float width = (wsi->get_platform().get_surface_width() / windowSize.x) * zoom;
  float height = (wsi->get_platform().get_surface_height() / windowSize.y) * zoom;

  float uniform_data[] = {// Size
                          width, height,
                          // Offset
                          (1.0f - width) * 0.5f, (1.0f - height) * 0.5f};

  cmd->push_constants(uniform_data, 0, sizeof(uniform_data));

  cmd->set_vertex_attrib(0, 0, VK_FORMAT_R32G32_SFLOAT, 0);
  cmd->set_depth_test(false, false);
  cmd->set_depth_compare(VK_COMPARE_OP_ALWAYS);
  cmd->set_primitive_topology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
  cmd->draw(3, 1);
}

void ParallelRDP::UpdateScreen(Util::IntrusivePtr<Image> image) const {
  wsi->begin_frame();

  if (!image) {
    auto info = Vulkan::ImageCreateInfo::immutable_2d_image(800, 600, VK_FORMAT_R8G8B8A8_UNORM);
    info.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    info.misc = IMAGE_MISC_MUTABLE_SRGB_BIT;
    info.initial_layout = VK_IMAGE_LAYOUT_UNDEFINED;
    image = wsi->get_device().create_image(info);

    auto cmd = wsi->get_device().request_command_buffer();

    cmd->image_barrier(*image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                       VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, VK_PIPELINE_STAGE_TRANSFER_BIT,
                       VK_ACCESS_TRANSFER_WRITE_BIT);
    cmd->clear_image(*image, {});
    cmd->image_barrier(*image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                       VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT,
                       VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_SHADER_READ_BIT);
    wsi->get_device().submit(cmd);
  }

  Util::IntrusivePtr<CommandBuffer> cmd = wsi->get_device().request_command_buffer();

  cmd->begin_render_pass(wsi->get_device().get_swapchain_render_pass(SwapchainRenderPass::ColorOnly));
  DrawFullscreenTexturedQuad(image, cmd);
  ImGui::Render();
  ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd->get_command_buffer());

  cmd->end_render_pass();
  wsi->get_device().submit(cmd);
  wsi->end_frame();
}

void ParallelRDP::UpdateScreen(const n64::VI &vi) const {
  command_processor->set_vi_register(VIRegister::Control, vi.status.raw);
  command_processor->set_vi_register(VIRegister::Origin, vi.origin);
  command_processor->set_vi_register(VIRegister::Width, vi.width);
  command_processor->set_vi_register(VIRegister::Intr, vi.intr);
  command_processor->set_vi_register(VIRegister::VCurrentLine, vi.current);
  command_processor->set_vi_register(VIRegister::Timing, vi.burst.raw);
  command_processor->set_vi_register(VIRegister::VSync, vi.vsync);
  command_processor->set_vi_register(VIRegister::HSync, vi.hsync);
  command_processor->set_vi_register(VIRegister::Leap, vi.hsyncLeap.raw);
  command_processor->set_vi_register(VIRegister::HStart, vi.hstart.raw);
  command_processor->set_vi_register(VIRegister::VStart, vi.vstart.raw);
  command_processor->set_vi_register(VIRegister::VBurst, vi.vburst);
  command_processor->set_vi_register(VIRegister::XScale, vi.xscale.raw);
  command_processor->set_vi_register(VIRegister::YScale, vi.yscale.raw);
  ScanoutOptions opts;
  opts.persist_frame_on_invalid_input = true;
  opts.vi.aa = true;
  opts.vi.scale = true;
  opts.vi.dither_filter = true;
  opts.vi.divot_filter = true;
  opts.vi.gamma_dither = true;
  opts.downscale_steps = true;
  opts.crop_overscan_pixels = true;
  Util::IntrusivePtr<Image> image = command_processor->scanout(opts);
  UpdateScreen(image);
  command_processor->begin_frame_context();
}

void ParallelRDP::EnqueueCommand(int command_length, const u32 *buffer) const {
  command_processor->enqueue_command(command_length, buffer);
}

void ParallelRDP::OnFullSync() const { command_processor->wait_for_timeline(command_processor->signal_timeline()); }
