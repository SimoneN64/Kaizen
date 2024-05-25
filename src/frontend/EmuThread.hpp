#pragma once
#include <RenderWidget.hpp>
#include <QThread>
#include <Core.hpp>
#include <SettingsWindow.hpp>
#include <memory>
#include <SDL2/SDL_gamecontroller.h>

class EmuThread : public QThread
{
  Q_OBJECT
  std::unique_ptr<QtInstanceFactory> instance;
  std::unique_ptr<Vulkan::WSIPlatform> wsiPlatform;
  std::unique_ptr<ParallelRDP::WindowInfo> windowInfo;
public:
  explicit EmuThread(std::unique_ptr<QtInstanceFactory>&& instance, std::unique_ptr<Vulkan::WSIPlatform>&& wsiPlatform, std::unique_ptr<ParallelRDP::WindowInfo>&& windowInfo, SettingsWindow&) noexcept;

  [[noreturn]] void run() noexcept override;

  Util::AutoRelease<SDL_GameController, int> controller;
  ParallelRDP parallel;
  n64::Core core;
  SettingsWindow& settings;
  bool running = false;

  void TogglePause()
  {
    running = !running;
  }

  void Reset()
  {
    running = false;
    core.Stop();
    core.LoadROM(core.rom);
    running = true;
  }

  void Stop()
  {
    core.rom = {};
    running = false;
    core.Stop();
  }
};