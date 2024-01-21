#include <QFileDialog>
#include <QMessageBox>
#include <QKeyEvent>
#include <MainWindow.hpp>

MainWindowController::MainWindowController() noexcept {
  view.setupUi(this);
  view.actionPause->setDisabled(true);
  view.actionReset->setDisabled(true);
  view.actionStop->setDisabled(true);
  view.vulkanWidget->hide();
  ConnectSignalsToSlots();
  setFocusPolicy(Qt::FocusPolicy::StrongFocus);
  setFocus();
}

void MainWindowController::ConnectSignalsToSlots() noexcept {
  connect(view.actionOpen, &QAction::triggered, this, [this]() {
    QString file_name = QFileDialog::getOpenFileName(this);

    if (!file_name.isEmpty()) {
      view.actionPause->setEnabled(true);
      view.actionReset->setEnabled(true);
      view.actionStop->setEnabled(true);
      emit OpenROM(file_name);
      view.vulkanWidget->show();
    }
  });

  connect(view.actionExit, &QAction::triggered, this, [this]() {
    emit Exit();
  });

  connect(view.actionReset, &QAction::triggered, this, [this]() {
    emit Reset();
  });

  connect(view.actionStop, &QAction::triggered, this, [this]() {
    view.vulkanWidget->hide();
    view.actionPause->setDisabled(true);
    view.actionReset->setDisabled(true);
    view.actionStop->setDisabled(true);
    emit Stop();
  });

  connect(view.actionPause, &QAction::triggered, this, [this]() {
    textPauseToggle = !textPauseToggle;
    view.actionPause->setText(textPauseToggle ? "Resume" : "Pause");
    emit Pause();
  });

  connect(view.actionAbout, &QAction::triggered, this, [this]() {
    QMessageBox::about(
      this, tr("About Kaizen"),
      tr("Kaizen is a Nintendo 64 emulator that strives to offer a friendly user "
        "experience and great compatibility.\n"
        "Kaizen is licensed under the BSD 3-clause license.\n"
        "Nintendo 64 is a registered trademarks of Nintendo Co., Ltd."));
  });
}

void MainWindowController::keyPressEvent(QKeyEvent* e) {
  n64::Controller data{};

  data.z = (e->key() == Qt::Key::Key_Z);
  data.a = (e->key() == Qt::Key::Key_X);
  data.b = (e->key() == Qt::Key::Key_C);
  data.start = e->key() == Qt::Key::Key_Enter || e->key() == Qt::Key::Key_Return;
  data.dp_up = (e->key() == Qt::Key::Key_I);
  data.dp_down = (e->key() == Qt::Key::Key_K);
  data.dp_left = (e->key() == Qt::Key::Key_J);
  data.dp_right = (e->key() == Qt::Key::Key_L);
  data.l = (e->key() == Qt::Key::Key_A);
  data.r = (e->key() == Qt::Key::Key_S);
  data.c_up = (e->key() == Qt::Key::Key_8);
  data.c_down = (e->key() == Qt::Key::Key_2);
  data.c_left = (e->key() == Qt::Key::Key_4);
  data.c_right = (e->key() == Qt::Key::Key_6);
  data.joy_y = (e->key() == Qt::Key::Key_Up) ? 127 : 0;
  data.joy_y = (e->key() == Qt::Key::Key_Down) ? -127 : 0;
  data.joy_x = (e->key() == Qt::Key::Key_Left) ? -127 : 0;
  data.joy_x = (e->key() == Qt::Key::Key_Right) ? 127 : 0;

  emuThread->core.pause = true;
  emuThread->core.cpu->mem.mmio.si.pif.UpdateController(data);
  emuThread->core.pause = false;
  QWidget::keyPressEvent(e);
  setFocus();
}