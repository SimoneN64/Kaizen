#include <RSPDebugger.hpp>
#include <imgui.h>
#include <imgui_stdlib.h>
#include <log.hpp>

RSPDebugger::RSPDebugger(EmuThread* emuThread) : emuThread(emuThread) {
  if (cs_open(CS_ARCH_MIPS, cs_mode(CS_MODE_BIG_ENDIAN | CS_MODE_MIPS32), &handle) != CS_ERR_OK) {
    Util::panic("Could not initialize capstone for RSP!");
  }

  if (cs_option(handle, CS_OPT_DETAIL, CS_OPT_ON) != CS_ERR_OK) {
    Util::panic("Could not enable capstone detail for RSP!");
  }
}

cs_insn RSPDebugger::disassemble(u32 instr) {

}