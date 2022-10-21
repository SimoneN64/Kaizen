#include <n64/core/mmio/PI.hpp>
#include <util.hpp>
#include <n64/core/Mem.hpp>
#include <n64/core/cpu/Registers.hpp>

namespace n64 {
PI::PI() {
  Reset();
}

void PI::Reset() {
  dramAddr = 0;
  cartAddr = 0;
  rdLen = 0;
  wrLen = 0;
  memset(stub, 0, 8);
}

auto PI::Read(MI& mi, u32 addr) const -> u32 {
  switch(addr) {
    case 0x04600000: return dramAddr;
    case 0x04600004: return cartAddr;
    case 0x04600008: return rdLen;
    case 0x0460000C: return wrLen;
    case 0x04600010: {
      u32 value = 0;
      value |= (0 << 0); // Is PI DMA active? No, because it's instant
      value |= (0 << 1); // Is PI IO busy? No, because it's instant
      value |= (0 << 2); // PI IO error?
      value |= (mi.miIntr.pi << 3); // PI interrupt?
      return value;
    }
    case 0x04600014: case 0x04600018: case 0x0460001C: case 0x04600020:
    case 0x04600024: case 0x04600028: case 0x0460002C: case 0x04600030:
      return stub[(addr & 0xff) - 5];
    default:
      util::panic("Unhandled PI[{:08X}] read\n", addr);
  }
}

void PI::Write(Mem& mem, Registers& regs, u32 addr, u32 val) {
  MI& mi = mem.mmio.mi;
  switch(addr) {
    case 0x04600000: dramAddr = val & 0xFFFFFF; break;
    case 0x04600004: cartAddr = val; break;
    case 0x04600008: {
      u32 len = (val & 0x00FFFFFF) + 1;
      u32 cart_addr = cartAddr & 0xFFFFFFFE;
      u32 dram_addr = dramAddr & 0x007FFFFE;
      if (dram_addr & 0x7) {
        len -= dram_addr & 0x7;
      }
      rdLen = len;
      for(int i = 0; i < len; i++) {
        mem.cart[BYTE_ADDRESS(cart_addr + i) & mem.romMask] = mem.mmio.rdp.dram[BYTE_ADDRESS(dram_addr + i) & RDRAM_DSIZE];
      }
      dramAddr = dram_addr + len;
      cartAddr = cart_addr + len;
      InterruptRaise(mi, regs, Interrupt::PI);
      util::logdebug("PI DMA from RDRAM to CARTRIDGE (size: {} KiB, {:08X} to {:08X})\n", len, dramAddr, cartAddr);
    } break;
    case 0x0460000C: {
      u32 len = (val & 0x00FFFFFF) + 1;
      u32 cart_addr = cartAddr & 0xFFFFFFFE;
      u32 dram_addr = dramAddr & 0x007FFFFE;
      if (dram_addr & 0x7) {
        len -= dram_addr & 0x7;
      }
      wrLen = len;
      for(int i = 0; i < len; i++) {
        mem.mmio.rdp.dram[BYTE_ADDRESS(dram_addr + i) & RDRAM_DSIZE] = mem.cart[BYTE_ADDRESS(cart_addr + i) & mem.romMask];
      }
      dramAddr = dram_addr + len;
      cartAddr = cart_addr + len;
      InterruptRaise(mi, regs, Interrupt::PI);
      util::logdebug("PI DMA from CARTRIDGE to RDRAM (size: {} KiB, {:08X} to {:08X})\n", len, cart_addr, dram_addr);
    } break;
    case 0x04600010:
      if(val & 2) {
        InterruptLower(mi, regs, Interrupt::PI);
      } break;
    case 0x04600014: case 0x04600018: case 0x0460001C: case 0x04600020:
    case 0x04600024: case 0x04600028: case 0x0460002C: case 0x04600030:
      stub[(addr & 0xff) - 5] = val & 0xff;
      break;
    default:
      util::panic("Unhandled PI[{:08X}] write ({:08X})\n", val, addr);
  }
}
}