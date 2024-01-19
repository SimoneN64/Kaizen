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

  grabKeyboard();
  setAcceptDrops(true);
  setFocusPolicy(Qt::FocusPolicy::StrongFocus);
  setFocus();

  mainWindow->show();
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
  emuThread->core.LoadROM(file_name.toStdString());
}

void KaizenQt::keyPressEvent(QKeyEvent* e) {
  u32 data = 0;
  emuThread->core.cpu->mem.mmio.si.pif.UpdateController(data);
  if (e->key() == Qt::Key::Key_X) data |= (1 << 31);
  if (e->key() == Qt::Key::Key_C) data |= (1 << 30);
  if (e->key() == Qt::Key::Key_Z) data |= (1 << 29);
  if (e->key() == Qt::Key::Key_Enter
    || e->key() == Qt::Key::Key_Return) data |= (1 << 28);
  if (e->key() == Qt::Key::Key_I) data |= (1 << 27);
  if (e->key() == Qt::Key::Key_K) data |= (1 << 26);
  if (e->key() == Qt::Key::Key_J) data |= (1 << 25);
  if (e->key() == Qt::Key::Key_L) data |= (1 << 24);
  if (e->key() == Qt::Key::Key_A) data |= (1 << 21);
  if (e->key() == Qt::Key::Key_S) data |= (1 << 20);
  if (e->key() == Qt::Key::Key_8) data |= (1 << 19);
  if (e->key() == Qt::Key::Key_2) data |= (1 << 18);
  if (e->key() == Qt::Key::Key_4) data |= (1 << 17);
  if (e->key() == Qt::Key::Key_6) data |= (1 << 16);
  if (e->key() == Qt::Key::Key_Up) data |= 127;
  if (e->key() == Qt::Key::Key_Down) data |= -127;
  if (e->key() == Qt::Key::Key_Left) data |= u32(-127) << 8;
  if (e->key() == Qt::Key::Key_Right) data |= u32(127) << 8;
  emuThread->core.cpu->mem.mmio.si.pif.UpdateController(data);
}