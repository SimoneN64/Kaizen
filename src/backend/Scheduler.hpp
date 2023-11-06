#pragma once
#include <queue>
#include <array>
#include <functional>
#include <log.hpp>

namespace n64 {
struct Mem;
struct Registers;
}

enum EventType {
  NONE,
  PI_BUS_WRITE_COMPLETE,
  PI_DMA_COMPLETE,
  SI_DMA,
  IMPOSSIBLE
};

struct Event {
  u64 time;
  EventType type;

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
    enqueueAbsolute(std::numeric_limits<u64>::max(), IMPOSSIBLE);
  }

  void enqueueRelative(u64, const EventType);
  void enqueueAbsolute(u64, const EventType);
  u64 remove(const EventType);
  void tick(u64 t, n64::Mem&, n64::Registers&);
  
  std::priority_queue<Event, std::vector<Event>, std::greater<>> events;
  u64 ticks = 0;
  u8 index = 0;
};

extern Scheduler scheduler;