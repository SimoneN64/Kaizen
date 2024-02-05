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

DebuggerWindow::DebuggerWindow(EmuThread* emuThread) : emuThread(emuThread), QOpenGLWidget(nullptr) {
  emuThread->bkps = &bkps;

  if (cs_open(CS_ARCH_MIPS, cs_mode(CS_MODE_BIG_ENDIAN | CS_MODE_MIPS64), &disasmHandle) != CS_ERR_OK) {
    Util::panic("Could not initialize capstone!");
  }

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
  ImGui::SameLine();
  ImGui::Checkbox("Follow PC", &followPC);
  ImGui::BeginChild("Disassembly", ImVec2(600,0), 0, ImGuiWindowFlags_NoScrollbar);
  renderDisasm();
  ImGui::EndChild();
  ImGui::SameLine();
  ImGui::BeginChild("Registers", ImVec2(360,0));
  renderRegs();
  ImGui::EndChild();
  ImGui::End();
  ImGui::PopStyleVar();

  // Do render before ImGui UI is rendered
  glViewport(0, 0, width(), height());
  glClearColor(0, 0, 0, 255);
  glClear(GL_COLOR_BUFFER_BIT);

  ImGui::Render();
  QtImGui::render();
}

void DebuggerWindow::toggleBkp(u32 addr) {
  auto pos = bkps.find(addr);
  if (pos != bkps.end()) {
    bkps.erase(pos);
  } else {
    bkps.insert(addr);
  }
}