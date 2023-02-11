#pragma once
#include <common.hpp>
#include <gdbstub.h>

struct Breakpoint {
  u32 address = 0;
  Breakpoint* next = nullptr;
};

namespace n64 { struct Core; }

struct Debugger {
  Debugger(n64::Core& core);
  ~Debugger();

  bool broken = false,  enabled = true;
#ifndef DISABLE_GDB_STUB
  int steps = 0;
  gdbstub_t* gdb;
  Breakpoint* breakpoints = nullptr;
  n64::Core& core;

  [[nodiscard]] inline bool checkBreakpoint(u32 address) const {
    auto* cur = breakpoints;
    while (cur != nullptr) {
      if (cur->address == address) {
        return true;
      }
      cur = cur->next;
    }
    return false;
  }
#else
  inline bool checkBreakpoint(u32 address) const { return false; }
#endif
  void tick() const;
  void breakpointHit();
};