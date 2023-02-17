#include <Debugger.hpp>

#ifndef DISABLE_GDB_STUB
#define GDBSTUB_IMPLEMENTATION
#include <gdbstub.h>
#include <log.hpp>
#include <Core.hpp>

const char* target_xml =
  "<?xml version=\"1.0\"?>"
  "<!DOCTYPE feature SYSTEM \"gdb-target.dtd\">"
  "<target version=\"1.0\">"
  "<architecture>mips:4000</architecture>"
  "<osabi>none</osabi>"
  "<feature name=\"org.gnu.gdb.mips.cpu\">"
  "        <reg name=\"r0\" bitsize=\"64\" regnum=\"0\"/>"
  "        <reg name=\"r1\" bitsize=\"64\"/>"
  "        <reg name=\"r2\" bitsize=\"64\"/>"
  "        <reg name=\"r3\" bitsize=\"64\"/>"
  "        <reg name=\"r4\" bitsize=\"64\"/>"
  "        <reg name=\"r5\" bitsize=\"64\"/>"
  "        <reg name=\"r6\" bitsize=\"64\"/>"
  "        <reg name=\"r7\" bitsize=\"64\"/>"
  "        <reg name=\"r8\" bitsize=\"64\"/>"
  "        <reg name=\"r9\" bitsize=\"64\"/>"
  "        <reg name=\"r10\" bitsize=\"64\"/>"
  "        <reg name=\"r11\" bitsize=\"64\"/>"
  "        <reg name=\"r12\" bitsize=\"64\"/>"
  "        <reg name=\"r13\" bitsize=\"64\"/>"
  "        <reg name=\"r14\" bitsize=\"64\"/>"
  "        <reg name=\"r15\" bitsize=\"64\"/>"
  "        <reg name=\"r16\" bitsize=\"64\"/>"
  "        <reg name=\"r17\" bitsize=\"64\"/>"
  "        <reg name=\"r18\" bitsize=\"64\"/>"
  "        <reg name=\"r19\" bitsize=\"64\"/>"
  "        <reg name=\"r20\" bitsize=\"64\"/>"
  "        <reg name=\"r21\" bitsize=\"64\"/>"
  "        <reg name=\"r22\" bitsize=\"64\"/>"
  "        <reg name=\"r23\" bitsize=\"64\"/>"
  "        <reg name=\"r24\" bitsize=\"64\"/>"
  "        <reg name=\"r25\" bitsize=\"64\"/>"
  "        <reg name=\"r26\" bitsize=\"64\"/>"
  "        <reg name=\"r27\" bitsize=\"64\"/>"
  "        <reg name=\"r28\" bitsize=\"64\"/>"
  "        <reg name=\"r29\" bitsize=\"64\"/>"
  "        <reg name=\"r30\" bitsize=\"64\"/>"
  "        <reg name=\"r31\" bitsize=\"64\"/>"
  "        <reg name=\"lo\" bitsize=\"64\" regnum=\"33\"/>"
  "        <reg name=\"hi\" bitsize=\"64\" regnum=\"34\"/>"
  "        <reg name=\"pc\" bitsize=\"64\" regnum=\"37\"/>"
  "        </feature>"
  "<feature name=\"org.gnu.gdb.mips.cp0\">"
  "        <reg name=\"status\" bitsize=\"32\" regnum=\"32\"/>"
  "        <reg name=\"badvaddr\" bitsize=\"32\" regnum=\"35\"/>"
  "        <reg name=\"cause\" bitsize=\"32\" regnum=\"36\"/>"
  "        </feature>"
  "<!-- TODO fix the sizes here. How do we deal with configurable sizes? -->"
  "<feature name=\"org.gnu.gdb.mips.fpu\">"
  "        <reg name=\"f0\" bitsize=\"32\" type=\"ieee_single\" regnum=\"38\"/>"
  "        <reg name=\"f1\" bitsize=\"32\" type=\"ieee_single\"/>"
  "        <reg name=\"f2\" bitsize=\"32\" type=\"ieee_single\"/>"
  "        <reg name=\"f3\" bitsize=\"32\" type=\"ieee_single\"/>"
  "        <reg name=\"f4\" bitsize=\"32\" type=\"ieee_single\"/>"
  "        <reg name=\"f5\" bitsize=\"32\" type=\"ieee_single\"/>"
  "        <reg name=\"f6\" bitsize=\"32\" type=\"ieee_single\"/>"
  "        <reg name=\"f7\" bitsize=\"32\" type=\"ieee_single\"/>"
  "        <reg name=\"f8\" bitsize=\"32\" type=\"ieee_single\"/>"
  "        <reg name=\"f9\" bitsize=\"32\" type=\"ieee_single\"/>"
  "        <reg name=\"f10\" bitsize=\"32\" type=\"ieee_single\"/>"
  "        <reg name=\"f11\" bitsize=\"32\" type=\"ieee_single\"/>"
  "        <reg name=\"f12\" bitsize=\"32\" type=\"ieee_single\"/>"
  "        <reg name=\"f13\" bitsize=\"32\" type=\"ieee_single\"/>"
  "        <reg name=\"f14\" bitsize=\"32\" type=\"ieee_single\"/>"
  "        <reg name=\"f15\" bitsize=\"32\" type=\"ieee_single\"/>"
  "        <reg name=\"f16\" bitsize=\"32\" type=\"ieee_single\"/>"
  "        <reg name=\"f17\" bitsize=\"32\" type=\"ieee_single\"/>"
  "        <reg name=\"f18\" bitsize=\"32\" type=\"ieee_single\"/>"
  "        <reg name=\"f19\" bitsize=\"32\" type=\"ieee_single\"/>"
  "        <reg name=\"f20\" bitsize=\"32\" type=\"ieee_single\"/>"
  "        <reg name=\"f21\" bitsize=\"32\" type=\"ieee_single\"/>"
  "        <reg name=\"f22\" bitsize=\"32\" type=\"ieee_single\"/>"
  "        <reg name=\"f23\" bitsize=\"32\" type=\"ieee_single\"/>"
  "        <reg name=\"f24\" bitsize=\"32\" type=\"ieee_single\"/>"
  "        <reg name=\"f25\" bitsize=\"32\" type=\"ieee_single\"/>"
  "        <reg name=\"f26\" bitsize=\"32\" type=\"ieee_single\"/>"
  "        <reg name=\"f27\" bitsize=\"32\" type=\"ieee_single\"/>"
  "        <reg name=\"f28\" bitsize=\"32\" type=\"ieee_single\"/>"
  "        <reg name=\"f29\" bitsize=\"32\" type=\"ieee_single\"/>"
  "        <reg name=\"f30\" bitsize=\"32\" type=\"ieee_single\"/>"
  "        <reg name=\"f31\" bitsize=\"32\" type=\"ieee_single\"/>"
  "        <reg name=\"fcsr\" bitsize=\"32\" group=\"float\"/>"
  "        <reg name=\"fir\" bitsize=\"32\" group=\"float\"/>"
  "        </feature>"
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

