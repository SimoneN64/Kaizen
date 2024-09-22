#pragma once
#include <QDockWidget>
#include <QWidget>
#include <QTreeView>

class Debugger : public QWidget {
  QDockWidget *disassembly{}, *cpuState{};
  QTreeView *codeView{}, *registers{};

public:
  Debugger();
};
