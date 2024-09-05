#pragma once
#include <Core.hpp>
#include <Discord.hpp>
#include <QThread>
#include <RenderWidget.hpp>
#include <SDL3/SDL_gamepad.h>
#include <SettingsWindow.hpp>
#include <memory>

class EmuThread : public QThread {
  Q_OBJECT
  std::shared_ptr<QtInstanceFactory> instance;
  std::shared_ptr<Vulkan::WSIPlatform> wsiPlatform;
  std::shared_ptr<ParallelRDP::WindowInfo> windowInfo;

public:
  explicit EmuThread(const std::shared_ptr<QtInstanceFactory> &instance,
                     const std::shared_ptr<Vulkan::WSIPlatform> &wsiPlatform,
                     const std::shared_ptr<ParallelRDP::WindowInfo> &windowInfo, SettingsWindow &) noexcept;

  [[noreturn]] void run() noexcept override;

  SDL_Gamepad *controller = nullptr;
  ParallelRDP parallel;
  n64::Core core;
  SettingsWindow &settings;

  void TogglePause() {
    core.TogglePause();
    Util::RPC::GetInstance().Update(core.pause ? Util::RPC::Paused : Util::RPC::GetInstance().GetState(),
                                    core.cpu->GetMem().rom.gameNameDB,
                                    core.cpu->GetMem().mmio.si.pif.movie.GetFilename());
  }

  void SetRender(bool v) { core.render = v; }

  void Reset() {
    core.pause = true;
    core.Stop();
    core.LoadROM(core.rom);
    core.pause = false;
  }

  void Stop() {
    Util::RPC::GetInstance().Update(Util::RPC::Idling);
    core.rom = {};
    core.pause = true;
    core.Stop();
  }
};
