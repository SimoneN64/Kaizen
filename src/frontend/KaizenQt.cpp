#include <KaizenQt.hpp>
#include <QApplication>
#include <QDropEvent>
#include <QMessageBox>
#include <QMimeData>
#include <filesystem>

namespace fs = std::filesystem;

KaizenQt::KaizenQt() noexcept : QWidget(nullptr) {
  mainWindow = std::make_unique<MainWindowController>();
  settingsWindow = std::make_unique<SettingsWindow>();
  emuThread = std::make_unique<EmuThread>(*mainWindow->view.vulkanWidget, *settingsWindow);

  ConnectMainWindowSignalsToSlots();
  Util::RPC::GetInstance().Update(Util::RPC::Idling);

  setAcceptDrops(true);
  setFocusPolicy(Qt::StrongFocus);
  setFocus();
  grabKeyboard();
  mainWindow->show();
  settingsWindow->hide();
  connect(settingsWindow.get(), &SettingsWindow::regrabKeyboard, this, [&]() { grabKeyboard(); });
}

void KaizenQt::ConnectMainWindowSignalsToSlots() noexcept {
  connect(mainWindow.get(), &MainWindowController::OpenSettings, this, [this]() { settingsWindow->show(); });
  connect(mainWindow.get(), &MainWindowController::OpenROM, this, &KaizenQt::LoadROM);
  connect(mainWindow.get(), &MainWindowController::Exit, this, &KaizenQt::Quit);
  connect(mainWindow.get(), &MainWindowController::Reset, emuThread.get(), &EmuThread::Reset);
  connect(mainWindow.get(), &MainWindowController::Stop, emuThread.get(), &EmuThread::Stop);
  connect(mainWindow.get(), &MainWindowController::Stop, this, [this]() { mainWindow->setWindowTitle("Kaizen"); });
  connect(mainWindow.get(), &MainWindowController::Pause, emuThread.get(), &EmuThread::TogglePause);
}

void KaizenQt::dragEnterEvent(QDragEnterEvent *event) {
  if (event->mimeData()->hasUrls()) {
    event->acceptProposedAction();
  }
}

void KaizenQt::dropEvent(QDropEvent *event) {
  auto path = event->mimeData()->urls()[0].toLocalFile();
  LoadROM(path);
}

void KaizenQt::LoadROM(const QString &fileName) noexcept {
  mainWindow->view.actionPause->setEnabled(true);
  mainWindow->view.actionReset->setEnabled(true);
  mainWindow->view.actionStop->setEnabled(true);
  emuThread->start();
  emuThread->core.LoadROM(fileName.toStdString());
  auto gameNameDB = emuThread->core.cpu->GetMem().rom.gameNameDB;
  mainWindow->setWindowTitle(emuThread->core.cpu->GetMem().rom.gameNameDB.c_str());
  Util::RPC::GetInstance().Update(Util::RPC::Playing, gameNameDB);
}

void KaizenQt::Quit() noexcept {
  if (emuThread) {
    emuThread->requestInterruption();
    while (emuThread->isRunning())
      ;
  }
  QApplication::quit();
}

void KaizenQt::LoadTAS(const QString &fileName) const noexcept {
  emuThread->core.LoadTAS(fs::path(fileName.toStdString()));
  auto gameNameDB = emuThread->core.cpu->GetMem().rom.gameNameDB;
  auto movieName = fs::path(fileName.toStdString()).stem().string();
  Util::RPC::GetInstance().Update(Util::RPC::MovieReplay, gameNameDB, movieName);
}

void KaizenQt::keyPressEvent(QKeyEvent *e) {
  emuThread->core.pause = true;
  n64::Mem &mem = emuThread->core.cpu->GetMem();
  n64::PIF &pif = mem.mmio.si.pif;

  auto k = static_cast<Qt::Key>(e->key());
  for (int i = 0; i < 14; i++) {
    if (k == settingsWindow->keyMap[i])
      pif.UpdateButton(0, static_cast<n64::Controller::Key>(i), true);
  }

  if (k == settingsWindow->keyMap[14])
    pif.UpdateAxis(0, n64::Controller::Axis::Y, 86);
  if (k == settingsWindow->keyMap[15])
    pif.UpdateAxis(0, n64::Controller::Axis::Y, -86);
  if (k == settingsWindow->keyMap[16])
    pif.UpdateAxis(0, n64::Controller::Axis::X, -86);
  if (k == settingsWindow->keyMap[17])
    pif.UpdateAxis(0, n64::Controller::Axis::X, 86);

  emuThread->core.pause = false;
  QWidget::keyPressEvent(e);
}

void KaizenQt::keyReleaseEvent(QKeyEvent *e) {
  emuThread->core.pause = true;
  n64::Mem &mem = emuThread->core.cpu->GetMem();
  n64::PIF &pif = mem.mmio.si.pif;

  auto k = static_cast<Qt::Key>(e->key());
  for (int i = 0; i < 14; i++) {
    if (k == settingsWindow->keyMap[i])
      pif.UpdateButton(0, static_cast<n64::Controller::Key>(i), false);
  }

  if (k == settingsWindow->keyMap[14])
    pif.UpdateAxis(0, n64::Controller::Axis::Y, 0);
  if (k == settingsWindow->keyMap[15])
    pif.UpdateAxis(0, n64::Controller::Axis::Y, 0);
  if (k == settingsWindow->keyMap[16])
    pif.UpdateAxis(0, n64::Controller::Axis::X, 0);
  if (k == settingsWindow->keyMap[17])
    pif.UpdateAxis(0, n64::Controller::Axis::X, 0);

  emuThread->core.pause = false;
  QWidget::keyReleaseEvent(e);
}
