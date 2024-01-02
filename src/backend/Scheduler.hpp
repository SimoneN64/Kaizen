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

struct IterableEvents {
  std::priority_queue<Event, std::vector<Event>, std::greater<>> events;
public:
  explicit IterableEvents() = default;
  auto top() { return events.top(); }
  auto pop() { events.pop(); }
  auto begin() { return (Event*)(&events.top()); }
  auto end() { return begin() + events.size(); }
  auto push(Event e) { events.push(e); }
};

struct Scheduler {
  Scheduler() {
    enqueueAbsolute(std::numeric_limits<u64>::max(), IMPOSSIBLE);
  }

  void enqueueRelative(u64, EventType);
  void enqueueAbsolute(u64, EventType);
  u64 remove(EventType);
  void tick(u64 t, n64::Mem&, n64::Registers&);

  IterableEvents events;
  u64 ticks = 0;
  u8 index = 0;
};

extern Scheduler scheduler;