#pragma once
#include <QOpenGLExtraFunctions>
#include <QOpenGLWidget>
#include <QTimer>
#include <QtImGui.h>
#include <set>
#include <log.hpp>
#include <capstone/capstone.h>
#include <EmuThread.hpp>
#include <imgui.h>
#include <RSPDebugger.hpp>

class DebuggerWindow : public QOpenGLWidget, private QOpenGLExtraFunctions {
  QTimer timer;
  csh disasmHandle;
  bool followPC = false;
  u64 scrollAmount = 0;
  void renderDisasm();
  void renderRegs();
  void renderCPU();
  EmuThread* emuThread;
  ImU32 hover_col;
  ImU32 bkp_col;
  ImVec4 instr_imm_col;
  ImVec4 instr_mnemonic_col;
  ImVec4 instr_regs_col;
  RSPDebugger rspDebugger;
public:
  void wheelEvent(QWheelEvent*) override;
  DebuggerWindow(EmuThread*);
protected:
  void initializeGL() override;
  void paintGL() override;
};