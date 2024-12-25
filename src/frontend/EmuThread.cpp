#include <Core.hpp>
#include <EmuThread.hpp>

EmuThread::EmuThread(const std::shared_ptr<n64::Core> &core, QLabel &fps, RenderWidget &renderWidget,
                     SettingsWindow &settings) noexcept :
    renderWidget(renderWidget), core(core), settings(settings), fps(fps) {}

void EmuThread::run() noexcept {
  core->parallel.Init(renderWidget.qtVkInstanceFactory, renderWidget.wsiPlatform, renderWidget.windowInfo,
                      core->cpu->GetMem().GetRDRAMPtr());

  auto lastSample = std::chrono::high_resolution_clock::now();
  auto avgFps = 16.667;
  auto sampledFps = 0;
  static bool oneSecondPassed = false;

  fps.setText(fmt::format("{:.2f} FPS", 1000.0 / avgFps).c_str());

  while (!isInterruptionRequested()) {
    const auto startFrameTime = std::chrono::high_resolution_clock::now();
    if (!core->pause) {
      core->Run(settings.getVolumeL(), settings.getVolumeR());
    }

    if (core->render) {
      core->parallel.UpdateScreen(core->cpu->GetMem().mmio.vi);
    }

    const auto endFrameTime = std::chrono::high_resolution_clock::now();
    using namespace std::chrono_literals;
    const auto frameTimeMs = std::chrono::duration<double>(endFrameTime - startFrameTime) / 1ms;
    avgFps += frameTimeMs;

    sampledFps++;

    if (const auto elapsedSinceLastSample = std::chrono::duration<double>(endFrameTime - lastSample) / 1s;
        elapsedSinceLastSample >= 1.0) {
      if (!oneSecondPassed) {
        oneSecondPassed = true;
        continue;
      }
      lastSample = endFrameTime;
      avgFps /= sampledFps;
      sampledFps = 0;
      fps.setText(fmt::format("{:.2f} FPS", 1000.0 / avgFps).c_str());
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
