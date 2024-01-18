#include <KaizenQt.hpp>
#include <QMessageBox>
#include <QApplication>

KaizenQt::KaizenQt() noexcept
  : mainWindow(new MainWindowController),
  emuThread(new EmuThread(mainWindow))
{
  ConnectMainWindowSignalsToSlots();

  mainWindow->show();
}

void KaizenQt::ConnectMainWindowSignalsToSlots() noexcept
{
  connect(mainWindow, &MainWindowController::OpenROM, this, &KaizenQt::LoadROM);
  connect(mainWindow, &MainWindowController::Exit, this, []() {
    QApplication::quit();
    });
  connect(mainWindow, &MainWindowController::Reset, emuThread, &EmuThread::Reset);
  connect(mainWindow, &MainWindowController::Stop, emuThread, &EmuThread::Stop);
  connect(mainWindow, &MainWindowController::Pause, emuThread, &EmuThread::TogglePause);
}

void KaizenQt::LoadROM(const QString& file_name) noexcept
{
  emuThread->core.LoadROM(file_name.toStdString());
}
