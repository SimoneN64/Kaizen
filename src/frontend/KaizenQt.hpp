#pragma once
#include <EmuThread.hpp>
#include <MainWindow.hpp>

enum class CompositorCategory {
  Windows, MacOS, XCB, Wayland
};

static inline CompositorCategory GetOSCompositorCategory() {
  const QString platform_name = QGuiApplication::platformName();
  if (platform_name == QStringLiteral("windows"))
    return CompositorCategory::Windows;
  else if (platform_name == QStringLiteral("xcb"))
    return CompositorCategory::XCB;
  else if (platform_name == QStringLiteral("wayland") ||
    platform_name == QStringLiteral("wayland-egl"))
    return CompositorCategory::Wayland;
  else if (platform_name == QStringLiteral("cocoa") || platform_name == QStringLiteral("ios"))
    return CompositorCategory::MacOS;

  Util::error("Unknown Qt platform!");
  return CompositorCategory::Windows;
}

class KaizenQt : public QWidget {
  Q_OBJECT
public:
  KaizenQt() noexcept;
  void LoadROM(const QString& path) noexcept;
  void dropEvent(QDropEvent*) override;
  void dragEnterEvent(QDragEnterEvent*) override;
protected:
  void keyPressEvent(QKeyEvent* event) override;
private:
  void ConnectMainWindowSignalsToSlots() noexcept;
  MainWindowController* mainWindow;
  EmuThread* emuThread;
};