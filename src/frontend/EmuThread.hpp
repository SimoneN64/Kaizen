#pragma once
#include <Discord.hpp>
#include <QThread>
#include <RenderWidget.hpp>
#include <SettingsWindow.hpp>
#include <memory>

namespace n64 {
struct Core;
}

class EmuThread : public QThread {
  Q_OBJECT
  RenderWidget &renderWidget;

public:
  explicit EmuThread(const std::shared_ptr<n64::Core> &, RenderWidget &, SettingsWindow &) noexcept;

  void run() noexcept override;
  void TogglePause() const noexcept;
  void SetRender(bool v) const noexcept;
  void Reset() const noexcept;
  void Stop() const noexcept;

  std::shared_ptr<n64::Core> core;
  SettingsWindow &settings;
};
