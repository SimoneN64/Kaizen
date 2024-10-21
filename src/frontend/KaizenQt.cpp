#include <Core.hpp>
#include <KaizenQt.hpp>
#include <QApplication>
#include <QDropEvent>
#include <QMessageBox>
#include <QMimeData>

namespace fs = std::filesystem;

KaizenQt::KaizenQt() noexcept : QWidget(nullptr) {
  core = std::make_shared<n64::Core>();
  mainWindow = std::make_unique<MainWindow>(core);
  settingsWindow = std::make_unique<SettingsWindow>();
  emuThread = std::make_unique<EmuThread>(core, *mainWindow->fpsCounter, *mainWindow->vulkanWidget, *settingsWindow);
  debugger = std::make_unique<Debugger>();

  ConnectMainWindowSignalsToSlots();
  Util::RPC::GetInstance().Update(Util::RPC::Idling);

  setAcceptDrops(true);
  setFocusPolicy(Qt::StrongFocus);
  setFocus();
  grabKeyboard();
  mainWindow->show();
  debugger->hide();
  settingsWindow->hide();
}

void KaizenQt::ConnectMainWindowSignalsToSlots() noexcept {
  connect(settingsWindow.get(), &SettingsWindow::regrabKeyboard, this, [&] { grabKeyboard(); });

  connect(settingsWindow.get(), &SettingsWindow::gotClosed, this,
          [&] { mainWindow->vulkanWidget->wsiPlatform->EnableEventPolling(); });

  connect(settingsWindow.get(), &SettingsWindow::gotOpened, this,
          [&] { mainWindow->vulkanWidget->wsiPlatform->DisableEventPolling(); });

  connect(mainWindow.get(), &MainWindow::OpenSettings, this, [this] { settingsWindow->show(); });
  connect(mainWindow.get(), &MainWindow::OpenDebugger, this, [this] { debugger->show(); });
  connect(mainWindow.get(), &MainWindow::OpenROM, this, &KaizenQt::LoadROM);
  connect(mainWindow.get(), &MainWindow::Exit, this, &KaizenQt::Quit);
  connect(mainWindow.get(), &MainWindow::Reset, emuThread.get(), &EmuThread::Reset);
  connect(mainWindow.get(), &MainWindow::Stop, this, [this] { emuThread->requestInterruption(); });
  connect(mainWindow.get(), &MainWindow::Pause, emuThread.get(), &EmuThread::TogglePause);
}

void KaizenQt::dragEnterEvent(QDragEnterEvent *event) {
  if (event->mimeData()->hasUrls()) {
    event->acceptProposedAction();
  }
}

void KaizenQt::dropEvent(QDropEvent *event) {
  const auto path = event->mimeData()->urls()[0].toLocalFile();
  LoadROM(path);
}

void KaizenQt::LoadROM(const QString &path) const noexcept {
  mainWindow->actionPause->setEnabled(true);
  mainWindow->actionReset->setEnabled(true);
  mainWindow->actionStop->setEnabled(true);
  emuThread->start();
  emuThread->core->LoadROM(path.toStdString());
  const auto gameNameDB = emuThread->core->cpu->GetMem().rom.gameNameDB;
  mainWindow->setWindowTitle(emuThread->core->cpu->GetMem().rom.gameNameDB.c_str());
  Util::RPC::GetInstance().Update(Util::RPC::Playing, gameNameDB);
}

void KaizenQt::Quit() const noexcept {
  if (emuThread) {
    emuThread->requestInterruption();
    while (emuThread->isRunning())
      ;
  }
  QApplication::quit();
}

void KaizenQt::LoadTAS(const QString &path) const noexcept {
  if (emuThread->core->LoadTAS(fs::path(path.toStdString()))) {
    const auto gameNameDB = emuThread->core->cpu->GetMem().rom.gameNameDB;
    const auto movieName = fs::path(path.toStdString()).stem().string();
    Util::RPC::GetInstance().Update(Util::RPC::MovieReplay, gameNameDB, movieName);
    return;
  }

  Util::panic("Could not load TAS movie {}!", path.toStdString());
}

void KaizenQt::keyPressEvent(QKeyEvent *e) {
  if (settingsWindow->inputSettings->selectedDeviceIsNotKeyboard)
    return;

  emuThread->core->pause = true;
  n64::Mem &mem = emuThread->core->cpu->GetMem();
  n64::PIF &pif = mem.mmio.si.pif;

  const auto k = static_cast<Qt::Key>(e->key());
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

  emuThread->core->pause = false;
  QWidget::keyPressEvent(e);
}

void KaizenQt::keyReleaseEvent(QKeyEvent *e) {
  if (settingsWindow->inputSettings->selectedDeviceIsNotKeyboard)
    return;

  emuThread->core->pause = true;
  n64::Mem &mem = emuThread->core->cpu->GetMem();
  n64::PIF &pif = mem.mmio.si.pif;

  const auto k = static_cast<Qt::Key>(e->key());
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

  emuThread->core->pause = false;
  QWidget::keyReleaseEvent(e);
}
