#include <Frontend.hpp>
#include <QPainter>

namespace natsukashii::frontend {
QVulkanWindowRenderer *VulkanWindow::createRenderer() {
  return new VulkanRenderer(this);
}

VulkanRenderer::VulkanRenderer(QVulkanWindow *w)
  : m_window(w) { }

void VulkanRenderer::initResources() {
  m_devFuncs = m_window->vulkanInstance()->deviceFunctions(m_window->device());
}

void VulkanRenderer::startNextFrame() {
  VkClearColorValue clearColor = {{ 0.0f, 0.0f, 0.0f, 1.0f }};
  VkClearDepthStencilValue clearDS = { 1.0f, 0 };
  VkClearValue clearValues[2];
  memset(clearValues, 0, sizeof(clearValues));
  clearValues[0].color = clearColor;
  clearValues[1].depthStencil = clearDS;

  VkRenderPassBeginInfo rpBeginInfo;
  memset(&rpBeginInfo, 0, sizeof(rpBeginInfo));
  rpBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  rpBeginInfo.renderPass = m_window->defaultRenderPass();
  rpBeginInfo.framebuffer = m_window->currentFramebuffer();
  const QSize sz = m_window->swapChainImageSize();
  rpBeginInfo.renderArea.extent.width = sz.width();
  rpBeginInfo.renderArea.extent.height = sz.height();
  rpBeginInfo.clearValueCount = 2;
  rpBeginInfo.pClearValues = clearValues;
  VkCommandBuffer cmdBuf = m_window->currentCommandBuffer();
  m_devFuncs->vkCmdBeginRenderPass(cmdBuf, &rpBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

  // do thing

  m_devFuncs->vkCmdEndRenderPass(cmdBuf);

  m_window->frameReady();
  m_window->requestUpdate(); // render continuously, throttled by the presentation rate
}

OpenGLWindow::OpenGLWindow(QWindow *parent) : QWindow(parent) {
  setSurfaceType(QWindow::OpenGLSurface);
}

void OpenGLWindow::render(QPainter *painter) {
  Q_UNUSED(painter);
}

void OpenGLWindow::initialize() {}

void OpenGLWindow::render() {
  if (!m_device)
    m_device = new QOpenGLPaintDevice;

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

  m_device->setSize(size() * devicePixelRatio());
  m_device->setDevicePixelRatio(devicePixelRatio());

  QPainter painter(m_device);
  render(&painter);
}

bool OpenGLWindow::event(QEvent *event) {
  switch (event->type()) {
    case QEvent::UpdateRequest:
      renderNow();
      return true;
    default:
      return QWindow::event(event);
  }
}

void OpenGLWindow::exposeEvent(QExposeEvent *event) {
  Q_UNUSED(event);

  if (isExposed())
    renderNow();
}

void OpenGLWindow::renderNow() {
  if (!isExposed())
    return;

  bool needsInitialize = false;

  if (!m_context) {
    m_context = new QOpenGLContext(this);
    m_context->setFormat(requestedFormat());
    m_context->create();

    needsInitialize = true;
  }

  m_context->makeCurrent(this);

  if (needsInitialize) {
    initializeOpenGLFunctions();
    initialize();
  }

  render();

  m_context->swapBuffers(this);
}


}
