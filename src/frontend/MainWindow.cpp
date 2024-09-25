#include <MainWindow.hpp>

MainWindow::MainWindow(const std::shared_ptr<n64::Core> &core) noexcept {
  if (objectName().isEmpty())
    setObjectName("MainWindow");
  resize(800, 646);
  actionOpenDebuggerWindow = std::make_unique<QAction>(this);
  actionOpenDebuggerWindow->setObjectName("actionOpenDebuggerWindow");
  actionAbout = std::make_unique<QAction>(this);
  actionAbout->setObjectName("actionAbout");
  actionOpen = std::make_unique<QAction>(this);
  actionOpen->setObjectName("actionOpen");
  actionExit = std::make_unique<QAction>(this);
  actionExit->setObjectName("actionExit");
  actionPause = std::make_unique<QAction>(this);
  actionPause->setObjectName("actionPause");
  actionReset = std::make_unique<QAction>(this);
  actionReset->setObjectName("actionReset");
  actionStop = std::make_unique<QAction>(this);
  actionStop->setObjectName("actionStop");
  actionSettings = std::make_unique<QAction>(this);
  actionSettings->setObjectName("actionSettings");
  centralwidget = std::make_unique<QWidget>(this);
  centralwidget->setObjectName("centralwidget");
  verticalLayout = std::make_unique<QVBoxLayout>();
  verticalLayout->setSpacing(0);
  verticalLayout->setObjectName("verticalLayout");
  verticalLayout->setContentsMargins(0, 0, 0, 0);
  vulkanWidget = std::make_unique<RenderWidget>(core);
  vulkanWidget->setObjectName("vulkanWidget");

  verticalLayout->addWidget(vulkanWidget.get());

  centralwidget->setLayout(verticalLayout.get());

  setCentralWidget(centralwidget.get());
  menubar = std::make_unique<QMenuBar>(this);
  menubar->setObjectName("menubar");
  menubar->setGeometry(QRect(0, 0, 800, 22));
  menuFile = std::make_unique<QMenu>(menubar.get());
  menuFile->setObjectName("menuFile");
  menuEmulation = std::make_unique<QMenu>(menubar.get());
  menuEmulation->setObjectName("menuEmulation");
  menuTools = std::make_unique<QMenu>(menubar.get());
  menuTools->setObjectName("menuTools");
  menuAbout = std::make_unique<QMenu>(menubar.get());
  menuAbout->setObjectName("menuAbout");
  setMenuBar(menubar.get());
  statusbar = std::make_unique<QStatusBar>(this);
  statusbar->setObjectName("statusbar");
  setStatusBar(statusbar.get());

  menubar->addAction(menuFile->menuAction());
  menubar->addAction(menuEmulation->menuAction());
  menubar->addAction(menuTools->menuAction());
  menubar->addAction(menuAbout->menuAction());
  menuFile->addAction(actionOpen.get());
  menuFile->addAction(actionExit.get());
  menuEmulation->addAction(actionSettings.get());
  menuEmulation->addAction(actionPause.get());
  menuEmulation->addAction(actionReset.get());
  menuEmulation->addAction(actionStop.get());
  menuTools->addAction(actionOpenDebuggerWindow.get());
  menuAbout->addAction(actionAbout.get());

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
  actionOpenDebuggerWindow->setText(QCoreApplication::translate("MainWindow", "CPU Debugger", nullptr));
  actionOpenDebuggerWindow->setStatusTip(QCoreApplication::translate(
    "MainWindow", "Open the CPU debugger window which allows you see registers, memory and disassembled code",
    nullptr));
  actionAbout->setText(QCoreApplication::translate("MainWindow", "About Kaizen", nullptr));
  actionAbout->setStatusTip(QCoreApplication::translate("MainWindow", "About this emulator", nullptr));
  actionOpen->setText(QCoreApplication::translate("MainWindow", "Open...", nullptr));
  actionOpen->setStatusTip(QCoreApplication::translate("MainWindow", "Open a ROM", nullptr));
  actionOpen->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+O", nullptr));
  actionExit->setText(QCoreApplication::translate("MainWindow", "Exit", nullptr));
  actionExit->setStatusTip(QCoreApplication::translate("MainWindow", "Quit the emulator", nullptr));
  actionPause->setText(QCoreApplication::translate("MainWindow", "Pause", nullptr));
  actionPause->setStatusTip(QCoreApplication::translate("MainWindow", "Pause the emulation", nullptr));
  actionReset->setText(QCoreApplication::translate("MainWindow", "Reset", nullptr));
  actionReset->setStatusTip(QCoreApplication::translate("MainWindow", "Reset the emulation", nullptr));
  actionStop->setText(QCoreApplication::translate("MainWindow", "Stop", nullptr));
  actionStop->setStatusTip(QCoreApplication::translate("MainWindow", "Stop the emulation", nullptr));
  actionSettings->setText(QCoreApplication::translate("MainWindow", "Settings", nullptr));
  actionSettings->setToolTip(QCoreApplication::translate("MainWindow", "Settings", nullptr));
  actionSettings->setStatusTip(QCoreApplication::translate("MainWindow", "Open the settings window", nullptr));
  menuFile->setTitle(QCoreApplication::translate("MainWindow", "File", nullptr));
  menuEmulation->setTitle(QCoreApplication::translate("MainWindow", "Emulation", nullptr));
  menuTools->setTitle(QCoreApplication::translate("MainWindow", "Tools", nullptr));
  menuAbout->setTitle(QCoreApplication::translate("MainWindow", "Help", nullptr));
} // retranslateUi

void MainWindow::ConnectSignalsToSlots() noexcept {
  connect(actionOpen.get(), &QAction::triggered, this, [this]() {
    QString file_name = QFileDialog::getOpenFileName(
      this, "Nintendo 64 executable", QString(),
      "All supported types (*.zip *.ZIP *.7z *.7Z *.rar *.RAR *.tar *.TAR *.n64 *.N64 *.v64 *.V64 *.z64 *.Z64)");

    if (!file_name.isEmpty()) {
      emit OpenROM(file_name);
      vulkanWidget->show();
    }
  });

  connect(actionExit.get(), &QAction::triggered, this, [this]() { emit Exit(); });

  connect(this, &MainWindow::destroyed, this, [this]() { emit Exit(); });

  connect(actionReset.get(), &QAction::triggered, this, [this]() { emit Reset(); });

  connect(actionStop.get(), &QAction::triggered, this, [this]() {
    vulkanWidget->hide();
    actionPause->setDisabled(true);
    actionReset->setDisabled(true);
    actionStop->setDisabled(true);
    emit Stop();
  });

  connect(actionPause.get(), &QAction::triggered, this, [this]() {
    textPauseToggle = !textPauseToggle;
    actionPause->setText(textPauseToggle ? "Resume" : "Pause");
    emit Pause();
  });

  connect(actionAbout.get(), &QAction::triggered, this, [this]() {
    QMessageBox::about(this, tr("About Kaizen"),
                       tr("Kaizen is a Nintendo 64 emulator that strives to offer a friendly user "
                          "experience and great compatibility.\n"
                          "Kaizen is licensed under the BSD 3-clause license.\n"
                          "Nintendo 64 is a registered trademarks of Nintendo Co., Ltd."));
  });

  connect(actionSettings.get(), &QAction::triggered, this, [this]() { emit OpenSettings(); });
  connect(actionOpenDebuggerWindow.get(), &QAction::triggered, this, [this]() { emit OpenDebugger(); });
}
