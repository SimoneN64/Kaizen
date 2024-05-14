#include <EmuThread.hpp>
#include <RenderWidget.hpp>
#include <ParallelRDPWrapper.hpp>
#include "Audio.hpp"

EmuThread::EmuThread(std::unique_ptr<QtInstanceFactory>&& instance_, std::unique_ptr<Vulkan::WSIPlatform>&& wsiPlatform_, std::unique_ptr<ParallelRDP::WindowInfo>&& windowInfo_, QObject* parent_object) noexcept
  : QThread(parent_object), instance(std::move(instance_)), wsiPlatform(std::move(wsiPlatform_)), windowInfo(std::move(windowInfo_)), parallel(instance.get(), std::move(wsiPlatform), std::move(windowInfo)) {}

[[noreturn]] void EmuThread::run() noexcept {
  while (true) {
    if (!core->pause) {
      core->Run(settings->getVolumeL(), settings->getVolumeR());
      if(core->render) {
        parallel.UpdateScreen(core->cpu->GetMem().mmio.vi);
      }
    } else {
      if(core->render) {
        parallel.UpdateScreen(core->cpu->GetMem().mmio.vi, true);
      }
    }
  }
}