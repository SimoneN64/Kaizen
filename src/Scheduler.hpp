#pragma once
#include <vector>
#include <common.hpp>

namespace n64 {
struct Mem;
struct Registers;

struct Event {
  u64 time;
  void (*func)(Mem&, Registers& regs);
};

enum {
  SI_DMA_COMPLETE
};

struct Scheduler {
  void enqueue(const Event&);
  void handleEvents(u64 tick, Mem& mem, Registers& regs);
  u64 ticks = 0;
  std::vector<Event> events;
};
}
