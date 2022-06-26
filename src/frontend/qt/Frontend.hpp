#pragma once
#include <QVulkanWindow>
#include <QVulkanWindowRenderer>
#include <QVulkanDeviceFunctions>
#include <QOpenGLWindow>
#include <QOpenGLFunctions>
#include <QOpenGLPaintDevice>

namespace natsukashii::frontend {
class VulkanRenderer : public QVulkanWindowRenderer {
public:
  VulkanRenderer(QVulkanWindow *w);

  void initResources() override;
  void initSwapChainResources() override;
  void releaseSwapChainResources() override;
  void releaseResources() override;

  void startNextFrame() override;

private:
  QVulkanWindow *m_window;
  QVulkanDeviceFunctions *m_devFuncs;
};

class VulkanWindow : public QVulkanWindow {
public:
  QVulkanWindowRenderer *createRenderer() override;
};

class OpenGLWindow : protected QOpenGLFunctions, public QWindow {
  Q_OBJECT
public:
  explicit OpenGLWindow(QWindow *parent = nullptr);
  ~OpenGLWindow();

  void render(QPainter *painter);
  void render();

  void initialize();

  void setAnimating(bool animating);

public slots:
  void renderNow();

protected:
  bool event(QEvent *event) override;
  void exposeEvent(QExposeEvent *event) override;

private:
  QOpenGLContext *m_context = nullptr;
  QOpenGLPaintDevice *m_device = nullptr;
};

class Window : public QWindow {
public:
  Window() {}
private:
  VulkanWindow vulkanWindow;
  OpenGLWindow openGlWindow;
};
}
