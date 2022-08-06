#include <n64/core/mmio/AI.hpp>
#include <util.hpp>
#include <n64/core/Mem.hpp>
#include <n64/core/cpu/Registers.hpp>
#include <n64/core/Audio.hpp>

namespace n64 {
auto AI::Read(u32 addr) const -> u32 {
  switch(addr) {
    case 0x04500004: return dmaLen[0];
    case 0x0450000C: {
      u32 val = 0;
      val |= (dmaCount > 1);
      val |= 1 << 20;
      val |= 1 << 24;
      val |= (dmaCount > 0) << 30;
      val |= (dmaCount > 1) << 31;
      return val;
    }
    default: util::panic("Unhandled AI read at addr {:08X}\n", addr);
  }
  return 0;
}

#define max(x, y) ((x) > (y) ? (x) : (y))

using namespace natsukashii::core;

void AI::Write(Mem& mem, Registers& regs, u32 addr, u32 val) {
  switch(addr) {
    case 0x04500000:
      if(dmaCount < 2) {
        dmaAddr[dmaCount] = val & 0xFFFFFF & ~7;
      }
      break;
    case 0x04500004: {
      u32 len = (val & 0x3FFFF) & ~7;
      if((dmaCount < 2) && len) {
        dmaLen[dmaCount] = len;
        dmaCount++;
      }
    } break;
    case 0x04500008:
      dmaEnable = val & 1;
      break;
    case 0x0450000C:
      InterruptLower(mem.mmio.mi, regs, Interrupt::AI);
      break;
    case 0x04500010: {
      u32 old_dac_freq = dac.freq;
      dacRate = val & 0x3FFF;
      dac.freq = max(1, N64_CPU_FREQ / 2 / (dacRate + 1)) * 1.037;
      dac.period = N64_CPU_FREQ / dac.freq;
      if(old_dac_freq != dac.freq) {
        AdjustSampleRate(dac.freq);
      }
    } break;
    case 0x04500014:
      bitrate = val & 0xF;
      break;
    default: util::panic("Unhandled AI write at addr {:08X} with val {:08X}\n", addr, val);
  }
}

void AI::Step(Mem& mem, Registers& regs, int) {
  cycles += cycles;
  while(cycles > dac.period) {
    if (dmaCount == 0) {
      return;
    }

    u32 address_hi = ((dmaAddr[0] >> 13) + dmaAddrCarry) & 0x7ff;
    dmaAddr[0] = (address_hi << 13) | dmaAddr[0] & 0x1fff;
    u32 data = mem.Read<u32, false>(regs, dmaAddr[0], regs.pc);

    s16 left  = (s16)(data >> 16);
    s16 right = (s16)data;
    PushSample(left, right);

    u32 address_lo = (dmaAddr[0] + 4) & 0x1fff;
    dmaAddr[0] = (dmaAddr[0] & ~0x1fff) | address_lo;
    dmaAddrCarry = (address_lo == 0);
    dmaLen[0] -= 4;

    if(!dmaLen[0]) {
      InterruptRaise(mem.mmio.mi, regs, Interrupt::AI);
      if(--dmaCount > 0) { // If we have another DMA pending, start on that one.
        dmaAddr[0] = dmaAddr[1];
        dmaLen[0]  = dmaLen[1];
      }
    }

    cycles -= dac.period;
  }
}

}
