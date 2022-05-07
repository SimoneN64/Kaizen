#include <Scheduler.hpp>
#include <algorithm>

namespace natsukashii::core {
Event::Event(u128 time, EventHandler handler, void* ptr) : time(time), handler(handler), ptr(ptr) { }
  
Scheduler::Scheduler() {
  events.emplace_back(Event(UINT128_MAX, [](void*) {
    printf("Reached event at UINT128_MAX, wtf?!\n");
    exit(1);
  }, nullptr));
}

void Scheduler::PushEvent(u128 time, EventHandler handler, void* ptr) {
  for(int i = 0; i < events.size(); i++) {
    if(time < events[i].time) { // if it's lower than the next event we can just insert it.
      events.emplace_back(Event(time, handler, ptr));
      break;
    }
  }
}

void Scheduler::Dispatch() {
  for(auto& event : events) {
    event.handler(event.ptr);
  }
}
}
