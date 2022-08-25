#pragma once
#define GDBSTUB_IMPLEMENTATION
#include <gdbstub.h>
#include <util.hpp>

struct Core;

#define GDB_CPU_PORT 1337

struct Breakpoint {
  u32 addr;
  Breakpoint* next;
};

struct DebuggerState {
  gdbstub_t* gdb;
  bool broken;
  int steps;
  Breakpoint* breakpoints;
  bool enabled;
};

inline bool CheckBreakpoint(DebuggerState& state, u32 addr) {
  Breakpoint* cur = state.breakpoints;
  while (cur != NULL) {
    if (cur->addr == addr) {
      util::print("Hit breakpoint at 0x{:08X}\n", addr);
      return true;
    }
    cur = cur->next;
  }
  return false;
}

const char* target_xml =
  "<?xml version=\"1.0\"?>"
  "<!DOCTYPE feature SYSTEM \"gdb-target.dtd\">"
  "<target version=\"1.0\">"
  "<architecture>mips:4000</architecture>"
  "<osabi>none</osabi>"
  "<feature name=\"org.gnu.gdb.mips.cpu\">"
  "  <reg name=\"r0\" bitsize=\"64\" regnum=\"0\"/>"
  "  <reg name=\"r1\" bitsize=\"64\"/>"
  "  <reg name=\"r2\" bitsize=\"64\"/>"
  "  <reg name=\"r3\" bitsize=\"64\"/>"
  "  <reg name=\"r4\" bitsize=\"64\"/>"
  "  <reg name=\"r5\" bitsize=\"64\"/>"
  "  <reg name=\"r6\" bitsize=\"64\"/>"
  "  <reg name=\"r7\" bitsize=\"64\"/>"
  "  <reg name=\"r8\" bitsize=\"64\"/>"
  "  <reg name=\"r9\" bitsize=\"64\"/>"
  "  <reg name=\"r10\" bitsize=\"64\"/>"
  "  <reg name=\"r11\" bitsize=\"64\"/>"
  "  <reg name=\"r12\" bitsize=\"64\"/>"
  "  <reg name=\"r13\" bitsize=\"64\"/>"
  "  <reg name=\"r14\" bitsize=\"64\"/>"
  "  <reg name=\"r15\" bitsize=\"64\"/>"
  "  <reg name=\"r16\" bitsize=\"64\"/>"
  "  <reg name=\"r17\" bitsize=\"64\"/>"
  "  <reg name=\"r18\" bitsize=\"64\"/>"
  "  <reg name=\"r19\" bitsize=\"64\"/>"
  "  <reg name=\"r20\" bitsize=\"64\"/>"
  "  <reg name=\"r21\" bitsize=\"64\"/>"
  "  <reg name=\"r22\" bitsize=\"64\"/>"
  "  <reg name=\"r23\" bitsize=\"64\"/>"
  "  <reg name=\"r24\" bitsize=\"64\"/>"
  "  <reg name=\"r25\" bitsize=\"64\"/>"
  "  <reg name=\"r26\" bitsize=\"64\"/>"
  "  <reg name=\"r27\" bitsize=\"64\"/>"
  "  <reg name=\"r28\" bitsize=\"64\"/>"
  "  <reg name=\"r29\" bitsize=\"64\"/>"
  "  <reg name=\"r30\" bitsize=\"64\"/>"
  "  <reg name=\"r31\" bitsize=\"64\"/>"
  "  <reg name=\"lo\" bitsize=\"64\" regnum=\"33\"/>"
  "  <reg name=\"hi\" bitsize=\"64\" regnum=\"34\"/>"
  "  <reg name=\"pc\" bitsize=\"64\" regnum=\"37\"/>"
  "</feature>"
  "<feature name=\"org.gnu.gdb.mips.cp0\">"
  "  <reg name=\"status\" bitsize=\"32\" regnum=\"32\"/>"
  "  <reg name=\"badvaddr\" bitsize=\"32\" regnum=\"35\"/>"
  "  <reg name=\"cause\" bitsize=\"32\" regnum=\"36\"/>"
  "  </feature>"
  "<!-- TODO fix the sizes here. How do we deal with configurable sizes? -->"
  "<feature name=\"org.gnu.gdb.mips.fpu\">"
  "  <reg name=\"f0\" bitsize=\"32\" type=\"ieee_single\" regnum=\"38\"/>"
  "  <reg name=\"f1\" bitsize=\"32\" type=\"ieee_single\"/>"
  "  <reg name=\"f2\" bitsize=\"32\" type=\"ieee_single\"/>"
  "  <reg name=\"f3\" bitsize=\"32\" type=\"ieee_single\"/>"
  "  <reg name=\"f4\" bitsize=\"32\" type=\"ieee_single\"/>"
  "  <reg name=\"f5\" bitsize=\"32\" type=\"ieee_single\"/>"
  "  <reg name=\"f6\" bitsize=\"32\" type=\"ieee_single\"/>"
  "  <reg name=\"f7\" bitsize=\"32\" type=\"ieee_single\"/>"
  "  <reg name=\"f8\" bitsize=\"32\" type=\"ieee_single\"/>"
  "  <reg name=\"f9\" bitsize=\"32\" type=\"ieee_single\"/>"
  "  <reg name=\"f10\" bitsize=\"32\" type=\"ieee_single\"/>"
  "  <reg name=\"f11\" bitsize=\"32\" type=\"ieee_single\"/>"
  "  <reg name=\"f12\" bitsize=\"32\" type=\"ieee_single\"/>"
  "  <reg name=\"f13\" bitsize=\"32\" type=\"ieee_single\"/>"
  "  <reg name=\"f14\" bitsize=\"32\" type=\"ieee_single\"/>"
  "  <reg name=\"f15\" bitsize=\"32\" type=\"ieee_single\"/>"
  "  <reg name=\"f16\" bitsize=\"32\" type=\"ieee_single\"/>"
  "  <reg name=\"f17\" bitsize=\"32\" type=\"ieee_single\"/>"
  "  <reg name=\"f18\" bitsize=\"32\" type=\"ieee_single\"/>"
  "  <reg name=\"f19\" bitsize=\"32\" type=\"ieee_single\"/>"
  "  <reg name=\"f20\" bitsize=\"32\" type=\"ieee_single\"/>"
  "  <reg name=\"f21\" bitsize=\"32\" type=\"ieee_single\"/>"
  "  <reg name=\"f22\" bitsize=\"32\" type=\"ieee_single\"/>"
  "  <reg name=\"f23\" bitsize=\"32\" type=\"ieee_single\"/>"
  "  <reg name=\"f24\" bitsize=\"32\" type=\"ieee_single\"/>"
  "  <reg name=\"f25\" bitsize=\"32\" type=\"ieee_single\"/>"
  "  <reg name=\"f26\" bitsize=\"32\" type=\"ieee_single\"/>"
  "  <reg name=\"f27\" bitsize=\"32\" type=\"ieee_single\"/>"
  "  <reg name=\"f28\" bitsize=\"32\" type=\"ieee_single\"/>"
  "  <reg name=\"f29\" bitsize=\"32\" type=\"ieee_single\"/>"
  "  <reg name=\"f30\" bitsize=\"32\" type=\"ieee_single\"/>"
  "  <reg name=\"f31\" bitsize=\"32\" type=\"ieee_single\"/>"
  "  <reg name=\"fcsr\" bitsize=\"32\" group=\"float\"/>"
  "  <reg name=\"fir\" bitsize=\"32\" group=\"float\"/>"
  "</feature>"
  "</target>";