void debugStart(void* user_data) {
  auto* debugger = (Debugger*)user_data;
  debugger->broken = false;
}

void debugStop(void* user_data) {
  auto* debugger = (Debugger*)user_data;
  debugger->broken = true;
}

void debugStep(void* user_data) {
  auto* debugger = (Debugger*)user_data;
  bool old_broken = debugger->broken;
  debugger->broken = false;
  n64::Core::CpuStep(debugger->core);
  debugger->broken = old_broken;
  debugger->steps += 2;
}

void debugSetBreakpoint(void* user_data, u32 address) {
  auto* debugger = (Debugger*)user_data;
  auto* breakpoint = (Breakpoint*)malloc(sizeof(Breakpoint));
  breakpoint->address = address;
  breakpoint->next = nullptr;

  // Special case for this being the first breakpoint
  if (debugger->breakpoints == nullptr) {
    debugger->breakpoints = breakpoint;
  } else {
    // Find end of the list
    auto* tail = debugger->breakpoints;
    while (tail->next != nullptr) {
      tail = tail->next;
    }

    tail->next = breakpoint;
  }
}

void debugClearBreakpoint(void* user_data, u32 address) {
  auto* debugger = (Debugger*)user_data;
  if (debugger->breakpoints == nullptr) {
    return; // No breakpoints set at all
  } else if (debugger->breakpoints->address == address) {
    // Special case for the first breakpoint being the one we want to clear
    auto* next = debugger->breakpoints->next;
    free(debugger->breakpoints);
    debugger->breakpoints = next;
  } else {
    // Find the breakpoint somewhere in the list and free it
    auto* iter = debugger->breakpoints;
    while (iter->next != nullptr) {
      if (iter->next->address == address) {
        auto* next = iter->next->next;
        free(iter->next);
        iter->next = next;
      }
    }
  }
}

