#include <ParallelRDPWrapper.hpp>
#include <EmuThread.hpp>
#include <RenderWidget.hpp>
#include "Audio.hpp"

EmuThread::EmuThread(std::unique_ptr<QtInstanceFactory>&& instance, std::unique_ptr<Vulkan::WSIPlatform>&& wsiPlatform, std::unique_ptr<ParallelRdpWindowInfo>&& windowInfo, QObject* parent_object) noexcept
  : QThread(parent_object), instance(std::move(instance)), wsiPlatform(std::move(wsiPlatform)), windowInfo(std::move(windowInfo)) {}

[[noreturn]] void EmuThread::run() noexcept {
  LoadWSIPlatform(instance.get(), std::move(wsiPlatform), std::move(windowInfo));
  LoadParallelRDP(core->cpu->mem.GetRDRAM());
  n64::InitAudio();
  while (true) {
    if (!core->pause) {
      core->Run(settings->getVolumeL(), settings->getVolumeR());
      if(core->render) {
        UpdateScreenParallelRdp(core->cpu->mem.mmio.vi);
      }
    } else {
      if(core->render) {
        UpdateScreenParallelRdpNoGame();
      }
    }
  }
}