#include <Core.hpp>
#include <EmuThread.hpp>

EmuThread::EmuThread(const std::shared_ptr<n64::Core> &core, RenderWidget &renderWidget,
                     SettingsWindow &settings) noexcept : renderWidget(renderWidget), core(core), settings(settings) {}

void EmuThread::run() noexcept {
  core->parallel.Init(renderWidget.qtVkInstanceFactory, renderWidget.wsiPlatform, renderWidget.windowInfo,
                      core->cpu->GetMem().GetRDRAMPtr());

  while (!isInterruptionRequested()) {
    if (!core->pause) {
      core->Run(settings.getVolumeL(), settings.getVolumeR());
    }

    if (core->render) {
      core->parallel.UpdateScreen(core->cpu->GetMem().mmio.vi);
    }
  }

  SetRender(false);
  Stop();
}

void EmuThread::TogglePause() const noexcept {
  core->TogglePause();
  Util::RPC::GetInstance().Update(core->pause ? Util::RPC::Paused : Util::RPC::GetInstance().GetState(),
                                  core->cpu->GetMem().rom.gameNameDB,
                                  core->cpu->GetMem().mmio.si.pif.movie.GetFilename());
}

void EmuThread::SetRender(const bool v) const noexcept { core->render = v; }

void EmuThread::Reset() const noexcept {
  core->pause = true;
  core->Stop();
  core->LoadROM(core->rom);
  core->pause = false;
}

void EmuThread::Stop() const noexcept {
  Util::RPC::GetInstance().Update(Util::RPC::Idling);
  core->rom = {};
  core->pause = true;
  core->Stop();
}
