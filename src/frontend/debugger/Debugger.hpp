#pragma once
#include <QWidget>
#include <QVBoxLayout>
#include <QPlainTextEdit>

class DebuggerWindow : public QWidget {
  QTextEdit* disasm, *addressBox;
  QVBoxLayout* disasmLayout, *regsLayout;
  QVBoxLayout* mainLayout;
public:
  bool eventFilter(QObject*, QEvent*);
  DebuggerWindow();
};