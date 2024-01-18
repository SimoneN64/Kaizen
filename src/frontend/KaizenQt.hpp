#pragma once
#include <MainWindow.hpp>
#include <EmuThread.hpp>

class KaizenQt : public QObject {
  Q_OBJECT
public:
  KaizenQt() noexcept;
  void LoadROM(const QString& path) noexcept;
private:
  void ConnectMainWindowSignalsToSlots() noexcept;
  MainWindowController* mainWindow;
  EmuThread* emuThread;
};