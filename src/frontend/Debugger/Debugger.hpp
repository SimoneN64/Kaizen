#pragma once
#include <QWidget>
#include <QVBoxLayout>
#include <QPlainTextEdit>

class DebuggerWindow : public QWidget {
  QPlainTextEdit* disasm;
  QVBoxLayout* disasmLayout, *regsLayout;
  QHBoxLayout* mainLayout;
public:
  DebuggerWindow();
};