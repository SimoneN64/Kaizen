#include <Scheduler.hpp>
#include <algorithm>
#include <Mem.hpp>
#include <Registers.hpp>

namespace n64 {
void Scheduler::enqueue(const Event &event) {
  events.push_back({event.time + ticks, event.func});
}

void Scheduler::handleEvents(u64 tick, Mem &mem, Registers &regs) {
  ticks += tick;
  for (int i = 0; i < events.size(); i++) {
    if (ticks >= events[i].time) {
      events[i].func(mem, regs);
      events.erase(events.begin() + i);
    }
  }
}
}