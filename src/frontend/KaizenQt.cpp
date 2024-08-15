#include <KaizenQt.hpp>
#include <QMessageBox>
#include <QApplication>
#include <QDropEvent>
#include <QMimeData>
#include <filesystem>

namespace fs = std::filesystem;

KaizenQt::KaizenQt() noexcept : QWidget(nullptr) {
  mainWindow = std::make_unique<MainWindowController>();
  settingsWindow = std::make_unique<SettingsWindow>();
  emuThread = std::make_unique<EmuThread>(
    std::move(mainWindow->view.vulkanWidget->instance),
    std::move(mainWindow->view.vulkanWidget->wsiPlatform),
    std::move(mainWindow->view.vulkanWidget->windowInfo),
    *settingsWindow);

  ConnectMainWindowSignalsToSlots();

  setAcceptDrops(true);
  setFocusPolicy(Qt::StrongFocus);
  setFocus();
  grabKeyboard();
  mainWindow->show();
  settingsWindow->hide();
  connect(settingsWindow.get(), &SettingsWindow::regrabKeyboard, this, [&]() {
    grabKeyboard();
  });
}
void KaizenQt::ConnectMainWindowSignalsToSlots() noexcept {
  connect(mainWindow.get(), &MainWindowController::OpenSettings, this, [this]() {
    settingsWindow->show();
  });
  connect(mainWindow.get(), &MainWindowController::OpenROM, this, &KaizenQt::LoadROM);
  connect(mainWindow.get(), &MainWindowController::Exit, this, []() {
    QApplication::quit();
  });
  connect(mainWindow.get(), &MainWindowController::Reset, emuThread.get(), &EmuThread::Reset);
  connect(mainWindow.get(), &MainWindowController::Stop, emuThread.get(), &EmuThread::Stop);
  connect(mainWindow.get(), &MainWindowController::Pause, emuThread.get(), &EmuThread::TogglePause);
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

void KaizenQt::LoadROM(const QString& fileName) noexcept {
  mainWindow->view.actionPause->setEnabled(true);
  mainWindow->view.actionReset->setEnabled(true);
  mainWindow->view.actionStop->setEnabled(true);
  emuThread->start();
  emuThread->core.LoadROM(fileName.toStdString());
}

void KaizenQt::closeEvent(QCloseEvent*) {
  emuThread->Stop();
}

void KaizenQt::LoadTAS(const QString& fileName) const noexcept {
  emuThread->core.LoadTAS(fs::path(fileName.toStdString()));
}

void KaizenQt::keyPressEvent(QKeyEvent *e) {
  emuThread->core.pause = true;
  auto k = static_cast<Qt::Key>(e->key());
  if(k == settingsWindow->keyMap[0])  emuThread->core.cpu->GetMem().mmio.si.pif.UpdateButton(0, n64::Controller::Key::A, true);
  if(k == settingsWindow->keyMap[1])  emuThread->core.cpu->GetMem().mmio.si.pif.UpdateButton(0, n64::Controller::Key::B, true);
  if(k == settingsWindow->keyMap[2])  emuThread->core.cpu->GetMem().mmio.si.pif.UpdateButton(0, n64::Controller::Key::Z, true);
  if(k == settingsWindow->keyMap[3])  emuThread->core.cpu->GetMem().mmio.si.pif.UpdateButton(0, n64::Controller::Key::Start, true);
  if(k == settingsWindow->keyMap[4])  emuThread->core.cpu->GetMem().mmio.si.pif.UpdateButton(0, n64::Controller::Key::LT, true);
  if(k == settingsWindow->keyMap[5])  emuThread->core.cpu->GetMem().mmio.si.pif.UpdateButton(0, n64::Controller::Key::RT, true);
  if(k == settingsWindow->keyMap[6])  emuThread->core.cpu->GetMem().mmio.si.pif.UpdateButton(0, n64::Controller::Key::DUp, true);
  if(k == settingsWindow->keyMap[7])  emuThread->core.cpu->GetMem().mmio.si.pif.UpdateButton(0, n64::Controller::Key::DDown, true);
  if(k == settingsWindow->keyMap[8])  emuThread->core.cpu->GetMem().mmio.si.pif.UpdateButton(0, n64::Controller::Key::DLeft, true);
  if(k == settingsWindow->keyMap[9])  emuThread->core.cpu->GetMem().mmio.si.pif.UpdateButton(0, n64::Controller::Key::DRight, true);
  if(k == settingsWindow->keyMap[10]) emuThread->core.cpu->GetMem().mmio.si.pif.UpdateButton(0, n64::Controller::Key::CUp, true);
  if(k == settingsWindow->keyMap[11]) emuThread->core.cpu->GetMem().mmio.si.pif.UpdateButton(0, n64::Controller::Key::CDown, true);
  if(k == settingsWindow->keyMap[12]) emuThread->core.cpu->GetMem().mmio.si.pif.UpdateButton(0, n64::Controller::Key::CLeft, true);
  if(k == settingsWindow->keyMap[13]) emuThread->core.cpu->GetMem().mmio.si.pif.UpdateButton(0, n64::Controller::Key::CRight, true);
  if(k == settingsWindow->keyMap[14]) emuThread->core.cpu->GetMem().mmio.si.pif.UpdateAxis(0, n64::Controller::Axis::Y, 86);
  if(k == settingsWindow->keyMap[15]) emuThread->core.cpu->GetMem().mmio.si.pif.UpdateAxis(0, n64::Controller::Axis::Y, -86);
  if(k == settingsWindow->keyMap[16]) emuThread->core.cpu->GetMem().mmio.si.pif.UpdateAxis(0, n64::Controller::Axis::X, -86);
  if(k == settingsWindow->keyMap[17]) emuThread->core.cpu->GetMem().mmio.si.pif.UpdateAxis(0, n64::Controller::Axis::X, 86);
  emuThread->core.pause = false;
  QWidget::keyPressEvent(e);
}

void KaizenQt::keyReleaseEvent(QKeyEvent *e) {
  emuThread->core.pause = true;
  auto k = static_cast<Qt::Key>(e->key());
  if (k == settingsWindow->keyMap[0])  emuThread->core.cpu->GetMem().mmio.si.pif.UpdateButton(0, n64::Controller::Key::A, false);
  if (k == settingsWindow->keyMap[1])  emuThread->core.cpu->GetMem().mmio.si.pif.UpdateButton(0, n64::Controller::Key::B, false);
  if (k == settingsWindow->keyMap[2])  emuThread->core.cpu->GetMem().mmio.si.pif.UpdateButton(0, n64::Controller::Key::Z, false);
  if (k == settingsWindow->keyMap[3])  emuThread->core.cpu->GetMem().mmio.si.pif.UpdateButton(0, n64::Controller::Key::Start, false);
  if (k == settingsWindow->keyMap[4])  emuThread->core.cpu->GetMem().mmio.si.pif.UpdateButton(0, n64::Controller::Key::LT, false);
  if (k == settingsWindow->keyMap[5])  emuThread->core.cpu->GetMem().mmio.si.pif.UpdateButton(0, n64::Controller::Key::RT, false);
  if (k == settingsWindow->keyMap[6])  emuThread->core.cpu->GetMem().mmio.si.pif.UpdateButton(0, n64::Controller::Key::DUp, false);
  if (k == settingsWindow->keyMap[7])  emuThread->core.cpu->GetMem().mmio.si.pif.UpdateButton(0, n64::Controller::Key::DDown, false);
  if (k == settingsWindow->keyMap[8])  emuThread->core.cpu->GetMem().mmio.si.pif.UpdateButton(0, n64::Controller::Key::DLeft, false);
  if (k == settingsWindow->keyMap[9])  emuThread->core.cpu->GetMem().mmio.si.pif.UpdateButton(0, n64::Controller::Key::DRight, false);
  if (k == settingsWindow->keyMap[10]) emuThread->core.cpu->GetMem().mmio.si.pif.UpdateButton(0, n64::Controller::Key::CUp, false);
  if (k == settingsWindow->keyMap[11]) emuThread->core.cpu->GetMem().mmio.si.pif.UpdateButton(0, n64::Controller::Key::CDown, false);
  if (k == settingsWindow->keyMap[12]) emuThread->core.cpu->GetMem().mmio.si.pif.UpdateButton(0, n64::Controller::Key::CLeft, false);
  if (k == settingsWindow->keyMap[13]) emuThread->core.cpu->GetMem().mmio.si.pif.UpdateButton(0, n64::Controller::Key::CRight, false);
  if (k == settingsWindow->keyMap[14]) emuThread->core.cpu->GetMem().mmio.si.pif.UpdateAxis(0, n64::Controller::Axis::Y, 0);
  if (k == settingsWindow->keyMap[15]) emuThread->core.cpu->GetMem().mmio.si.pif.UpdateAxis(0, n64::Controller::Axis::Y, 0);
  if (k == settingsWindow->keyMap[16]) emuThread->core.cpu->GetMem().mmio.si.pif.UpdateAxis(0, n64::Controller::Axis::X, 0);
  if (k == settingsWindow->keyMap[17]) emuThread->core.cpu->GetMem().mmio.si.pif.UpdateAxis(0, n64::Controller::Axis::X, 0);
  emuThread->core.pause = false;
  QWidget::keyPressEvent(e);
}