#pragma once
#include <common.hpp>

struct Breakpoint {
  u32 address = 0;
  Breakpoint* next = nullptr;
};

namespace n64 { struct Core; }

struct Debugger {
  Debugger(n64::Core& core) :core(core) {}
  ~Debugger() = default;

  bool broken = false,  enabled = true;
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

  void tick() const {}
  void breakpointHit() {}
};