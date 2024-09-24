#include <Debugger.hpp>

Debugger::Debugger() : QWidget(nullptr) {
  disassembly->setWindowTitle("Disassembly");
  disassembly->setFeatures(QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable);
  codeView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  codeView->setHeaderHidden(true);
  codeView->setModel(codeModel.get());
  cpuState->setWindowTitle("Registers");
  cpuState->setFeatures(QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable);
  registers->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

  horLayout->addWidget(disassembly.get());
  horLayout->addWidget(cpuState.get());

  verLayout->addLayout(horLayout.get());

  setLayout(verLayout.get());

  // connect(codeView.get(), &QTreeView::activated, this, );
}
