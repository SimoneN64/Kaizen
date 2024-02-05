#pragma once
#include <QOpenGLExtraFunctions>
#include <QOpenGLWidget>
#include <QTimer>
#include <set>
#include <log.hpp>
#include <capstone/capstone.h>
#include <EmuThread.hpp>

class DebuggerWindow : public QOpenGLWidget, private QOpenGLExtraFunctions {
  std::set<u32> bkps{};
  void toggleBkp(u32);
  QTimer timer;
  csh disasmHandle;
  bool followPC = false;
  int scrollAmount = 0;
  void renderDisasm();
  void renderRegs();
  EmuThread* emuThread;
public:
  void wheelEvent(QWheelEvent*) override;
  DebuggerWindow(EmuThread*);
protected:
  void initializeGL() override;
  void paintGL() override;
};