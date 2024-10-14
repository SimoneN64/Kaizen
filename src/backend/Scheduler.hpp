#pragma once
#include <functional>
#include <log.hpp>
#include <queue>

namespace n64 {
struct Mem;
struct Registers;
} // namespace n64

enum EventType { NONE, PI_BUS_WRITE_COMPLETE, PI_DMA_COMPLETE, SI_DMA, IMPOSSIBLE };

struct Event {
  u64 time;
  EventType type;

  friend bool operator<(const Event &rhs, const Event &lhs) { return rhs.time < lhs.time; }

  friend bool operator>(const Event &rhs, const Event &lhs) { return rhs.time > lhs.time; }

  friend bool operator>=(const Event &rhs, const Event &lhs) { return rhs.time >= lhs.time; }
};

struct IterableEvents {
  std::priority_queue<Event, std::vector<Event>, std::greater<>> events;

  explicit IterableEvents() = default;
  [[nodiscard]] auto top() const { return events.top(); }
  auto pop() { events.pop(); }
  [[nodiscard]] auto begin() const { return const_cast<Event *>(&events.top()); }
  [[nodiscard]] auto end() const { return begin() + events.size(); }
  auto push(const Event e) { events.push(e); }
};

struct Scheduler {
  Scheduler() { EnqueueAbsolute(std::numeric_limits<u64>::max(), IMPOSSIBLE); }

  void EnqueueRelative(u64, EventType);
  void EnqueueAbsolute(u64, EventType);
  [[nodiscard]] u64 Remove(EventType) const;
  void Tick(u64 t, n64::Mem &);

  IterableEvents events{};
  u64 ticks = 0;
  u8 index = 0;
};

inline Scheduler scheduler;
