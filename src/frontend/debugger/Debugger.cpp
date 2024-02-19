#include <Debugger.hpp>
#include <imgui.h>
#include <imgui_stdlib.h>
#include <QGuiApplication>
#include <QStyleHints>
#include <QWheelEvent>
#include <CpuDefinitions.hpp>
#include <AudioSettings.hpp>

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

DebuggerWindow::DebuggerWindow(EmuThread* emuThread, AudioSettings* audio) : audio(audio), emuThread(emuThread), QOpenGLWidget(nullptr) {
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

  setFocusPolicy(Qt::WheelFocus);
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
    bkp_col = IM_COL32(168, 147, 40, 80);
    hover_col = IM_COL32(173, 35, 35, 80);
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
}

void DebuggerWindow::renderRegs() {
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
}

static RSP_Instruction disasmSWC2(u32 address, u32 instr) {
  u8 mask = (instr >> 11) & 0x1f;
  std::vector<RSP_Operand> operands{};
  RSP_Operand op;
  op.type = MIPS_OP_REG;
  op.reg.i = static_cast<eRSP_Reg>(VT(instr) + 34);
  op.reg.e = E1(instr);
  operands.push_back(op);
  op.type = MIPS_OP_MEM;
  op.mem.base.i = static_cast<eRSP_Reg>(BASE(instr) + 1);
  op.mem.disp = OFFSET(instr);
  operands.push_back(op);

  switch(mask) {
    case 0x00: return {address, operands, "sbv"};
    case 0x01: return {address, operands, "ssv"};
    case 0x02: return {address, operands, "slv"};
    case 0x03: return {address, operands, "sdv"};
    case 0x04: return {address, operands, "sqv"};
    case 0x05: return {address, operands, "srv"};
    case 0x06: return {address, operands, "spv"};
    case 0x07: return {address, operands, "suv"};
    case 0x08: return {address, operands, "shv"};
    case 0x09: return {address, operands, "sfv"};
    case 0x0A: return {address, operands, "swv"};
    case 0x0B: return {address, operands, "stv"};
    default: return {address, {}, "unk"};
  }
}

static RSP_Instruction disasmLWC2(u32 address, u32 instr) {
  u8 mask = (instr >> 11) & 0x1f;
  std::vector<RSP_Operand> operands{};
  RSP_Operand op;
  op.type = MIPS_OP_REG;
  op.reg.i = static_cast<eRSP_Reg>(VT(instr) + 34);
  op.reg.e = E1(instr);
  operands.push_back(op);
  op.type = MIPS_OP_MEM;
  op.mem.base.i = static_cast<eRSP_Reg>(BASE(instr) + 1);
  op.mem.disp = OFFSET(instr);
  operands.push_back(op);

  switch(mask) {
    case 0x00: return {address, operands, "lbv"};
    case 0x01: return {address, operands, "lsv"};
    case 0x02: return {address, operands, "llv"};
    case 0x03: return {address, operands, "ldv"};
    case 0x04: return {address, operands, "lqv"};
    case 0x05: return {address, operands, "lrv"};
    case 0x06: return {address, operands, "lpv"};
    case 0x07: return {address, operands, "luv"};
    case 0x08: return {address, operands, "lhv"};
    case 0x09: return {address, operands, "lfv"};
    case 0x0A: return { address, {}, "swv (nop)" };
    case 0x0B: return {address, operands, "ltv"};
    default: return {address, {}, "unk"};
  }
}

static RSP_Instruction disasmRSPUnique(u32 address, u32 instr) {
  switch(instr) {
    case n64::LWC2: return disasmLWC2(address, instr);
    case n64::SWC2: return disasmSWC2(address, instr);
    case n64::COP2:
    default: return {address, {}, "unk"};
  }
}

RSP_Instruction DebuggerWindow::disassembleRSP(u32 address, u32 instr) {
  switch (instr) {
    case n64::COP2: case n64::LWC2: case n64::SWC2: return disasmRSPUnique(address, instr);
    default: {
      cs_insn* insn;
      size_t count = cs_disasm(rspHandle, reinterpret_cast<u8*>(&instr), 4, address, 0, &insn);
      if(count > 0) {
        std::vector<RSP_Operand> operands;
        for(int i = 0; i < insn->detail->mips.op_count; i++) {
          cs_mips_op mips_op = insn->detail->mips.operands[i];
          RSP_Operand op;
          op.type = mips_op.type;
          op.reg.i = static_cast<eRSP_Reg>(mips_op.reg - 1);
          op.imm = mips_op.imm;
          op.mem.base.i = static_cast<eRSP_Reg>(mips_op.mem.base - 1);
          op.mem.disp = mips_op.mem.disp;
          operands.push_back(op);
        }
        return { address, operands, insn->mnemonic };
      }
      return { address, {}, "unk" };
    }
  }
}

