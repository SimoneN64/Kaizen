#include <KaizenQt.hpp>
#include <QMessageBox>
#include <QApplication>
#include <QDropEvent>
#include <QMimeData>
#include <SDL2/SDL.h>

KaizenQt::KaizenQt() noexcept : QWidget(nullptr) {
  mainWindow = new MainWindowController();
  emuThread = new EmuThread(
    std::move(mainWindow->view.vulkanWidget->instance),
    std::move(mainWindow->view.vulkanWidget->wsiPlatform),
    std::move(mainWindow->view.vulkanWidget->windowInfo),
    mainWindow);

  ConnectMainWindowSignalsToSlots();

  setAcceptDrops(true);

  mainWindow->show();
  emuThread->core = new n64::Core();
}

void KaizenQt::ConnectMainWindowSignalsToSlots() noexcept {
  connect(mainWindow, &MainWindowController::OpenROM, this, &KaizenQt::LoadROM);
  connect(mainWindow, &MainWindowController::Exit, this, []() {
    QApplication::quit();
  });
  connect(mainWindow, &MainWindowController::Reset, emuThread, &EmuThread::Reset);
  connect(mainWindow, &MainWindowController::Stop, emuThread, &EmuThread::Stop);
  connect(mainWindow, &MainWindowController::Pause, emuThread, &EmuThread::TogglePause);
}

void KaizenQt::dragEnterEvent(QDragEnterEvent* event) {
  if (event->mimeData()->hasUrls()) {
    event->acceptProposedAction();
  }
}

void KaizenQt::dropEvent(QDropEvent* event) {
  auto path = event->mimeData()->urls()[0].toLocalFile();
  LoadROM(path);
}

void KaizenQt::LoadROM(const QString& file_name) noexcept {
  emuThread->start();
  emuThread->core->LoadROM(file_name.toStdString());
}