#include <Debugger.hpp>

DebuggerWindow::DebuggerWindow() : QWidget(nullptr) {
  if (objectName().isEmpty())
    setObjectName("Debugger");

  resize(500, 400);
  setWindowTitle("Debugger");
}