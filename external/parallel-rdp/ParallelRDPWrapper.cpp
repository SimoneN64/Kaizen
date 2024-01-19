#include <ParallelRDPWrapper.hpp>
#include <core/RDP.hpp>
#include <memory>
#include <rdp_device.hpp>
#include <log.hpp>
#include <File.hpp>
#include <SDL2/SDL_vulkan.h>
#include <imgui_impl_vulkan.h>

using namespace Vulkan;
using namespace RDP;

static CommandProcessor* command_processor;
static std::unique_ptr<ParallelRdpWindowInfo> windowInfo;

VkQueue GetGraphicsQueue() {
  return wsi->get_context().get_queue_info().queues[QUEUE_INDEX_GRAPHICS];
}

VkInstance GetVkInstance() {
  return wsi->get_context().get_instance();
}

VkPhysicalDevice GetVkPhysicalDevice() {
  return wsi->get_device().get_physical_device();
}

VkDevice GetVkDevice() {
  return wsi->get_device().get_device();
}

uint32_t GetVkGraphicsQueueFamily() {
  return wsi->get_context().get_queue_info().family_indices[QUEUE_INDEX_GRAPHICS];
}

VkFormat GetVkFormat() {
  return wsi->get_device().get_swapchain_view().get_format();
}

CommandBufferHandle requested_command_buffer;

VkRenderPass GetVkRenderPass() {
  return wsi->get_device().request_render_pass(
    wsi->get_device().get_swapchain_render_pass(SwapchainRenderPass::ColorOnly), true
  ).get_render_pass();
}

VkCommandBuffer GetVkCommandBuffer() {
  requested_command_buffer = wsi->get_device().request_command_buffer();
  return requested_command_buffer->get_command_buffer();
}

void SubmitRequestedVkCommandBuffer() {
  wsi->get_device().submit(requested_command_buffer);
}

bool IsFramerateUnlocked() {
  return wsi->get_present_mode() != PresentMode::SyncToVBlank;
}

void SetFramerateUnlocked(bool unlocked) {
  if (unlocked) {
    wsi->set_present_mode(PresentMode::UnlockedForceTearing);
  } else {
    wsi->set_present_mode(PresentMode::SyncToVBlank);
  }
}

Program* fullscreen_quad_program;

WSI* LoadWSIPlatform(Vulkan::InstanceFactory* instanceFactory, std::unique_ptr<Vulkan::WSIPlatform>&& wsi_platform, std::unique_ptr<ParallelRdpWindowInfo>&& newWindowInfo) {
  wsi = new WSI();
  wsi->set_backbuffer_srgb(false);
  wsi->set_platform(wsi_platform.get());
  wsi->set_present_mode(PresentMode::SyncToVBlank);
  Context::SystemHandles handles;
  if (!wsi->init_simple(instanceFactory, 1, handles)) {
    Util::panic("Failed to initialize WSI!");
  }

  windowInfo = std::move(newWindowInfo);
  return wsi;
}

void LoadParallelRDP(const u8* rdram) {
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

  fullscreen_quad_program = wsi->get_device().request_program(reinterpret_cast<u32*>(fullscreenQuadVert.data()), sizeVert,
                                                              reinterpret_cast<u32*>(fullscreenQuadFrag.data()), sizeFrag,
                                                              &vertLayout, &fragLayout);

  auto aligned_rdram = reinterpret_cast<uintptr_t>(rdram);
  uintptr_t offset = 0;

  if (wsi->get_device().get_device_features().supports_external_memory_host)
  {
    size_t align = wsi->get_device().get_device_features().host_memory_properties.minImportedHostPointerAlignment;
    offset = aligned_rdram & (align - 1);
    aligned_rdram -= offset;
  }

  CommandProcessorFlags flags = 0;

  command_processor = new CommandProcessor(wsi->get_device(), reinterpret_cast<void *>(aligned_rdram),
                                           offset, 8 * 1024 * 1024, 4 * 1024 * 1024, flags);

  if (!command_processor->device_is_supported()) {
    Util::panic("This device probably does not support 8/16-bit storage. Make sure you're using up-to-date drivers!");
  }
}

