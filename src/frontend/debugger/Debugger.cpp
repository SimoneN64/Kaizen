#include <Debugger.hpp>
#include <QtImGui.h>
#include <imgui.h>
#include <QGuiApplication>
#include <QStyleHints>

const std::string regNames[] = {
  "r0", "at", "v0", "v1",
  "a0", "a1", "a2", "a3",
  "t0", "t1", "t2", "t3",
  "t4", "t5", "t6", "t7",
  "s0", "s1", "s2", "s3",
  "s4", "s5", "s6", "s7",
  "t8", "t9", "k0", "k1",
  "gp", "sp", "s8", "ra",
};

DebuggerWindow::DebuggerWindow() : QOpenGLWidget(nullptr) {
  if (objectName().isEmpty())
    setObjectName("Debugger");

  setFixedSize(960, 600);
  setWindowTitle("Debugger");
  QSurfaceFormat glFormat;
  if (QOpenGLContext::openGLModuleType() == QOpenGLContext::LibGL)
  {
    glFormat.setVersion(3, 3);
    glFormat.setProfile(QSurfaceFormat::CoreProfile);
  }
  QSurfaceFormat::setDefaultFormat(glFormat);

  QObject::connect(&timer, SIGNAL(timeout()), this, SLOT(update()));
  timer.start(16);
}

void DebuggerWindow::initializeGL() {
  initializeOpenGLFunctions();
  QtImGui::initialize(this);
  if(QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark) {
    ImGui::StyleColorsDark();
  } else {
    ImGui::StyleColorsLight();
  }
}

void DebuggerWindow::renderDisasm() {

}

void DebuggerWindow::renderRegs() {
  for(int i = 0; i < 32; i += 2) {
    ImGui::Text("%s", regNames[i].c_str());
    ImGui::SameLine();
    ImGui::Text("%s", fmt::format("{:016X}", 0).c_str());
    ImGui::SameLine();
    ImGui::Text("%s", regNames[i+1].c_str());
    ImGui::SameLine();
    ImGui::Text("%s", fmt::format("{:016X}", 0).c_str());
  }
}

void DebuggerWindow::paintGL() {
  QtImGui::newFrame();

  ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
  ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
  ImGui::Begin("##debugger", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize);
  if(ImGui::Button("Continue")) {

  }
  ImGui::SameLine();
  if(ImGui::Button("Step over")) {

  }
  ImGui::SameLine();
  if(ImGui::Button("Step in")) {

  }
  ImGui::SameLine();
  if(ImGui::Button("Step out")) {

  }
  ImGui::BeginChild("Disassembly", ImVec2(0,0), ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AlwaysAutoResize);
  renderDisasm();
  ImGui::EndChild();
  ImGui::SameLine();
  ImGui::BeginChild("Registers", ImVec2(0,0), ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AlwaysAutoResize);
  renderRegs();
  ImGui::EndChild();
  ImGui::End();
  ImGui::PopStyleVar();

  // Do render before ImGui UI is rendered
  glViewport(0, 0, width(), height());
  glClearColor(0,0,0,255);
  glClear(GL_COLOR_BUFFER_BIT);

  ImGui::Render();
  QtImGui::render();
}

void DebuggerWindow::toggleBkp(u32 addr) {
  auto pos = std::find(bkps.begin(), bkps.end(), addr);
  if (pos == bkps.end()) {
    bkps.push_back(addr);
  }
  else {
    bkps.erase(pos);
  }
}