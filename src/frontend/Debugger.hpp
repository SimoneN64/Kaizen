#pragma once
#include <QDockWidget>
#include <QWidget>
#include <QTreeView>
#include <QHBoxLayout>
#include <QVBoxLayout>

class Debugger : public QWidget {
  QDockWidget *disassembly{}, *cpuState{};
  QTreeView *codeView{}, *registers{};
  QHBoxLayout *horLayout{};
  QVBoxLayout *verLayout{};

public:
  Debugger();
};
