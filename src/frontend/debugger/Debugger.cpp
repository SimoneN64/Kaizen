#include <Debugger.hpp>
#include <imgui.h>
#include <imgui_stdlib.h>
#include <QGuiApplication>
#include <QStyleHints>
#include <QWheelEvent>
#include <CpuDefinitions.hpp>

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
    Util::panic("Could not initialize capstone for main CPU!");
  }

  if (cs_option(disasmHandle, CS_OPT_DETAIL, CS_OPT_ON) != CS_ERR_OK) {
    Util::panic("Could not enable capstone detail for main CPU!");
  }

  if (cs_open(CS_ARCH_MIPS, cs_mode(CS_MODE_BIG_ENDIAN | CS_MODE_MIPS32), &rspHandle) != CS_ERR_OK) {
    Util::panic("Could not initialize capstone for main CPU!");
  }

  if (cs_option(rspHandle, CS_OPT_DETAIL, CS_OPT_ON) != CS_ERR_OK) {
    Util::panic("Could not enable capstone detail for main CPU!");
  }

  if (objectName().isEmpty())
    setObjectName("Debugger");

  resize(1024, 768);
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
    instr_imm_col = ImVec4{0.878, 0.875, 0.584, 1};
    instr_mnemonic_col = ImVec4{0.6, 0.929, 0.847, 1};
    instr_regs_col = ImVec4{0.788, 0.6, 0.929, 1};
    bkp_col = IM_COL32(245, 217, 78, 80);
    hover_col = IM_COL32(245, 78, 78, 80);
  } else {
    ImGui::StyleColorsLight();
    instr_imm_col = ImVec4{0.702, 0.694, 0.365, 1};
    instr_mnemonic_col = ImVec4{0.365, 0.702, 0.616, 1};
    instr_regs_col = ImVec4{0.498, 0.302, 0.651, 1};
    bkp_col = IM_COL32(168, 147, 40, 255);
    hover_col = IM_COL32(173, 35, 35, 255);
  }
}

