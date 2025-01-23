#pragma once
#include <Discord.hpp>
#include <EmuThread.hpp>
#include <MainWindow.hpp>
#include <SettingsWindow.hpp>
#include <log.hpp>

enum class CompositorCategory { Windows, MacOS, XCB, Wayland };

static CompositorCategory GetOSCompositorCategory() {
  const QString platform_name = QGuiApplication::platformName();
  if (platform_name == QStringLiteral("windows"))
    return CompositorCategory::Windows;
  if (platform_name == QStringLiteral("xcb"))
    return CompositorCategory::XCB;
  if (platform_name == QStringLiteral("wayland") || platform_name == QStringLiteral("wayland-egl"))
    return CompositorCategory::Wayland;
  if (platform_name == QStringLiteral("cocoa") || platform_name == QStringLiteral("ios"))
    return CompositorCategory::MacOS;

  Util::error("Unknown Qt platform!");
  return CompositorCategory::Windows;
}

class KaizenQt : public QWidget {
  Q_OBJECT
public:
  KaizenQt() noexcept;
  void LoadTAS(const QString &path) const noexcept;
  void LoadROM(const QString &path) const noexcept;
  void dropEvent(QDropEvent *) override;
  void dragEnterEvent(QDragEnterEvent *) override;
  void keyPressEvent(QKeyEvent *) override;
  void keyReleaseEvent(QKeyEvent *) override;

private:
  void Quit() const noexcept;
  void ConnectMainWindowSignalsToSlots() noexcept;
  std::unique_ptr<MainWindow> mainWindow;
  std::unique_ptr<SettingsWindow> settingsWindow;
  std::unique_ptr<EmuThread> emuThread;
  // std::unique_ptr<Debugger> debugger;
  std::shared_ptr<n64::Core> core;
};
