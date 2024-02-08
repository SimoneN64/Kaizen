#pragma once
#include <EmuThread.hpp>
#include <capstone/capstone.h>
#include <set>

struct RSPDebugger {
  csh handle;
  EmuThread* emuThread;
  cs_insn disassemble(u32);
  RSPDebugger(EmuThread*);
};