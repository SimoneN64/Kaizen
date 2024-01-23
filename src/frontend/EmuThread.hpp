#pragma once
#include <QThread>
#include <Core.hpp>
#include <SettingsWindow.hpp>

struct QtInstanceFactory;
struct ParallelRdpWindowInfo;
namespace Vulkan {
struct WSIPlatform;
}

class EmuThread : public QThread
{
  Q_OBJECT
  std::unique_ptr<QtInstanceFactory> instance;
  std::unique_ptr<Vulkan::WSIPlatform> wsiPlatform;
  std::unique_ptr<ParallelRdpWindowInfo> windowInfo;
public:
  explicit EmuThread(std::unique_ptr<QtInstanceFactory>&& instance, std::unique_ptr<Vulkan::WSIPlatform>&& wsiPlatform, std::unique_ptr<ParallelRdpWindowInfo>&& windowInfo, QObject* parent_object) noexcept;

  [[noreturn]] void run() noexcept override;

  n64::Core* core;
  SettingsWindow* settings;
  bool running = false;

  void TogglePause()
  {
    running = !running;
  }

  void Reset()
  {
    running = false;
    core->Stop();
    core->LoadROM(core->rom);
    running = true;
  }

  void Stop()
  {
    core->rom = {};
    running = false;
    core->Stop();
  }
};