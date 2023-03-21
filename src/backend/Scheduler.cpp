#include <Scheduler.hpp>
#include <Mem.hpp>
#include <core/registers/Registers.hpp>

Scheduler scheduler;

Scheduler::Scheduler() {
  enqueueAbsolute({UINT64_MAX, [](n64::Mem&, n64::Registers&){
    Util::panic("How the fuck did we get here?!");
  }});
}

void Scheduler::enqueueRelative(const Event& event) {
  enqueueAbsolute({event.time + ticks, event.handler});
}

void Scheduler::enqueueAbsolute(const Event& e) {
  events.push(e);
}

void Scheduler::tick(u64 t, n64::Mem& mem, n64::Registers& regs) {
  ticks += t;
  while(ticks >= events.top().time) {
    events.top().handler(mem, regs);
    events.pop();
  }
}
