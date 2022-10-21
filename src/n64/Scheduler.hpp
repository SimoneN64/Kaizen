#pragma once
#include <common.hpp>
#include <queue>

namespace n64 {
struct Mem;
struct Registers;
}

struct Event {
  u64 time = UINT64_MAX;
  void(*event_cb)(n64::Mem&, n64::Registers&) = nullptr;

  friend bool operator<(const Event& rhs, const Event& lhs) {
    return lhs.time < rhs.time;
  }
};

struct Scheduler {
  Scheduler();
  void enqueueRelative(const Event&);
  void enqueueAbsolute(const Event&);
  void tick(u64, n64::Mem&, n64::Registers&);
  std::priority_queue<Event> events;
  u64 ticks = 0;
};

extern Scheduler scheduler;