const char* memory_map =
  "<?xml version=\"1.0\"?>"
  "<memory-map>"
  "<!-- KUSEG - TLB mapped, treat it as a giant block of RAM. Not ideal, but not sure how else to deal with it -->"
  "<memory type=\"ram\" start=\"0x0000000000000000\" length=\"0x80000000\"/>"

  "<!-- KSEG0 hardware mapped, full copy of the memory map goes here -->" // TODO finish
  "<memory type=\"ram\" start=\"0xffffffff80000000\" length=\"0x800000\"/>" // RDRAM
  "<memory type=\"ram\" start=\"0xffffffff84000000\" length=\"0x1000\"/>" // RSP DMEM
  "<memory type=\"ram\" start=\"0xffffffff84001000\" length=\"0x1000\"/>" // RSP IMEM
  "<memory type=\"rom\" start=\"0xffffffff9fc00000\" length=\"0x7c0\"/>" // PIF ROM

  "<!-- KSEG1 hardware mapped, full copy of the memory map goes here -->" // TODO finish
  "<memory type=\"ram\" start=\"0xffffffffa0000000\" length=\"0x800000\"/>" // RDRAM
  "<memory type=\"ram\" start=\"0xffffffffa4000000\" length=\"0x1000\"/>" // RSP DMEM
  "<memory type=\"ram\" start=\"0xffffffffa4001000\" length=\"0x1000\"/>" // RSP IMEM
  "<memory type=\"rom\" start=\"0xffffffffbfc00000\" length=\"0x7c0\"/>" // PIF ROM
  "</memory-map>";


void DebugStart(void* user_data);
void DebugStop(void* user_data);
void DebugStep(void* user_data);
void DebugSetBreakpoint(void* user_data, u32 address);
void DebugClearBreakpoint(void* user_data, u32 address);
ssize_t DebugGetMemory(void* user_data, char* buffer, size_t length, u32 address, size_t bytes);
ssize_t DebugGetRegisterValue(void* user_data, char * buffer, size_t buffer_length, int reg);
ssize_t DebugGetGeneralRegisters(void* user_data, char * buffer, size_t buffer_length);