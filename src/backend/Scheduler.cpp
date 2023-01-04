#include <Scheduler.hpp>
#include <Mem.hpp>
#include <core/registers/Registers.hpp>

Scheduler scheduler;

Scheduler::Scheduler() {
  events.push({UINT64_MAX, [](n64::Mem&, n64::Registers&){
    Util::panic("How the fuck did we get here?!\n");
  }});
}

void Scheduler::enqueueRelative(const Event& event) {
  events.push({event.time + ticks, event.event_cb});
}

void Scheduler::enqueueAbsolute(const Event& event) {
  events.push({event.time, event.event_cb});
}

void Scheduler::tick(u64 t, n64::Mem& mem, n64::Registers& regs) {
  ticks += t;
  while(ticks >= events.top().time) {
    events.top().event_cb(mem, regs);
    events.pop();
  }
}
