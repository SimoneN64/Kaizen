#pragma once
#include <QOpenGLExtraFunctions>
#include <QOpenGLWidget>
#include <QTimer>
#include <QtImGui.h>
#include <set>
#include <log.hpp>
#include <capstone/capstone.h>
#include <EmuThread.hpp>
#include <imgui.h>

class AudioSettings;

enum eRSP_Reg {
  RSP_INVALID,
  RSP_R0,
  RSP_R1,
  RSP_R2,
  RSP_R3,
  RSP_R4,
  RSP_R5,
  RSP_R6,
  RSP_R7,
  RSP_R8,
  RSP_R9,
  RSP_R10,
  RSP_R11,
  RSP_R12,
  RSP_R13,
  RSP_R14,
  RSP_R15,
  RSP_R16,
  RSP_R17,
  RSP_R18,
  RSP_R19,
  RSP_R20,
  RSP_R21,
  RSP_R22,
  RSP_R23,
  RSP_R24,
  RSP_R25,
  RSP_R26,
  RSP_R27,
  RSP_R28,
  RSP_R29,
  RSP_R30,
  RSP_R31,
  RSP_PC,
  RSP_VPR0,
  RSP_VPR1,
  RSP_VPR2,
  RSP_VPR3,
  RSP_VPR4,
  RSP_VPR5,
  RSP_VPR6,
  RSP_VPR7,
  RSP_VPR8,
  RSP_VPR9,
  RSP_VPR10,
  RSP_VPR11,
  RSP_VPR12,
  RSP_VPR13,
  RSP_VPR14,
  RSP_VPR15,
  RSP_VCE,
  RSP_ACC_LO,
  RSP_ACC_MID,
  RSP_ACC_HI,
  RSP_VCC_LO,
  RSP_VCC_HI,
  RSP_VCO_LO,
  RSP_VCO_HI,
  RSP_SEMAPHORE
};

struct RSP_Reg {
  int e;
  eRSP_Reg i;
};

struct RSP_Mem {
	RSP_Reg base;	///< base register
	int64_t disp;	///< displacement/offset value

  RSP_Mem() {
    disp = -1;
  }
};

struct RSP_Operand {
  mips_op_type type;
  union {
    RSP_Reg reg;
    int64_t imm;		///< immediate value for IMM operand
		RSP_Mem mem;	///< base/index/scale/disp value for MEM operand
  };

  RSP_Operand() {
    type = MIPS_OP_INVALID;
    imm = -1;
  }
};

struct RSP_Instruction {
  u32 address;
  std::vector<RSP_Operand> operands;
  std::string mnemonic;
};

class DebuggerWindow : public QOpenGLWidget, private QOpenGLExtraFunctions {
Q_OBJECT
  QTimer timer;
  csh disasmHandle, rspHandle;
  bool followPC = false;
  bool isRSPfocused = false;
  u64 scrollAmount = 0;
  u16 scrollAmountRSP = 0;
  void renderDisasm();
  void renderRegs();
  void renderCPU();
  void renderDisasmRSP();
  void renderRegsRSP();
  void renderRSP();
  RSP_Instruction disassembleRSP(u32, u32);
  EmuThread* emuThread;
  ImU32 hover_col;
  ImU32 bkp_col;
  ImVec4 instr_imm_col;
  ImVec4 instr_mnemonic_col;
  ImVec4 instr_regs_col;
  AudioSettings* audio;
  void focusInEvent(QFocusEvent*) override;
  void focusOutEvent(QFocusEvent*) override;
public:
  void wheelEvent(QWheelEvent*) override;
  DebuggerWindow(EmuThread*, AudioSettings*);
protected:
  void initializeGL() override;
  void paintGL() override;
Q_SIGNALS:
  void regrabKeyboard();
};