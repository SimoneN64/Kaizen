#include <Debugger.hpp>
#include <QtImGui.h>
#include <imgui.h>
#include <imgui_stdlib.h>
#include <QGuiApplication>
#include <QStyleHints>
#include <QWheelEvent>

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
    instr_hex_col = ImVec4{0.878, 0.875, 0.584, 1};
    instr_mnemonic_col = ImVec4{0.6, 0.929, 0.847, 1};
    instr_ops_col = ImVec4{0.788, 0.6, 0.929, 1};
    bkp_col = IM_COL32(245, 217, 78, 80);
    hover_col = IM_COL32(245, 78, 78, 80);
  } else {
    ImGui::StyleColorsLight();
    instr_hex_col = ImVec4{0.702, 0.694, 0.365, 1};
    instr_mnemonic_col = ImVec4{0.365, 0.702, 0.616, 1};
    instr_ops_col = ImVec4{0.498, 0.302, 0.651, 1};
    bkp_col = IM_COL32(168, 147, 40, 255);
    hover_col = IM_COL32(173, 35, 35, 255);
  }
}

void DebuggerWindow::wheelEvent(QWheelEvent* e) {
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

void DebuggerWindow::renderDisasm() {
  auto areaAvail = ImGui::GetContentRegionAvail();
  auto lineHeight = ImGui::GetTextLineHeightWithSpacing();
  auto lineCount = areaAvail.y / lineHeight;
  auto drawList = ImGui::GetWindowDrawList();

  emuThread->core->pause = true;
  if(followPC) {
    scrollAmount = emuThread->core->cpu->regs.pc;
  }
  for(int i = 0; i < lineCount; i++) {
    u32 pc = scrollAmount + (i * 4);

    auto pos = ImGui::GetCursorPos();
    pos.y -= 4;
    auto end = ImVec2{pos.x + areaAvail.x, pos.y + lineHeight - 2};

    if (ImGui::IsWindowHovered()) { // if mouse inside this frame
      auto mousePosY = ImGui::GetMousePos().y;

      if (std::trunc(mousePosY / lineHeight) == i) { // is hovering this line
        if(ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
          emuThread->core->toggleBkp(Breakpoint{pc-8, false});
        }
        drawList->AddRectFilled(pos, end, hover_col);
      }
    }

    for(auto bkp : emuThread->core->bkps) {
      if(pc-8 == bkp.addr && !bkp.ghost) {
        drawList->AddRectFilled(pos, end, bkp_col);
      }
    }

    pos.y = ImGui::GetCursorScreenPos().y;
    end = ImVec2{pos.x + areaAvail.x, pos.y + lineHeight - 2};

    // program counter is teal
    if(pc == (u32)emuThread->core->cpu->regs.pc)
      drawList->AddRectFilled(pos, end, IM_COL32(0, 128, 128, 80));

    auto instr = emuThread->core->cpu->mem.ReadDebugger<u32>(emuThread->core->cpu->regs, pc);
    instr = bswap_32(instr);

    cs_insn *insn;
    size_t count = cs_disasm(disasmHandle, reinterpret_cast<u8*>(&instr), 4, pc, 0, &insn);
    if(count > 0) {
      for(size_t j = 0; j < count; j++) {
        ImGui::Text("%08X", insn[j].address);
        ImGui::SameLine();
        ImGui::TextColored(instr_hex_col, "%08X", instr);
        ImGui::SameLine();
        ImGui::TextColored(instr_mnemonic_col, "%s", insn[j].mnemonic);
        ImGui::SameLine();
        ImGui::TextColored(instr_ops_col, "%s", insn[j].op_str);
      }

      cs_free(insn, count);
    } else {
      ImGui::Text("%08X", pc);
      ImGui::SameLine();
      ImGui::TextColored(instr_hex_col, "%08X", instr);
      ImGui::SameLine();
      ImGui::TextColored(instr_mnemonic_col, "invalid");
    }
  }

  emuThread->core->pause = false;
}

void DebuggerWindow::renderRegs() {
  emuThread->core->pause = true;
  n64::Registers& regs = emuThread->core->cpu->regs;
  for(int i = 0; i < 32; i += 2) {
    ImGui::Text("%s", regNames[i].c_str());
    ImGui::SameLine();
    ImGui::Text("%s", fmt::format("{:016X}", (u64)regs.gpr[i]).c_str());
    ImGui::SameLine();
    ImGui::Text("%s", regNames[i+1].c_str());
    ImGui::SameLine();
    ImGui::Text("%s", fmt::format("{:016X}", (u64)regs.gpr[i+1]).c_str());
  }
  emuThread->core->pause = false;
}

void DebuggerWindow::paintGL() {
  static std::string goToAddrBuf{"00000000"};
  static u32 goToAddr=0;
  QtImGui::newFrame();

  grabKeyboard();

  ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
  ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
  ImGui::Begin("##debugger", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize);
  if(ImGui::Button("Continue")) {
    emuThread->core->Step();
    emuThread->core->broken = false;
  }
  ImGui::SameLine();
  if(ImGui::Button("Step over")) {
    emuThread->core->Step();
    emuThread->core->broken = false;
  }
  ImGui::SameLine();
  if(ImGui::Button("Step in")) {
    emuThread->core->Step();
  }
  ImGui::SameLine();
  if(ImGui::Button("Step out")) {
    emuThread->core->Step();
    emuThread->core->broken = false;
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

  releaseKeyboard();

  ImGui::Render();
  QtImGui::render();
}