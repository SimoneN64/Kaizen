#include <MainWindow.hpp>
#include <QFileDialog>
#include <QKeyEvent>
#include <QMessageBox>
#include <QSlider>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>

MainWindow::MainWindow() noexcept {
  if (objectName().isEmpty())
    setObjectName("MainWindow");
  resize(800, 646);
  actionAbout = new QAction(this);
  actionAbout->setObjectName("actionAbout");
  actionOpen = new QAction(this);
  actionOpen->setObjectName("actionOpen");
  actionExit = new QAction(this);
  actionExit->setObjectName("actionExit");
  actionPause = new QAction(this);
  actionPause->setObjectName("actionPause");
  actionReset = new QAction(this);
  actionReset->setObjectName("actionReset");
  actionStop = new QAction(this);
  actionStop->setObjectName("actionStop");
  actionSettings = new QAction(this);
  actionSettings->setObjectName("actionSettings");
  centralwidget = new QWidget(this);
  centralwidget->setObjectName("centralwidget");
  verticalLayout = new QVBoxLayout(centralwidget);
  verticalLayout->setSpacing(0);
  verticalLayout->setObjectName("verticalLayout");
  verticalLayout->setContentsMargins(0, 0, 0, 0);
  horizontalLayout = new QHBoxLayout(centralwidget);
  horizontalLayout->setSpacing(0);
  verticalLayout->setObjectName("horizontalLayout");
  verticalLayout->setContentsMargins(0, 0, 0, 0);
  vulkanWidget = new RenderWidget(centralwidget);
  vulkanWidget->setObjectName("vulkanWidget");
  debugger = new ImGuiWidget(centralwidget);
  debugger->setObjectName("debugger");

  horizontalLayout->addWidget(vulkanWidget);
  horizontalLayout->addWidget(debugger);
  verticalLayout->addLayout(horizontalLayout);

  setCentralWidget(centralwidget);
  menubar = new QMenuBar(this);
  menubar->setObjectName("menubar");
  menubar->setGeometry(QRect(0, 0, 800, 22));
  menuFile = new QMenu(menubar);
  menuFile->setObjectName("menuFile");
  menuEmulation = new QMenu(menubar);
  menuEmulation->setObjectName("menuEmulation");
  menuAbout = new QMenu(menubar);
  menuAbout->setObjectName("menuAbout");
  setMenuBar(menubar);
  statusbar = new QStatusBar(this);
  statusbar->setObjectName("statusbar");
  setStatusBar(statusbar);

  menubar->addAction(menuFile->menuAction());
  menubar->addAction(menuEmulation->menuAction());
  menubar->addAction(menuAbout->menuAction());
  menuFile->addAction(actionOpen);
  menuFile->addAction(actionExit);
  menuEmulation->addAction(actionSettings);
  menuEmulation->addAction(actionPause);
  menuEmulation->addAction(actionReset);
  menuEmulation->addAction(actionStop);
  menuAbout->addAction(actionAbout);

  Retranslate();

  QMetaObject::connectSlotsByName(this);
  actionPause->setDisabled(true);
  actionReset->setDisabled(true);
  actionStop->setDisabled(true);
  vulkanWidget->hide();
  ConnectSignalsToSlots();
}

