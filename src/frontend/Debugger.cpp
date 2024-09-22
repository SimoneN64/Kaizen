#include <Debugger.hpp>
#include <QGuiApplication>

Debugger::Debugger() : QWidget(nullptr) {
  disassembly = new QDockWidget(this);
  disassembly->setWindowTitle("Disassembly");
  disassembly->setFeatures(QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable);
  codeView = new QTreeView(disassembly);
  codeView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  codeView->setHeaderHidden(true);
  cpuState = new QDockWidget(this);
  cpuState->setWindowTitle("Registers");
  cpuState->setFeatures(QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable);
  registers = new QTreeView(cpuState);
  registers->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}
