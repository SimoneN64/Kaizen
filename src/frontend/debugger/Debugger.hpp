#pragma once
#include <QWidget>
#include <QVBoxLayout>
#include <QPlainTextEdit>
#include <vector>
#include <log.hpp>

class DebuggerWindow : public QWidget {
  QTextEdit* disasm, *addressBox;
  QVBoxLayout* disasmLayout, *regsLayout;
  QVBoxLayout* mainLayout;
  std::vector<u32> bkps{};
  void toggleBkp(u32);
public:
  bool eventFilter(QObject*, QEvent*);
  DebuggerWindow();
};