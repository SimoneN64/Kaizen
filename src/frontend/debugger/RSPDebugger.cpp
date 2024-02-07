#include <RSPDebugger.hpp>
#include <imgui.h>
#include <imgui_stdlib.h>
#include <QGuiApplication>
#include <QStyleHints>
#include <QWheelEvent>
#include <log.hpp>

RSPDebugger::RSPDebugger(EmuThread* emuThread) : emuThread(emuThread) {
  if (cs_open(CS_ARCH_MIPS, cs_mode(CS_MODE_BIG_ENDIAN | CS_MODE_MIPS32), &handle) != CS_ERR_OK) {
    Util::panic("Could not initialize capstone for RSP!");
  }

  if (cs_option(handle, CS_OPT_DETAIL, CS_OPT_ON) != CS_ERR_OK) {
    Util::panic("Could not enable capstone detail for RSP!");
  }

  if (objectName().isEmpty())
    setObjectName("RSP Debugger");

  setFixedSize(960, 600);
  setWindowTitle("RSP Debugger");
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

void RSPDebugger::initializeGL() {
  initializeOpenGLFunctions();
  QtImGui::initialize(this, false);
  
  if(QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark) {
    instr_imm_col = ImVec4{0.878, 0.875, 0.584, 1};
    instr_mnemonic_col = ImVec4{0.6, 0.929, 0.847, 1};
    instr_regs_col = ImVec4{0.788, 0.6, 0.929, 1};
    bkp_col = IM_COL32(245, 217, 78, 80);
    hover_col = IM_COL32(245, 78, 78, 80);
  } else {
    instr_imm_col = ImVec4{0.702, 0.694, 0.365, 1};
    instr_mnemonic_col = ImVec4{0.365, 0.702, 0.616, 1};
    instr_regs_col = ImVec4{0.498, 0.302, 0.651, 1};
    bkp_col = IM_COL32(168, 147, 40, 255);
    hover_col = IM_COL32(173, 35, 35, 255);
  }
}

void RSPDebugger::wheelEvent(QWheelEvent* e) {
  if (!e->angleDelta().isNull()) {
    if (e->angleDelta().y() < 0) {
      scrollAmount += 4;
    } else if(e->angleDelta().y() > 0) {
      scrollAmount -= 4;
    }
  }

  followPC = false;

  e->accept();
}

void RSPDebugger::paintGL() {
  static std::string goToAddrBuf{"0000"};
  static u32 goToAddr=0;
  QtImGui::newFrame(ref);

  grabKeyboard();

  ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
  ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
  ImGui::Begin("##rspdebugger", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize);
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
  ImGui::SameLine();
  ImGui::SetNextItemWidth(100.f);
  ImGui::InputText("Go to address", &goToAddrBuf, ImGuiInputTextFlags_CharsHexadecimal);
  ImGui::SameLine();
  if(ImGui::Button("Go")) {
    goToAddr = std::stoull(goToAddrBuf, nullptr, 16);
    goToAddr &= ~3;
    scrollAmount = goToAddr;
  }
  ImGui::BeginChild("Disassembly", ImVec2(600,0), 0, ImGuiWindowFlags_NoScrollbar);
  //renderDisasm();
  ImGui::EndChild();
  ImGui::SameLine();
  ImGui::BeginChild("Registers", ImVec2(360,0));
  //renderRegs();
  ImGui::EndChild();
  ImGui::End();
  ImGui::PopStyleVar();

  // Do render before ImGui UI is rendered
  glViewport(0, 0, width(), height());
  glClearColor(0, 0, 0, 255);
  glClear(GL_COLOR_BUFFER_BIT);

  releaseKeyboard();

  ImGui::Render();
  QtImGui::render(ref);
}