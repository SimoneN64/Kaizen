#pragma once
#include <common.hpp>
#include <vector>

namespace natsukashii::core {
using EventHandler = void(*)(void*);
struct Event {
  Event(u128, EventHandler, void*);
private:
  friend class Scheduler;
  u128 time;
  void* ptr;
  EventHandler handler;
};

struct Scheduler {
  Scheduler();
  void PushEvent(u128, EventHandler, void*);
  void Dispatch();
private:
  u128 currentTime = 0;
  std::vector<Event> events{};
};
}
