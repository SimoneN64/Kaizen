#include <core/mmio/AI.hpp>
#include <log.hpp>
#include <core/Mem.hpp>
#include <core/registers/Registers.hpp>
#include <core/Audio.hpp>

namespace n64 {
void AI::Reset() {
  dmaEnable = false;
  dacRate = 0;
  bitrate = 0;
  dmaCount = 0;
  dmaAddrCarry = false;
  cycles = 0;
  memset(dmaLen, 0, 2);
  memset(dmaAddr, 0, 2);
  dac = {44100, N64_CPU_FREQ / dac.freq, 16};
}

auto AI::Read(u32 addr) const -> u32 {
  if(addr == 0x0450000C) {
    u32 val = 0;
    val |= (dmaCount > 1);
    val |= 1 << 20;
    val |= 1 << 24;
    val |= (dmaEnable << 25);
    val |= (dmaCount > 0) << 30;
    val |= (dmaCount > 1) << 31;
    return val;
  }

  return dmaLen[0];
}

void AI::Write(Mem& mem, Registers& regs, u32 addr, u32 val) {
  switch(addr) {
    case 0x04500000:
      if(dmaCount < 2) {
        dmaAddr[dmaCount] = val & 0xFFFFFF & ~7;
      }
      break;
    case 0x04500004: {
      u32 len = (val & 0x3FFFF) & ~7;
      if(dmaCount < 2) {
        if(dmaCount == 0) InterruptRaise(mem.mmio.mi, regs, Interrupt::AI);
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
      dac.freq = std::max(1u, GetVideoFrequency(mem.IsROMPAL()) / (dacRate + 1));
      dac.period = N64_CPU_FREQ / dac.freq;
      if(old_dac_freq != dac.freq) {
        AdjustSampleRate(dac.freq);
      }
    } break;
    case 0x04500014:
      bitrate = val & 0xF;
      dac.precision = bitrate + 1;
      break;
    default:
      Util::panic("Unhandled AI write at addr {:08X} with val {:08X}", addr, val);
  }
}

void AI::Step(Mem& mem, Registers& regs, int cpuCycles, float volumeL, float volumeR) {
  cycles += cpuCycles;
  while(cycles > dac.period) {
    if (dmaCount == 0) {
      return;
    }

    if(dmaLen[0] && dmaEnable) {u32 addrHi = ((dmaAddr[0] >> 13) + dmaAddrCarry) & 0x7FF;
      dmaAddr[0] = (addrHi << 13) | (dmaAddr[0] & 0x1FFF);
      u32 data = Util::ReadAccess<u32>(mem.mmio.rdp.rdram, dmaAddr[0] & RDRAM_DSIZE);
      s16 l = s16(data >> 16);
      s16 r = s16(data);

      if(volumeR > 0 && volumeL > 0) {
        PushSample((float) l / INT16_MAX, volumeL, (float) r / INT16_MAX, volumeR);
      }

      u32 addrLo = (dmaAddr[0] + 4) & 0x1FFF;
      dmaAddr[0] = (dmaAddr[0] & ~0x1FFF) | addrLo;
      dmaAddrCarry = addrLo == 0;
      dmaLen[0] -= 4;
    }

    if(!dmaLen[0]) {
      if(--dmaCount > 0) {
        InterruptRaise(mem.mmio.mi, regs, Interrupt::AI);
        dmaAddr[0] = dmaAddr[1];
        dmaLen[0] = dmaLen[1];
      }
    }

    cycles -= dac.period;
  }
}

}
