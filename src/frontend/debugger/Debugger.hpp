#pragma once
#include <QOpenGLExtraFunctions>
#include <QOpenGLWidget>
#include <QTimer>
#include <set>
#include <log.hpp>
#include <capstone/capstone.h>
#include <EmuThread.hpp>
#include <imgui.h>

class DebuggerWindow : public QOpenGLWidget, private QOpenGLExtraFunctions {
  std::set<u32> bkps{};
  void toggleBkp(u32);
  QTimer timer;
  csh disasmHandle;
  bool followPC = false;
  u64 scrollAmount = 0;
  void renderDisasm();
  void renderRegs();
  EmuThread* emuThread;
  ImU32 hover_col;
  ImU32 bkp_col;
  ImVec4 instr_hex_col;
  ImVec4 instr_mnemonic_col;
  ImVec4 instr_ops_col;
public:
  void wheelEvent(QWheelEvent*) override;
  DebuggerWindow(EmuThread*);
protected:
  void initializeGL() override;
  void paintGL() override;
};