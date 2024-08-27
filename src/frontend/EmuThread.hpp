#pragma once
#include <Core.hpp>
#include <Discord.hpp>
#include <QThread>
#include <RenderWidget.hpp>
#include <SDL2/SDL_gamecontroller.h>
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

  SDL_GameController *controller = nullptr;
  ParallelRDP parallel;
  n64::Core core;
  SettingsWindow &settings;

  void TogglePause() {
    core.pause = !core.pause;
    Util::UpdateRPC(core.pause ? Util::Idling : Util::Playing);
  }

  void SetRender(bool v) { core.render = v; }

  void Reset() {
    core.pause = true;
    core.Stop();
    core.LoadROM(core.rom);
    core.pause = false;
  }

  void Stop() {
    Discord_ClearPresence();
    core.rom = {};
    core.pause = true;
    core.Stop();
  }
};
