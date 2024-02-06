#pragma once
#include <QThread>
#include <set>
#include <Core.hpp>
#include <SettingsWindow.hpp>

struct QtInstanceFactory;
class ParallelRdpWindowInfo;
namespace Vulkan {
class WSIPlatform;
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

  void TogglePause()
  {
    core->pause = !core->pause;
  }

  void Reset()
  {
    core->pause = true;
    core->Stop();
    core->LoadROM(core->rom);
    core->pause = false;
  }

  void Stop()
  {
    core->rom = {};
    core->pause = true;
    core->Stop();
  }
};