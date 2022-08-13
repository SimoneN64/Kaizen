#include <n64/core/mmio/VI.hpp>
#include <util.hpp>
#include <n64/core/cpu/Registers.hpp>
#include <n64/core/mmio/MI.hpp>
#include <n64/core/mmio/Interrupt.hpp>

namespace n64 {
VI::VI () {
  Reset();
}

void VI::Reset() {
  status.raw = 0xF;
  intr = 256;
  origin = 0;
  width = 320;
  current = 0;
  vsync = 0;
  hsync = 0;
  numHalflines = 262;
  numFields = 1;
  cyclesPerHalfline = 1000;
}

u32 VI::Read(u32 paddr) const {
  switch(paddr) {
    case 0x04400000: return status.raw;
    case 0x04400004: return origin;
    case 0x04400008: return width;
    case 0x0440000C: return intr;
    case 0x04400010: return current << 1;
    case 0x04400014: return burst.raw;
    case 0x04400018: return vsync;
    case 0x0440001C: return hsync;
    case 0x04400020: return hsyncLeap.raw;
    case 0x04400024: return hvideo.raw;
    case 0x04400028: return vvideo.raw;
    case 0x0440002C: return vburst;
    case 0x04400030: return xscale.raw;
    case 0x04400034: return yscale.raw;
    default:
      util::panic("Unimplemented VI[%08X] read\n", paddr);
  }
  return 0;
}

void VI::Write(MI& mi, Registers& regs, u32 paddr, u32 val) {
  switch(paddr) {
    case 0x04400000:
      status.raw = val;
      numFields = status.serrate ? 2 : 1;
      break;
    case 0x04400004: {
      u32 masked = val & 0xFFFFFF;
      if(origin != masked) {
        swaps++;
      }
      origin = masked;
    } break;
    case 0x04400008: {
      width = val & 0x7FF;
    } break;
    case 0x0440000C: {
      intr = val & 0x3FF;
    } break;
    case 0x04400010:
      InterruptLower(mi, regs, Interrupt::VI);
      break;
    case 0x04400014: burst.raw = val; break;
    case 0x04400018: {
      vsync = val & 0x3FF;
      numHalflines = vsync >> 1;
      cyclesPerHalfline = N64_CYCLES_PER_FRAME / numHalflines;
    } break;
    case 0x0440001C: {
      hsync = val & 0x3FF;
    } break;
    case 0x04400020: hsyncLeap.raw = val; break;
    case 0x04400024: hvideo.raw = val; break;
    case 0x04400028: vvideo.raw = val; break;
    case 0x0440002C: vburst = val; break;
    case 0x04400030: xscale.raw = val; break;
    case 0x04400034: yscale.raw = val; break;
    default:
      util::panic("Unimplemented VI[%08X] write (%08X)\n", paddr, val);
  }
}
}