size_t debugGetMemory(void* user_data, char* buffer, size_t length, u32 address, size_t bytes) {
  auto* debugger = (Debugger*)user_data;
  printf("Checking memory at address 0x%08X\n", address);
  int printed = 0;
  u32 paddr;
  if(!n64::MapVAddr(debugger->core.CpuGetRegs(), n64::LOAD, address, paddr)) {
    return 0;
  }

  for (int i = 0; i < bytes; i++) {
    u8 value = debugger->core.mem.Read8(debugger->core.CpuGetRegs(), paddr + i);
    printed += snprintf(buffer + (i*2), length, "%02X", value);
  }
  printf("Get memory: %ld bytes from 0x%08X: %d\n", bytes, paddr, printed);
  return printed + 1;
}

size_t debugGetRegisterValue(void* user_data, char * buffer, size_t buffer_length, int reg) {
  auto* debugger = (Debugger*)user_data;
  switch (reg) {
    case 0 ... 31:
      return snprintf(buffer, buffer_length, "%016lx", debugger->core.CpuGetRegs().gpr[reg]);
    case 32:
      return snprintf(buffer, buffer_length, "%08x", debugger->core.CpuGetRegs().cop0.status.raw);
    case 33:
      return snprintf(buffer, buffer_length, "%016lx", debugger->core.CpuGetRegs().lo);
    case 34:
      return snprintf(buffer, buffer_length, "%016lx", debugger->core.CpuGetRegs().hi);
    case 35:
      return snprintf(buffer, buffer_length, "%016lx", debugger->core.CpuGetRegs().cop0.badVaddr);
    case 36:
      return snprintf(buffer, buffer_length, "%08x", debugger->core.CpuGetRegs().cop0.cause.raw);
    case 37:
      //printf("Sending PC: 0x%016lX\n", debugger->core.CpuGetRegs().pc);
      return snprintf(buffer, buffer_length, "%016lx", debugger->core.CpuGetRegs().pc);
    case 38 ... 71: // TODO FPU stuff
      return snprintf(buffer, buffer_length, "%08x", 0);
    default:
      Util::panic("Debug get register %d value\n", reg);
  }
}

size_t debugGetGeneralRegisters(void* user_data, char * buffer, size_t buffer_length) {
  auto* debugger = (Debugger*)user_data;
  printf("The buffer length is %ld!\n", buffer_length);
  size_t printed = 0;
  for (int i = 0; i < 32; i++) {
    int ofs = i * 16; // 64 bit regs take up 16 ascii chars to print in hex
    if (ofs + 16 > buffer_length) {
      Util::panic("Too big!\n");
    }
    u64 reg = debugger->core.CpuGetRegs().gpr[i];
    printed += snprintf(buffer + ofs, buffer_length - ofs, "%016lx", reg);
  }
  return printed;
}

Debugger::Debugger(n64::Core& core) : core(core) {
  gdbstub_config_t config;
  memset(&config, 0, sizeof(gdbstub_config_t));
  config.port                  = 1337;
  config.user_data             = this;
  config.start                 = (gdbstub_start_t) debugStart;
  config.stop                  = (gdbstub_stop_t) debugStop;
  config.step                  = (gdbstub_step_t) debugStep;
  config.set_breakpoint        = (gdbstub_set_breakpoint_t) debugSetBreakpoint;
  config.clear_breakpoint      = (gdbstub_clear_breakpoint_t) debugClearBreakpoint;
  config.get_memory            = (gdbstub_get_memory_t) debugGetMemory;
  config.get_register_value    = (gdbstub_get_register_value_t) debugGetRegisterValue;
  config.get_general_registers = (gdbstub_get_general_registers_t) debugGetGeneralRegisters;

  config.target_config = target_xml;
  config.target_config_length = strlen(target_xml);

  printf("Sizeof target: %ld\n", config.target_config_length);

  config.memory_map = memory_map;
  config.memory_map_length = strlen(memory_map);

  printf("Sizeof memory map: %ld\n", config.memory_map_length);

  gdb = gdbstub_init(config);
  if (!gdb) {
    Util::panic("Failed to initialize GDB stub!\n");
  }
}

Debugger::~Debugger() {
  if(enabled) {
    gdbstub_term(gdb);
  }
}

void Debugger::tick() const {
  gdbstub_tick(gdb);
}

void Debugger::breakpointHit() {
  broken = true;
  gdbstub_breakpoint_hit(gdb);
}
#else
Debugger::Debugger(n64::Core& core) {

}

Debugger::~Debugger() {

}

void Debugger::tick() const {

}

void Debugger::breakpointHit() {

}
#endif