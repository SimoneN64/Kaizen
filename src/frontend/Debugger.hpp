#pragma once
#include <QDockWidget>
#include <QWidget>
#include <QTreeView>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <CodeModel.hpp>

class Debugger : public QWidget {
  std::unique_ptr<QDockWidget> disassembly = std::make_unique<QDockWidget>(),
                               cpuState = std::make_unique<QDockWidget>();
  std::unique_ptr<QTreeView> codeView = std::make_unique<QTreeView>(disassembly.get()),
                             registers = std::make_unique<QTreeView>(cpuState.get());
  std::unique_ptr<QHBoxLayout> horLayout = std::make_unique<QHBoxLayout>();
  std::unique_ptr<QVBoxLayout> verLayout = std::make_unique<QVBoxLayout>();
  std::unique_ptr<CodeModel> codeModel = std::make_unique<CodeModel>();

public:
  Debugger();
};