void DrawFullscreenTexturedQuad(Util::IntrusivePtr<Image> image, Util::IntrusivePtr<CommandBuffer> cmd) {
  cmd->set_texture(0, 0, image->get_view(), Vulkan::StockSampler::LinearClamp);
  cmd->set_program(fullscreen_quad_program);
  cmd->set_quad_state();
  auto data = static_cast<float*>(cmd->allocate_vertex_data(0, 6 * sizeof(float), 2 * sizeof(float)));
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

  float uniform_data[] = {
    // Size
    width, height,
    // Offset
    (1.0f - width) * 0.5f,
    (1.0f - height) * 0.5f};

  cmd->push_constants(uniform_data, 0, sizeof(uniform_data));

  cmd->set_vertex_attrib(0, 0, VK_FORMAT_R32G32_SFLOAT, 0);
  cmd->set_depth_test(false, false);
  cmd->set_depth_compare(VK_COMPARE_OP_ALWAYS);
  cmd->set_primitive_topology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
  cmd->draw(3, 1);
}

void UpdateScreen(n64::Core& core, Util::IntrusivePtr<Image> image) {
  wsi->begin_frame();

  if (!image) {
    auto info = Vulkan::ImageCreateInfo::immutable_2d_image(800, 600, VK_FORMAT_R8G8B8A8_UNORM);
    info.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                 VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    info.misc = IMAGE_MISC_MUTABLE_SRGB_BIT;
    info.initial_layout = VK_IMAGE_LAYOUT_UNDEFINED;
    image = wsi->get_device().create_image(info);

    auto cmd = wsi->get_device().request_command_buffer();

    cmd->image_barrier(*image,
                       VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                       VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0,
                       VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT);
    cmd->clear_image(*image, {});
    cmd->image_barrier(*image,
                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                       VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT,
                       VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_SHADER_READ_BIT);
    wsi->get_device().submit(cmd);
  }

  Util::IntrusivePtr<CommandBuffer> cmd = wsi->get_device().request_command_buffer();

  cmd->begin_render_pass(wsi->get_device().get_swapchain_render_pass(SwapchainRenderPass::ColorOnly));
  DrawFullscreenTexturedQuad(image, cmd);

  cmd->end_render_pass();
  wsi->get_device().submit(cmd);
  wsi->end_frame();
}

void UpdateScreenParallelRdp(n64::Core& core, n64::VI& vi) {
  command_processor->set_vi_register(VIRegister::Control,      vi.status.raw);
  command_processor->set_vi_register(VIRegister::Origin,       vi.origin);
  command_processor->set_vi_register(VIRegister::Width,        vi.width);
  command_processor->set_vi_register(VIRegister::Intr,         vi.intr);
  command_processor->set_vi_register(VIRegister::VCurrentLine, vi.current);
  command_processor->set_vi_register(VIRegister::Timing,       vi.burst.raw);
  command_processor->set_vi_register(VIRegister::VSync,        vi.vsync);
  command_processor->set_vi_register(VIRegister::HSync,        vi.hsync);
  command_processor->set_vi_register(VIRegister::Leap,         vi.hsyncLeap.raw);
  command_processor->set_vi_register(VIRegister::HStart,       vi.hstart.raw);
  command_processor->set_vi_register(VIRegister::VStart,       vi.vstart.raw);
  command_processor->set_vi_register(VIRegister::VBurst,       vi.vburst);
  command_processor->set_vi_register(VIRegister::XScale,       vi.xscale.raw);
  command_processor->set_vi_register(VIRegister::YScale,       vi.yscale.raw);

  RDP::ScanoutOptions opts;
  opts.persist_frame_on_invalid_input = true;
  opts.vi.aa = true;
  opts.vi.scale = true;
  opts.vi.dither_filter = true;
  opts.vi.divot_filter = true;
  opts.vi.gamma_dither = true;
  opts.downscale_steps = true;
  opts.crop_overscan_pixels = true;
  Util::IntrusivePtr<Image> image = command_processor->scanout(opts);
  UpdateScreen(core, image);
  command_processor->begin_frame_context();
}

void UpdateScreenParallelRdpNoGame(n64::Core& core) {
  UpdateScreen(core, static_cast<Util::IntrusivePtr<Image>>(nullptr));
}

void ParallelRdpEnqueueCommand(int command_length, u32* buffer) {
  command_processor->enqueue_command(command_length, buffer);
}

void ParallelRdpOnFullSync() {
  command_processor->wait_for_timeline(command_processor->signal_timeline());
}