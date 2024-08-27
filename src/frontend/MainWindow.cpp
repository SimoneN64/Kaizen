#include <MainWindow.hpp>
#include <QFileDialog>
#include <QKeyEvent>
#include <QMessageBox>
#include <QSlider>

MainWindowController::MainWindowController() noexcept {
  view.setupUi(this);
  view.actionPause->setDisabled(true);
  view.actionReset->setDisabled(true);
  view.actionStop->setDisabled(true);
  view.vulkanWidget->hide();
  ConnectSignalsToSlots();
}

void MainWindowController::ConnectSignalsToSlots() noexcept {
  connect(view.actionOpen, &QAction::triggered, this, [this]() {
    QString file_name = QFileDialog::getOpenFileName(
      this, "Nintendo 64 executable", QString(),
      "All supported types (*.zip *.ZIP *.7z *.7Z *.rar *.RAR *.tar *.TAR *.n64 *.N64 *.v64 *.V64 *.z64 *.Z64)");

    if (!file_name.isEmpty()) {
      emit OpenROM(file_name);
      view.vulkanWidget->show();
    }
  });

  connect(view.actionExit, &QAction::triggered, this, [this]() { emit Exit(); });

  connect(this, &MainWindowController::destroyed, this, [this]() { emit Exit(); });

  connect(view.actionReset, &QAction::triggered, this, [this]() { emit Reset(); });

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
    QMessageBox::about(this, tr("About Kaizen"),
                       tr("Kaizen is a Nintendo 64 emulator that strives to offer a friendly user "
                          "experience and great compatibility.\n"
                          "Kaizen is licensed under the BSD 3-clause license.\n"
                          "Nintendo 64 is a registered trademarks of Nintendo Co., Ltd."));
  });

  connect(view.actionSettings, &QAction::triggered, this, [this]() { emit OpenSettings(); });
}
