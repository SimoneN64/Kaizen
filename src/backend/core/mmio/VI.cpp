#include <core/Mem.hpp>
#include <core/mmio/VI.hpp>
#include <core/registers/Registers.hpp>
#include <log.hpp>

namespace n64 {
VI::VI(Mem &mem, Registers &regs) : mem(mem), regs(regs) { Reset(); }

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
  xscale = {}, yscale = {};
  hsyncLeap = {}, burst = {}, vburst = {};
  hstart = {}, vstart = {};
  isPal = false;
  swaps = {};
}

u32 VI::Read(u32 paddr) const {
  switch (paddr) {
  case 0x04400000:
    return status.raw;
  case 0x04400004:
    return origin;
  case 0x04400008:
    return width;
  case 0x0440000C:
    return intr;
  case 0x04400010:
    return current << 1;
  case 0x04400014:
    return burst.raw;
  case 0x04400018:
    return vsync;
  case 0x0440001C:
    return hsync;
  case 0x04400020:
    return hsyncLeap.raw;
  case 0x04400024:
    return hstart.raw;
  case 0x04400028:
    return vstart.raw;
  case 0x0440002C:
    return vburst;
  case 0x04400030:
    return xscale.raw;
  case 0x04400034:
    return yscale.raw;
  default:
    Util::panic("Unimplemented VI[%08X] read", paddr);
  }
}

void VI::Write(u32 paddr, u32 val) {
  switch (paddr) {
  case 0x04400000:
    status.raw = val;
    numFields = status.serrate ? 2 : 1;
    break;
  case 0x04400004:
    {
      u32 masked = val & 0xFFFFFF;
      if (origin != masked) {
        swaps++;
      }
      origin = masked;
    }
    break;
  case 0x04400008:
    width = val & 0x7FF;
    break;
  case 0x0440000C:
    intr = val & 0x3FF;
    break;
  case 0x04400010:
    mem.mmio.mi.InterruptLower(MI::Interrupt::VI);
    break;
  case 0x04400014:
    burst.raw = val;
    break;
  case 0x04400018:
    vsync = val & 0x3FF;
    numHalflines = vsync >> 1;
    cyclesPerHalfline = GetCyclesPerFrame(isPal) / numHalflines;
    break;
  case 0x0440001C:
    hsync = val & 0x3FF;
    break;
  case 0x04400020:
    hsyncLeap.raw = val;
    break;
  case 0x04400024:
    hstart.raw = val;
    break;
  case 0x04400028:
    vstart.raw = val;
    break;
  case 0x0440002C:
    vburst = val;
    break;
  case 0x04400030:
    xscale.raw = val;
    break;
  case 0x04400034:
    yscale.raw = val;
    break;
  default:
    Util::panic("Unimplemented VI[%08X] write (%08X)", paddr, val);
  }
}
} // namespace n64