void DebuggerWindow::wheelEvent(QWheelEvent* e) {
  if (!e->angleDelta().isNull()) {
    if (e->angleDelta().y() < 0) {
      if (isRSPfocused) {
        scrollAmountRSP += 4;
      } else {
        scrollAmount += 4;
      }
    } else if(e->angleDelta().y() > 0) {
      if (isRSPfocused) {
        scrollAmountRSP -= 4;
      } else {
        scrollAmount -= 4;
      }
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
    auto end = ImVec2{pos.x + areaAvail.x, pos.y + lineHeight - 2};

    if (ImGui::IsWindowHovered()) { // if mouse inside this frame
      auto mousePosY = ImGui::GetMousePos().y;

      if (std::trunc(mousePosY / lineHeight) == i) { // is hovering this line
        if(ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
          emuThread->core->toggleBkp(Breakpoint{pc - 8, false});
        }
        drawList->AddRectFilled(pos, end, hover_col);
      }
    }

    for(auto bkp : emuThread->core->bkps) {
      if(pc - 8 == bkp.addr && !bkp.ghost) {
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
        cs_insn *in = &insn[j];
        ImGui::Text("%08X", in->address);
        ImGui::SameLine();
        ImGui::TextColored(instr_imm_col, "%08X", instr);
        ImGui::SameLine();
        ImGui::TextColored(instr_mnemonic_col, "%s", in->mnemonic);
        if(in->detail->mips.op_count > 0) {
          ImGui::SameLine();
          for(int x = 0; x < in->detail->mips.op_count; x++) {
            cs_mips_op* op = &in->detail->mips.operands[x];

            auto printReg = [this](mips_reg reg) {
              if(reg == MIPS_REG_PC) {
                ImGui::TextColored(instr_regs_col, "pc");
              } else if(reg == MIPS_REG_LO) {
                ImGui::TextColored(instr_regs_col, "lo");
              } else if(reg == MIPS_REG_HI) {
                ImGui::TextColored(instr_regs_col, "hi");
              } else {
                reg = static_cast<mips_reg>(static_cast<int>(reg - 2));
                if(reg >= 0 && reg <= 31) {
                  ImGui::TextColored(instr_regs_col, "%s", regNames[reg].c_str());
                } else if(reg >= 55 && reg <= 86) {
                  ImGui::TextColored(instr_regs_col, "f%d", static_cast<int>(reg) - 55);
                } else {
                  ImGui::TextColored(instr_regs_col, "unk");
                }
              }
            };

            switch(op->type) {
              case MIPS_OP_REG:
                printReg(op->reg);
                break;
              case MIPS_OP_IMM:
                ImGui::TextColored(instr_imm_col, "#%X", (u32)op->imm);
                break;
              case MIPS_OP_MEM:
                ImGui::TextColored(instr_imm_col, "%X", (u32)op->mem.disp);
                ImGui::SameLine(0, 0);
                ImGui::TextUnformatted("(");
                ImGui::SameLine(0, 0);
                printReg(op->mem.base);
                ImGui::SameLine(0, 0);
                ImGui::TextUnformatted(")");
              break;
              default:
                break;
            }

            if(x < (in->detail->mips.op_count-1)) {
              ImGui::SameLine(0, 0);
              ImGui::TextUnformatted(", ");
              ImGui::SameLine(0, 0);
            }
          }
        }        
      }

      cs_free(insn, count);
    } else {
      ImGui::Text("%08X", pc);
      ImGui::SameLine();
      ImGui::TextColored(instr_imm_col, "%08X", instr);
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

RSP_Instruction DebuggerWindow::disassembleRSP(u32 address, u32 instr) {
  switch (instr) {
    case n64::SPECIAL: {
      cs_insn* insn;
      size_t count = cs_disasm(rspHandle, reinterpret_cast<u8*>(&instr), 4, address, 0, &insn);
      if(count > 0) {
        std::vector<RSP_Operand> operands;
        for(int i = 0; i < insn->detail->mips.op_count; i++) {
          cs_mips_op mips_op = insn->detail->mips.operands[i];
          RSP_Operand op;
          op.type = mips_op.type;
          op.reg.idx = op.type == MIPS_OP_REG ? static_cast<eRSP_Reg>(mips_op.reg - 1) : RSP_INVALID;
          op.imm = op.type == MIPS_OP_IMM ? mips_op.imm : 0;
          op.mem.base.idx = op.type == MIPS_OP_MEM ? static_cast<eRSP_Reg>(mips_op.mem.base - 1) : RSP_INVALID;
          op.mem.base.disp = op.type == MIPS_OP_MEM ? mips_op.mem.disp : -1;
          operands.push_back(op);
        }
        return { address, operands, insn->mnemonic };
      }
    }
    default: return { address, {}, "unk" };
  }
}

void DebuggerWindow::renderDisasmRSP() {
  auto areaAvail = ImGui::GetContentRegionAvail();
  auto lineHeight = ImGui::GetTextLineHeightWithSpacing();
  auto lineCount = areaAvail.y / lineHeight;
  auto drawList = ImGui::GetWindowDrawList();

  n64::Mem& mem = emuThread->core->cpu->mem;
  n64::RSP& rsp = mem.mmio.rsp;

  emuThread->core->pause = true;
  if(followPC) {
    scrollAmountRSP = rsp.pc;
  }
  for(int i = 0; i < lineCount; i++) {
    u16 pc = scrollAmountRSP + (i * 4);

    auto pos = ImGui::GetCursorPos();
    auto end = ImVec2{pos.x + areaAvail.x, pos.y + lineHeight - 2};

    if (ImGui::IsWindowHovered()) { // if mouse inside this frame
      auto mousePosY = ImGui::GetMousePos().y;

      if (std::trunc(mousePosY / lineHeight) == i) { // is hovering this line
        if(ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
          emuThread->core->toggleBkp(Breakpoint{(u32)(pc - 8), false});
        }
        drawList->AddRectFilled(pos, end, hover_col);
      }
    }

    for(auto bkp : emuThread->core->bkps) {
      if(pc - 8 == bkp.addr && !bkp.ghost) {
        drawList->AddRectFilled(pos, end, bkp_col);
      }
    }

    pos.y = ImGui::GetCursorScreenPos().y;
    end = ImVec2{pos.x + areaAvail.x, pos.y + lineHeight - 2};

    // program counter is teal
    if(pc == rsp.pc)
      drawList->AddRectFilled(pos, end, IM_COL32(0, 128, 128, 80));

    auto instr = Util::ReadAccess<u32>(rsp.imem, pc & IMEM_DSIZE);
    instr = bswap_32(instr);

    auto in = disassembleRSP(pc, instr);
    ImGui::Text("%08X", in.address);
    ImGui::SameLine();
    ImGui::TextColored(instr_imm_col, "%08X", instr);
    ImGui::SameLine();
    ImGui::TextColored(instr_mnemonic_col, "%s", in.mnemonic.c_str());
    if(in.operands.size() > 0) {
      ImGui::SameLine();
      int x = 0;
      for(auto op : in.operands) {
        auto printVec = [this](const char* name, RSP_Reg reg) {
          switch(reg.acType) {
            case Uint8:
              ImGui::TextColored(instr_regs_col, "%s.8[%d]", name, reg.disp);
              break;
            case Uint16:
              ImGui::TextColored(instr_regs_col, "%s.16[%d]", name, reg.disp);
              break;
            case Uint32:
              ImGui::TextColored(instr_regs_col, "%s.32[%d]", name, reg.disp);
              break;
            case Uint128:
              ImGui::TextColored(instr_regs_col, "%s.128", name);
              break;
          }
        };

        auto printReg = [printVec, this](RSP_Reg reg) {
          if(reg.idx == RSP_PC) {
            ImGui::TextColored(instr_regs_col, "pc");
          } else {
            if(reg.idx >= RSP_R0 && reg.idx <= RSP_R31) {
              ImGui::TextColored(instr_regs_col, "%s", regNames[reg.idx].c_str());
            } else if(reg.idx >= RSP_VPR0 && reg.idx <= RSP_VPR15) {
              printVec(fmt::format("vpr{}", reg.idx-33).c_str(), reg);
            } else if(reg.idx == RSP_VCE) {
              printVec("vce", reg);
            } else if(reg.idx == RSP_ACC_LO) {
              printVec("acc.lo", reg);
            } else if(reg.idx == RSP_ACC_MID) {
              printVec("acc.mid", reg);
            } else if(reg.idx == RSP_ACC_HI) {
              printVec("acc.hi", reg);
            } else if(reg.idx == RSP_VCC_LO) {
              printVec("vcc.lo", reg);
            } else if(reg.idx == RSP_VCC_HI) {
              printVec("vcc.hi", reg);
            } else if(reg.idx == RSP_VCO_LO) {
              printVec("vco.lo", reg);
            } else if(reg.idx == RSP_VCO_HI) {
              printVec("vco.hi", reg);
            } else if(reg.idx == RSP_SEMAPHORE) {
              ImGui::TextColored(instr_regs_col, "semaphore");
            } else {
              ImGui::TextColored(instr_regs_col, "unk");
            }
          }
        };

        switch(op.type) {
          case MIPS_OP_REG:
            printReg(op.reg);
            break;
          case MIPS_OP_IMM:
            ImGui::TextColored(instr_imm_col, "#%X", (u32)op.imm);
            break;
          case MIPS_OP_MEM:
            ImGui::TextColored(instr_imm_col, "%X", (u32)op.mem.disp);
            ImGui::SameLine(0, 0);
            ImGui::TextUnformatted("(");
            ImGui::SameLine(0, 0);
            printReg(op.mem.base);
            ImGui::SameLine(0, 0);
            ImGui::TextUnformatted(")");
          break;
          default:
            break;
        }

        if(x < (in.operands.size()-1)) {
          ImGui::SameLine(0, 0);
          ImGui::TextUnformatted(", ");
          ImGui::SameLine(0, 0);
        }

        x++;
      }
    }        
  }

  emuThread->core->pause = false;
}

void DebuggerWindow::renderRegsRSP() {
  emuThread->core->pause = true;
  n64::MMIO& mmio = emuThread->core->cpu->mem.mmio;
  n64::RSP& rsp = mmio.rsp;
  for(int i = 0; i < 32; i += 2) {
    ImGui::Text("%s", regNames[i].c_str());
    ImGui::SameLine();
    ImGui::Text("%s", fmt::format("{:08X}", (u32)rsp.gpr[i]).c_str());
    ImGui::SameLine();
    ImGui::Text("%s", regNames[i+1].c_str());
    ImGui::SameLine();
    ImGui::Text("%s", fmt::format("{:08X}", (u32)rsp.gpr[i+1]).c_str());
  }
  for(int i = 0; i < 32; i += 2) {
    ImGui::Text("%s", fmt::format("vpr{}", i).c_str());
    ImGui::SameLine();
    ImGui::Text("%s", fmt::format("{:08X}{:08X}{:08X}{:08X}", rsp.vpr[i].word[0], rsp.vpr[i].word[1], rsp.vpr[i].word[2], rsp.vpr[i].word[3]).c_str());
    ImGui::SameLine();
    ImGui::Text("%s", fmt::format("vpr{}", i+1).c_str());
    ImGui::SameLine();
    ImGui::Text("%s", fmt::format("{:08X}{:08X}{:08X}{:08X}", rsp.vpr[i+1].word[0], rsp.vpr[i+1].word[1], rsp.vpr[i+1].word[2], rsp.vpr[i+1].word[3]).c_str());
  }
  ImGui::Text("vce");
  ImGui::SameLine();
  ImGui::Text("%s", fmt::format("{:08X}{:08X}{:08X}{:08X}", rsp.vce.word[0], rsp.vce.word[1], rsp.vce.word[2], rsp.vce.word[3]).c_str());
  ImGui::Text("acc.lo");
  ImGui::SameLine();
  ImGui::Text("%s", fmt::format("{:08X}{:08X}{:08X}{:08X}", rsp.acc.l.word[0], rsp.acc.l.word[1], rsp.acc.l.word[2], rsp.acc.l.word[3]).c_str());
  ImGui::Text("acc.mid");
  ImGui::SameLine();
  ImGui::Text("%s", fmt::format("{:08X}{:08X}{:08X}{:08X}", rsp.acc.m.word[0], rsp.acc.m.word[1], rsp.acc.m.word[2], rsp.acc.m.word[3]).c_str());
  ImGui::Text("acc.hi");
  ImGui::SameLine();
  ImGui::Text("%s", fmt::format("{:08X}{:08X}{:08X}{:08X}", rsp.acc.h.word[0], rsp.acc.h.word[1], rsp.acc.h.word[2], rsp.acc.h.word[3]).c_str());
  ImGui::Text("vcc.lo");
  ImGui::SameLine();
  ImGui::Text("%s", fmt::format("{:08X}{:08X}{:08X}{:08X}", rsp.vcc.l.word[0], rsp.vcc.l.word[1], rsp.vcc.l.word[2], rsp.vcc.l.word[3]).c_str());
  ImGui::Text("vcc.hi");
  ImGui::SameLine();
  ImGui::Text("%s", fmt::format("{:08X}{:08X}{:08X}{:08X}", rsp.vcc.h.word[0], rsp.vcc.h.word[1], rsp.vcc.h.word[2], rsp.vcc.h.word[3]).c_str());
  ImGui::Text("vco.lo");
  ImGui::SameLine();
  ImGui::Text("%s", fmt::format("{:08X}{:08X}{:08X}{:08X}", rsp.vco.l.word[0], rsp.vco.l.word[1], rsp.vco.l.word[2], rsp.vco.l.word[3]).c_str());
  ImGui::Text("vco.hi");
  ImGui::SameLine();
  ImGui::Text("%s", fmt::format("{:08X}{:08X}{:08X}{:08X}", rsp.vco.h.word[0], rsp.vco.h.word[1], rsp.vco.h.word[2], rsp.vco.h.word[3]).c_str());
  ImGui::Text("semaphore");
  ImGui::SameLine();
  ImGui::Text("%s", fmt::format("{}", rsp.semaphore).c_str());
  emuThread->core->pause = false;
}

void DebuggerWindow::renderCPU() {
  static std::string goToAddrBuf{"00000000"};
  static u32 goToAddr=0;
  ImGui::BeginChild("##cpudisasm");
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
  ImGui::BeginChild("Disassembly", ImVec2((float)size().width()*6.f/10.f,0), 0, ImGuiWindowFlags_NoScrollbar);
  renderDisasm();
  ImGui::EndChild();
  ImGui::SameLine();
  ImGui::BeginChild("Registers", ImVec2((float)size().width()*4.f/10.f,0));
  renderRegs();
  ImGui::EndChild();
  ImGui::EndChild();
}

void DebuggerWindow::renderRSP() {
  static std::string goToAddrBuf{"0000"};
  static u16 goToAddr=0;
  ImGui::BeginChild("##cpudisasm");
  if(ImGui::Button("Continue")) {
    emuThread->core->Step<true>();
    emuThread->core->broken = false;
  }
  ImGui::SameLine();
  if(ImGui::Button("Step over")) {
    emuThread->core->Step<true>();
    emuThread->core->broken = false;
  }
  ImGui::SameLine();
  if(ImGui::Button("Step in")) {
    emuThread->core->Step<true>();
  }
  ImGui::SameLine();
  if(ImGui::Button("Step out")) {
    emuThread->core->Step<true>();
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
    goToAddr &= 0x1FFF;
    goToAddr &= ~3;
    scrollAmount = goToAddr;
  }
  ImGui::BeginChild("Disassembly", ImVec2((float)size().width()*6.f/10.f,0), 0, ImGuiWindowFlags_NoScrollbar);
  renderDisasmRSP();
  ImGui::EndChild();
  ImGui::SameLine();
  ImGui::BeginChild("Registers", ImVec2((float)size().width()*4.f/10.f,0));
  renderRegsRSP();
  ImGui::EndChild();
  ImGui::EndChild();
}

void DebuggerWindow::paintGL() {
  QtImGui::newFrame();

  grabKeyboard();

  ImGui::SetNextWindowPos(ImVec2{0.f, 0.f});
  ImGui::SetNextWindowSize(ImVec2{(float)size().width(), (float)size().height()});
  ImGui::Begin("##debugger", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
  ImGui::BeginTabBar("Debuggers");
  if(ImGui::BeginTabItem("CPU")) {
    isRSPfocused = false;
    renderCPU();
    ImGui::EndTabItem();
  }

  if(ImGui::BeginTabItem("RSP")) {
    isRSPfocused = true;
    renderRSP();
    ImGui::EndTabItem();
  }
  ImGui::EndTabBar();
  ImGui::End();

  // Do render before ImGui UI is rendered
  glViewport(0, 0, width(), height());
  glClearColor(0, 0, 0, 255);
  glClear(GL_COLOR_BUFFER_BIT);

  releaseKeyboard();

  ImGui::Render();
  QtImGui::render();
}