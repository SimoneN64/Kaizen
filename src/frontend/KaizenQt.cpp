#include <KaizenQt.hpp>
#include <QMessageBox>
#include <QApplication>
#include <QDropEvent>
#include <QMimeData>

KaizenQt::KaizenQt() noexcept : QWidget(nullptr) {
  mainWindow = new MainWindowController();
  emuThread = new EmuThread(
    std::move(mainWindow->view.vulkanWidget->instance),
    std::move(mainWindow->view.vulkanWidget->wsiPlatform),
    std::move(mainWindow->view.vulkanWidget->windowInfo),
    mainWindow);

  ConnectMainWindowSignalsToSlots();

  setAcceptDrops(true);
  setFocusPolicy(Qt::StrongFocus);
  setFocus();
  grabKeyboard();
  mainWindow->show();
  settingsWindow = new SettingsWindow;
  settingsWindow->hide();
  emuThread->core = new n64::Core();
}

void KaizenQt::ConnectMainWindowSignalsToSlots() noexcept {
  connect(mainWindow, &MainWindowController::OpenSettings, this, [this]() {
    settingsWindow->show();
  });
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

void KaizenQt::keyPressEvent(QKeyEvent *e) {
  emuThread->core->pause = true;
  auto k = static_cast<Qt::Key>(e->key());
  if(k == Qt::Key_Z)                            emuThread->core->cpu->mem.mmio.si.pif.joybusDevices[0].controller.UpdateButton(n64::Controller::Key::Z, true);
  if(k == Qt::Key_X)                            emuThread->core->cpu->mem.mmio.si.pif.joybusDevices[0].controller.UpdateButton(n64::Controller::Key::A, true);
  if(k == Qt::Key_C)                            emuThread->core->cpu->mem.mmio.si.pif.joybusDevices[0].controller.UpdateButton(n64::Controller::Key::B, true);
  if(k == Qt::Key_Return || k == Qt::Key_Enter) emuThread->core->cpu->mem.mmio.si.pif.joybusDevices[0].controller.UpdateButton(n64::Controller::Key::Start, true);
  if(k == Qt::Key_I)                            emuThread->core->cpu->mem.mmio.si.pif.joybusDevices[0].controller.UpdateButton(n64::Controller::Key::DUp, true);
  if(k == Qt::Key_K)                            emuThread->core->cpu->mem.mmio.si.pif.joybusDevices[0].controller.UpdateButton(n64::Controller::Key::DDown, true);
  if(k == Qt::Key_J)                            emuThread->core->cpu->mem.mmio.si.pif.joybusDevices[0].controller.UpdateButton(n64::Controller::Key::DLeft, true);
  if(k == Qt::Key_L)                            emuThread->core->cpu->mem.mmio.si.pif.joybusDevices[0].controller.UpdateButton(n64::Controller::Key::DRight, true);
  if(k == Qt::Key_A)                            emuThread->core->cpu->mem.mmio.si.pif.joybusDevices[0].controller.UpdateButton(n64::Controller::Key::LT, true);
  if(k == Qt::Key_S)                            emuThread->core->cpu->mem.mmio.si.pif.joybusDevices[0].controller.UpdateButton(n64::Controller::Key::RT, true);
  if(k == Qt::Key_8)                            emuThread->core->cpu->mem.mmio.si.pif.joybusDevices[0].controller.UpdateButton(n64::Controller::Key::CUp, true);
  if(k == Qt::Key_2)                            emuThread->core->cpu->mem.mmio.si.pif.joybusDevices[0].controller.UpdateButton(n64::Controller::Key::CDown, true);
  if(k == Qt::Key_4)                            emuThread->core->cpu->mem.mmio.si.pif.joybusDevices[0].controller.UpdateButton(n64::Controller::Key::CLeft, true);
  if(k == Qt::Key_6)                            emuThread->core->cpu->mem.mmio.si.pif.joybusDevices[0].controller.UpdateButton(n64::Controller::Key::CRight, true);
  if(k == Qt::Key_Left)                         emuThread->core->cpu->mem.mmio.si.pif.joybusDevices[0].controller.UpdateAxis(n64::Controller::Axis::X, -86);
  if(k == Qt::Key_Right)                        emuThread->core->cpu->mem.mmio.si.pif.joybusDevices[0].controller.UpdateAxis(n64::Controller::Axis::X, 86);
  if(k == Qt::Key_Up)                           emuThread->core->cpu->mem.mmio.si.pif.joybusDevices[0].controller.UpdateAxis(n64::Controller::Axis::Y, 86);
  if(k == Qt::Key_Down)                         emuThread->core->cpu->mem.mmio.si.pif.joybusDevices[0].controller.UpdateAxis(n64::Controller::Axis::Y, -86);
  emuThread->core->pause = false;
  QWidget::keyPressEvent(e);
}

void KaizenQt::keyReleaseEvent(QKeyEvent *e) {
  emuThread->core->pause = true;
  auto k = static_cast<Qt::Key>(e->key());
  if(k == Qt::Key_Z)                            emuThread->core->cpu->mem.mmio.si.pif.joybusDevices[0].controller.UpdateButton(n64::Controller::Key::Z, false);
  if(k == Qt::Key_X)                            emuThread->core->cpu->mem.mmio.si.pif.joybusDevices[0].controller.UpdateButton(n64::Controller::Key::A, false);
  if(k == Qt::Key_C)                            emuThread->core->cpu->mem.mmio.si.pif.joybusDevices[0].controller.UpdateButton(n64::Controller::Key::B, false);
  if(k == Qt::Key_Return || k == Qt::Key_Enter) emuThread->core->cpu->mem.mmio.si.pif.joybusDevices[0].controller.UpdateButton(n64::Controller::Key::Start, false);
  if(k == Qt::Key_I)                            emuThread->core->cpu->mem.mmio.si.pif.joybusDevices[0].controller.UpdateButton(n64::Controller::Key::DUp, false);
  if(k == Qt::Key_K)                            emuThread->core->cpu->mem.mmio.si.pif.joybusDevices[0].controller.UpdateButton(n64::Controller::Key::DDown, false);
  if(k == Qt::Key_J)                            emuThread->core->cpu->mem.mmio.si.pif.joybusDevices[0].controller.UpdateButton(n64::Controller::Key::DLeft, false);
  if(k == Qt::Key_L)                            emuThread->core->cpu->mem.mmio.si.pif.joybusDevices[0].controller.UpdateButton(n64::Controller::Key::DRight, false);
  if(k == Qt::Key_A)                            emuThread->core->cpu->mem.mmio.si.pif.joybusDevices[0].controller.UpdateButton(n64::Controller::Key::LT, false);
  if(k == Qt::Key_S)                            emuThread->core->cpu->mem.mmio.si.pif.joybusDevices[0].controller.UpdateButton(n64::Controller::Key::RT, false);
  if(k == Qt::Key_8)                            emuThread->core->cpu->mem.mmio.si.pif.joybusDevices[0].controller.UpdateButton(n64::Controller::Key::CUp, false);
  if(k == Qt::Key_2)                            emuThread->core->cpu->mem.mmio.si.pif.joybusDevices[0].controller.UpdateButton(n64::Controller::Key::CDown, false);
  if(k == Qt::Key_4)                            emuThread->core->cpu->mem.mmio.si.pif.joybusDevices[0].controller.UpdateButton(n64::Controller::Key::CLeft, false);
  if(k == Qt::Key_6)                            emuThread->core->cpu->mem.mmio.si.pif.joybusDevices[0].controller.UpdateButton(n64::Controller::Key::CRight, false);
  if(k == Qt::Key_Left)                         emuThread->core->cpu->mem.mmio.si.pif.joybusDevices[0].controller.UpdateAxis(n64::Controller::Axis::X, 0);
  if(k == Qt::Key_Right)                        emuThread->core->cpu->mem.mmio.si.pif.joybusDevices[0].controller.UpdateAxis(n64::Controller::Axis::X, 0);
  if(k == Qt::Key_Up)                           emuThread->core->cpu->mem.mmio.si.pif.joybusDevices[0].controller.UpdateAxis(n64::Controller::Axis::Y, 0);
  if(k == Qt::Key_Down)                         emuThread->core->cpu->mem.mmio.si.pif.joybusDevices[0].controller.UpdateAxis(n64::Controller::Axis::Y, 0);
  emuThread->core->pause = false;
  QWidget::keyPressEvent(e);
}