void DebuggerWindow::renderDisasmRSP() {
  auto areaAvail = ImGui::GetContentRegionAvail();
  auto lineHeight = ImGui::GetTextLineHeightWithSpacing();
  auto lineCount = areaAvail.y / lineHeight;
  auto drawList = ImGui::GetWindowDrawList();

  n64::Mem& mem = emuThread->core->cpu->mem;
  n64::RSP& rsp = mem.mmio.rsp;

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
        auto printReg = [this](RSP_Reg reg) {
          int elem = reg.e;
          eRSP_Reg reg_idx = reg.i;
          if(reg_idx== RSP_PC) {
            ImGui::TextColored(instr_regs_col, "pc");
          } else {
            if(reg_idx>= RSP_R0 && reg_idx<= RSP_R31) {
              ImGui::TextColored(instr_regs_col, "%s", regNames[reg_idx].c_str());
            } else if(reg_idx>= RSP_VPR0 && reg_idx<= RSP_VPR15) {
              ImGui::TextColored(instr_regs_col, "%s", fmt::format("vpr{}[{}]", reg_idx-34, elem).c_str());
            } else if(reg_idx== RSP_VCE) {
              ImGui::TextColored(instr_regs_col, "%s", "vce");
            } else if(reg_idx== RSP_ACC_LO) {
              ImGui::TextColored(instr_regs_col, "%s", "acc.lo");
            } else if(reg_idx== RSP_ACC_MID) {
              ImGui::TextColored(instr_regs_col, "%s", "acc.mid");
            } else if(reg_idx== RSP_ACC_HI) {
              ImGui::TextColored(instr_regs_col, "%s", "acc.hi");
            } else if(reg_idx== RSP_VCC_LO) {
              ImGui::TextColored(instr_regs_col, "%s", "vcc.lo");
            } else if(reg_idx== RSP_VCC_HI) {
              ImGui::TextColored(instr_regs_col, "%s", "vcc.hi");
            } else if(reg_idx== RSP_VCO_LO) {
              ImGui::TextColored(instr_regs_col, "%s", "vco.lo");
            } else if(reg_idx== RSP_VCO_HI) {
              ImGui::TextColored(instr_regs_col, "%s", "vco.hi");
            } else if(reg_idx== RSP_SEMAPHORE) {
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
}

void DebuggerWindow::renderRegsRSP() {
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
}

void DebuggerWindow::focusInEvent(QFocusEvent*) {
  grabKeyboard();
}

void DebuggerWindow::focusOutEvent(QFocusEvent*) {
  releaseKeyboard();
  emit regrabKeyboard();
}

void DebuggerWindow::renderCPU() {
  static std::string goToAddrBuf{"00000000"};
  static u32 goToAddr=0;
  ImGui::BeginChild("##cpudisasm");
  ImGui::BeginDisabled(!emuThread->core->broken && !emuThread->core->pause);
  if(ImGui::Button("Continue")) {
    emuThread->core->Step(audio->volumeL->value(), audio->volumeR->value());
    emuThread->core->broken = false;
  }
  ImGui::SameLine();
  if(ImGui::Button("Step over")) {
    emuThread->core->Step(audio->volumeL->value(), audio->volumeR->value());
    emuThread->core->broken = false;
  }
  ImGui::SameLine();
  if(ImGui::Button("Step in")) {
    emuThread->core->Step(audio->volumeL->value(), audio->volumeR->value());
  }
  ImGui::SameLine();
  if(ImGui::Button("Step out")) {
    emuThread->core->Step(audio->volumeL->value(), audio->volumeR->value());
    emuThread->core->broken = false;
  }
  ImGui::EndDisabled();
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
  ImGui::BeginDisabled((!emuThread->core->broken && !emuThread->core->pause) || emuThread->core->cpu->mem.mmio.rsp.spStatus.halt);
  if(ImGui::Button("Continue")) {
    emuThread->core->Step<true>(audio->volumeL->value(), audio->volumeR->value());
    emuThread->core->broken = false;
  }
  ImGui::SameLine();
  if(ImGui::Button("Step over")) {
    emuThread->core->Step<true>(audio->volumeL->value(), audio->volumeR->value());
    emuThread->core->broken = false;
  }
  ImGui::SameLine();
  if(ImGui::Button("Step in")) {
    emuThread->core->Step<true>(audio->volumeL->value(), audio->volumeR->value());
  }
  ImGui::SameLine();
  if(ImGui::Button("Step out")) {
    emuThread->core->Step<true>(audio->volumeL->value(), audio->volumeR->value());
    emuThread->core->broken = false;
  }
  ImGui::EndDisabled();
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

  ImGui::Render();
  QtImGui::render();
}