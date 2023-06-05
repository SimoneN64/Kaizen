#pragma once
#include <common.hpp>
#include <queue>
#include <array>
#include "log.hpp"

namespace n64 {
struct Mem;
struct Registers;
}

struct Event {
  u64 time = 0;
  void(*handler)(n64::Mem&, n64::Registers&) = nullptr;

  friend bool operator<(const Event& rhs, const Event& lhs) {
    return rhs.time < lhs.time;
  }

  friend bool operator>(const Event& rhs, const Event& lhs) {
    return rhs.time > lhs.time;
  }

  friend bool operator>=(const Event& rhs, const Event& lhs) {
    return rhs.time >= lhs.time;
  }
};

struct Scheduler {
  Scheduler() {
    enqueueAbsolute(Event{std::numeric_limits<u64>::max(), [](n64::Mem&, n64::Registers&) {
      Util::panic("How the fuck did we get here?!");
    }});
  }
  void enqueueRelative(const Event&);
  void enqueueAbsolute(const Event&);
  void tick(u64, n64::Mem&, n64::Registers&);
  std::priority_queue<Event, std::vector<Event>, std::greater<>> events;
  u64 ticks = 0;
  u8 index = 0;
};

extern Scheduler scheduler;