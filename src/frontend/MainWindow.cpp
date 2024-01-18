#include <QFileDialog>
#include <QMessageBox>
#include <MainWindow.hpp>

MainWindowController::MainWindowController() noexcept : vulkanWidget(new RenderWidget(this)) {
  view.setupUi(this);
  ConnectSignalsToSlots();
}

void MainWindowController::ConnectSignalsToSlots() noexcept {
  connect(view.actionOpen, &QAction::triggered, this, [this]() {
    QString file_name = QFileDialog::getOpenFileName(this);

    if (!file_name.isEmpty()) {
      view.actionPause->setEnabled(true);
      view.actionReset->setEnabled(true);
      view.actionStop->setEnabled(true);
      emit OpenROM(file_name);
    }
  });

  connect(view.actionExit, &QAction::triggered, this, [this]() {
    emit Exit();
  });

  connect(view.actionReset, &QAction::triggered, this, [this]() {
    emit Reset();
  });

  connect(view.actionStop, &QAction::triggered, this, [this]() {
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
        "sliice is licensed under the BSD 3-clause license.\n"
        "Nintendo 64 is a registered trademarks of Nintendo Co., Ltd."));
  });
}