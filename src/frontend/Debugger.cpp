#include <Debugger.hpp>
#include <QGuiApplication>

Debugger::Debugger() : QWidget(nullptr) {
  disassembly = new QDockWidget;
  disassembly->setWindowTitle("Disassembly");
  disassembly->setFeatures(QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable);
  codeView = new QTreeView(disassembly);
  codeView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  codeView->setHeaderHidden(true);
  cpuState = new QDockWidget;
  cpuState->setWindowTitle("Registers");
  cpuState->setFeatures(QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable);
  registers = new QTreeView(cpuState);
  registers->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

  horLayout = new QHBoxLayout;
  horLayout->addWidget(disassembly);
  horLayout->addWidget(cpuState);

  verLayout = new QVBoxLayout;
  verLayout->addLayout(horLayout);

  setLayout(verLayout);
}
