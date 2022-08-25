#include <Core.hpp>
#include <debugger.hpp>

void DebugStart(void* user_data) {
  auto* core = (n64::Core*)user_data;
  core->debuggerState.broken = false;
}

void DebugStop(void* user_data) {
  auto* core = (n64::Core*)user_data;
  core->debuggerState.broken = true;
}

void DebugStep(void* user_data) {
  auto* core = (n64::Core*)user_data;
  bool old_broken = core->debuggerState.broken;
  core->debuggerState.broken = false;
  core->Step();
  core->debuggerState.broken = old_broken;
  core->debuggerState.steps += 2;
}

void DebugSetBreakpoint(void* user_data, u32 address) {
  auto* core = (n64::Core*)user_data;
  auto* breakpoint = (Breakpoint*)malloc(sizeof(Breakpoint));
  breakpoint->addr = address;
  breakpoint->next = NULL;

  // Special case for this being the first breakpoint
  if (core->debuggerState.breakpoints == NULL) {
    core->debuggerState.breakpoints = breakpoint;
  } else {
    // Find end of the list
    Breakpoint* tail = core->debuggerState.breakpoints;
    while (tail->next != NULL) {
      tail = tail->next;
    }

    tail->next = breakpoint;
  }
}

void DebugClearBreakpoint(void* user_data, u32 address) {
  auto* core = (n64::Core*)user_data;
  if (core->debuggerState.breakpoints == NULL) {
    return; // No breakpoints set at all
  } else if (core->debuggerState.breakpoints->addr == address) {
    // Special case for the first breakpoint being the one we want to clear
    Breakpoint* next = core->debuggerState.breakpoints->next;
    free(core->debuggerState.breakpoints);
    core->debuggerState.breakpoints = next;
  } else {
    // Find the breakpoint somewhere in the list and free it
    Breakpoint* iter = core->debuggerState.breakpoints;
    while (iter->next != NULL) {
      if (iter->next->addr == address) {
        Breakpoint* next = iter->next->next;
        free(iter->next);
        iter->next = next;
      }
    }
  }
}

ssize_t DebugGetMemory(void* user_data, char* buffer, size_t length, u32 address, size_t bytes) {
  auto* core = (n64::Core*)user_data;
  printf("Checking memory at address 0x%08X\n", address);
  int printed = 0;
  for (int i = 0; i < bytes; i++) {
    u8 value = core->mem.Read<u8>(core->cpu.regs, address + i, core->cpu.regs.pc);
    printed += snprintf(buffer + (i*2), length, "%02X", value);
  }
  printf("Get memory: %ld bytes from 0x%08X: %d\n", bytes, address, printed);
  return printed + 1;
}

ssize_t DebugGetRegisterValue(void* user_data, char * buffer, size_t buffer_length, int reg) {
  auto* core = (n64::Core*)user_data;
  switch (reg) {
    case 0 ... 31:
      return snprintf(buffer, buffer_length, "%016llx", core->cpu.regs.gpr[reg]);
    case 32:
      return snprintf(buffer, buffer_length, "%08x", core->cpu.regs.cop0.status.raw);
    case 33:
      return snprintf(buffer, buffer_length, "%016llx", core->cpu.regs.lo);
    case 34:
      return snprintf(buffer, buffer_length, "%016llx", core->cpu.regs.hi);
    case 35:
      return snprintf(buffer, buffer_length, "%016llx", core->cpu.regs.cop0.badVaddr);
    case 36:
      return snprintf(buffer, buffer_length, "%08x", core->cpu.regs.cop0.cause.raw);
    case 37:
      printf("Sending PC: 0x%016llX\n", core->cpu.regs.pc);
      return snprintf(buffer, buffer_length, "%016llx", core->cpu.regs.pc);
    case 38 ... 71: // TODO FPU stuff
      return snprintf(buffer, buffer_length, "%08x", 0);
    default:
      util::panic("debug get register {} value", reg);
  }
}

ssize_t DebugGetGeneralRegisters(void* user_data, char * buffer, size_t buffer_length) {
  auto* core = (n64::Core*)user_data;
  printf("The buffer length is %zu!\n", buffer_length);
  ssize_t printed = 0;
  for (int i = 0; i < 32; i++) {
    int ofs = i * 16; // 64 bit regs take up 16 ascii chars to print in hex
    if (ofs + 16 > buffer_length) {
      util::panic("Too big!");
    }
    u64 reg = core->cpu.regs.gpr[i];
    printed += snprintf(buffer + ofs, buffer_length - ofs, "%016llx", reg);
  }
  return printed;
}