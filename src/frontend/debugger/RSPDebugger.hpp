#pragma once
#include <EmuThread.hpp>
#include <capstone/capstone.h>
#include <set>

struct RSPDebugger {
  csh handle;
  EmuThread* emuThread;
  cs_insn disassemble(u32, u16);
  RSPDebugger(EmuThread*);
  ImVec4 instr_imm_col;
  ImVec4 instr_mnemonic_col;
  ImVec4 instr_regs_col;
};