void MainWindow::Retranslate() {
  setWindowTitle(QCoreApplication::translate("MainWindow", "Kaizen", nullptr));
  actionAbout->setText(QCoreApplication::translate("MainWindow", "About Kaizen", nullptr));
#if QT_CONFIG(statustip)
  actionAbout->setStatusTip(QCoreApplication::translate("MainWindow", "About this emulator", nullptr));
#endif // QT_CONFIG(statustip)
  actionOpen->setText(QCoreApplication::translate("MainWindow", "Open...", nullptr));
#if QT_CONFIG(statustip)
  actionOpen->setStatusTip(QCoreApplication::translate("MainWindow", "Open a ROM", nullptr));
#endif // QT_CONFIG(statustip)
#if QT_CONFIG(shortcut)
  actionOpen->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+O", nullptr));
#endif // QT_CONFIG(shortcut)
  actionExit->setText(QCoreApplication::translate("MainWindow", "Exit", nullptr));
#if QT_CONFIG(statustip)
  actionExit->setStatusTip(QCoreApplication::translate("MainWindow", "Quit the emulator", nullptr));
#endif // QT_CONFIG(statustip)
  actionPause->setText(QCoreApplication::translate("MainWindow", "Pause", nullptr));
#if QT_CONFIG(statustip)
  actionPause->setStatusTip(QCoreApplication::translate("MainWindow", "Pause the emulation", nullptr));
#endif // QT_CONFIG(statustip)
  actionReset->setText(QCoreApplication::translate("MainWindow", "Reset", nullptr));
#if QT_CONFIG(statustip)
  actionReset->setStatusTip(QCoreApplication::translate("MainWindow", "Reset the emulation", nullptr));
#endif // QT_CONFIG(statustip)
  actionStop->setText(QCoreApplication::translate("MainWindow", "Stop", nullptr));
#if QT_CONFIG(statustip)
  actionStop->setStatusTip(QCoreApplication::translate("MainWindow", "Stop the emulation", nullptr));
#endif // QT_CONFIG(statustip)
  actionSettings->setText(QCoreApplication::translate("MainWindow", "Settings", nullptr));
#if QT_CONFIG(tooltip)
  actionSettings->setToolTip(QCoreApplication::translate("MainWindow", "Settings", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(statustip)
  actionSettings->setStatusTip(QCoreApplication::translate("MainWindow", "Open the settings window", nullptr));
#endif // QT_CONFIG(statustip)
  menuFile->setTitle(QCoreApplication::translate("MainWindow", "File", nullptr));
  menuEmulation->setTitle(QCoreApplication::translate("MainWindow", "Emulation", nullptr));
  menuAbout->setTitle(QCoreApplication::translate("MainWindow", "Help", nullptr));
} // retranslateUi

void MainWindow::ConnectSignalsToSlots() noexcept {
  connect(actionOpen, &QAction::triggered, this, [this]() {
    QString file_name = QFileDialog::getOpenFileName(
      this, "Nintendo 64 executable", QString(),
      "All supported types (*.zip *.ZIP *.7z *.7Z *.rar *.RAR *.tar *.TAR *.n64 *.N64 *.v64 *.V64 *.z64 *.Z64)");

    if (!file_name.isEmpty()) {
      emit OpenROM(file_name);
      vulkanWidget->show();
    }
  });

  connect(actionExit, &QAction::triggered, this, [this]() { emit Exit(); });

  connect(this, &MainWindow::destroyed, this, [this]() { emit Exit(); });

  connect(actionReset, &QAction::triggered, this, [this]() { emit Reset(); });

  connect(actionStop, &QAction::triggered, this, [this]() {
    vulkanWidget->hide();
    actionPause->setDisabled(true);
    actionReset->setDisabled(true);
    actionStop->setDisabled(true);
    emit Stop();
  });

  connect(actionPause, &QAction::triggered, this, [this]() {
    textPauseToggle = !textPauseToggle;
    actionPause->setText(textPauseToggle ? "Resume" : "Pause");
    emit Pause();
  });

  connect(actionAbout, &QAction::triggered, this, [this]() {
    QMessageBox::about(this, tr("About Kaizen"),
                       tr("Kaizen is a Nintendo 64 emulator that strives to offer a friendly user "
                          "experience and great compatibility.\n"
                          "Kaizen is licensed under the BSD 3-clause license.\n"
                          "Nintendo 64 is a registered trademarks of Nintendo Co., Ltd."));
  });

  connect(actionSettings, &QAction::triggered, this, [this]() { emit OpenSettings(); });
}
