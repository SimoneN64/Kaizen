#pragma once
#include <QOpenGLExtraFunctions>
#include <QOpenGLWidget>
#include <QTimer>
#include <vector>
#include <log.hpp>

class DebuggerWindow : public QOpenGLWidget, private QOpenGLExtraFunctions {
  std::vector<u32> bkps{};
  void toggleBkp(u32);
  QTimer timer;
  void renderDisasm();
  void renderRegs();
public:
  DebuggerWindow();
protected:
  void initializeGL() override;
  void paintGL() override;
};