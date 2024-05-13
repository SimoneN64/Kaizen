#include <Scheduler.hpp>
#include <core/registers/Registers.hpp>
#include <core/Mem.hpp>

Scheduler scheduler;

void Scheduler::EnqueueRelative(u64 t, const EventType type) {
  EnqueueAbsolute(t + ticks, type);
}

void Scheduler::EnqueueAbsolute(u64 t, const EventType type) {
  events.push({t, type});
}

u64 Scheduler::Remove(EventType type) {
  for (auto& e : events) {
    if(e.type == type) {
      u64 ret = e.time - ticks;
      e.type = NONE;
      e.time = ticks;
      return ret;
    }
  }

  return 0;
}

void Scheduler::Tick(u64 t, n64::Mem& mem) {
  ticks += t;
  n64::MI& mi = mem.mmio.mi;
  n64::SI& si = mem.mmio.si;
  n64::PI& pi = mem.mmio.pi;

  while(ticks >= events.top().time) {
    switch(auto type = events.top().type) {
      case SI_DMA:
        si.DMA();
        break;
      case PI_DMA_COMPLETE:
        mi.InterruptRaise(n64::MI::Interrupt::PI);
        pi.dmaBusy = false;
        break;
      case PI_BUS_WRITE_COMPLETE:
        pi.ioBusy = false;
        break;
      case NONE:
        break;
      case IMPOSSIBLE:
        Util::panic("Congratulations on keeping the emulator on for about 5 billion years, I guess, nerd.");
      default:
        Util::panic("Unknown scheduler event type {}", static_cast<int>(type));
    }
    events.pop();
  }
}