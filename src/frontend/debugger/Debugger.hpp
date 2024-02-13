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

enum RSP_Reg {
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

enum RSP_VPRAccessType {
  Uint8, Uint16, Uint32, Uint128
};

struct RSP_Mem {
	RSP_Reg base;	///< base register
	int64_t disp;	///< displacement/offset value
};

struct RSP_Operand {
  mips_op_type type;
  union {
    RSP_Reg reg;
    int64_t imm;		///< immediate value for IMM operand
		RSP_Mem mem;	///< base/index/scale/disp value for MEM operand
  };
};

class DebuggerWindow : public QOpenGLWidget, private QOpenGLExtraFunctions {
  QTimer timer;
  csh disasmHandle;
  bool followPC = false;
  u64 scrollAmount = 0;
  void renderDisasm();
  void renderRegs();
  void renderCPU();
  void renderDisasmRSP();
  void renderRegsRSP();
  void renderRSP();
  EmuThread* emuThread;
  ImU32 hover_col;
  ImU32 bkp_col;
  ImVec4 instr_imm_col;
  ImVec4 instr_mnemonic_col;
  ImVec4 instr_regs_col;
public:
  void wheelEvent(QWheelEvent*) override;
  DebuggerWindow(EmuThread*);
protected:
  void initializeGL() override;
  void paintGL() override;
};