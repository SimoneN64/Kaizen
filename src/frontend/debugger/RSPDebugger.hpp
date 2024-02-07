#pragma once
#include <QOpenGLExtraFunctions>
#include <QOpenGLWidget>
#include <QTimer>
#include <EmuThread.hpp>
#include <QtImGui.h>
#include <capstone/capstone.h>
#include <imgui.h>
#include <set>

class RSPDebugger : public QOpenGLWidget, private QOpenGLExtraFunctions {
  QTimer timer;
  csh handle;
  bool followPC = false;
  u64 scrollAmount = 0;
  void renderDisasm();
  void renderRegs();
  EmuThread* emuThread;
  ImU32 hover_col;
  ImU32 bkp_col;
  ImVec4 instr_imm_col;
  ImVec4 instr_mnemonic_col;
  ImVec4 instr_regs_col;
public:
  QtImGui::RenderRef ref = nullptr;
  void wheelEvent(QWheelEvent*) override;
  RSPDebugger(EmuThread*);
protected:
  void initializeGL() override;
  void paintGL() override;
};