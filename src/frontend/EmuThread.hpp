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
  RenderWidget &renderWidget;

public:
  explicit EmuThread(RenderWidget &, SettingsWindow &) noexcept;

  [[noreturn]] void run() noexcept override;

  SDL_Gamepad *controller{};
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
