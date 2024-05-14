#include <EmuThread.hpp>
#include <RenderWidget.hpp>
#include <ParallelRDPWrapper.hpp>
#include "Audio.hpp"

EmuThread::EmuThread(std::unique_ptr<QtInstanceFactory>&& instance_, std::unique_ptr<Vulkan::WSIPlatform>&& wsiPlatform_, std::unique_ptr<ParallelRDP::WindowInfo>&& windowInfo_, SettingsWindow& settings) noexcept
  : instance(std::move(instance_)), wsiPlatform(std::move(wsiPlatform_)), windowInfo(std::move(windowInfo_)), core(parallel), settings(settings) {}

[[noreturn]] void EmuThread::run() noexcept {
  parallel.Init(instance.get(), std::move(wsiPlatform), std::move(windowInfo), core.cpu->GetMem().GetRDRAMPtr());

  while (true) {
    if (!core.pause) {
      core.Run(settings.getVolumeL(), settings.getVolumeR());
      if(core.render) {
        parallel.UpdateScreen(core.cpu->GetMem().mmio.vi);
      }
    } else {
      if(core.render) {
        parallel.UpdateScreen(core.cpu->GetMem().mmio.vi, true);
      }
    }
  }
}