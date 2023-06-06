#include <core/mmio/PI.hpp>
#include <log.hpp>
#include <core/Mem.hpp>
#include <core/registers/Registers.hpp>
#include "Scheduler.hpp"

namespace n64 {
PI::PI() {
  Reset();
}

void PI::Reset() {
  dramAddr = 0;
  cartAddr = 0;
  rdLen = 0;
  wrLen = 0;
}

auto PI::Read(MI& mi, u32 addr) const -> u32 {
  switch(addr) {
    case 0x04600000: return dramAddr;
    case 0x04600004: return cartAddr;
    case 0x04600008: return rdLen;
    case 0x0460000C: return wrLen;
    case 0x04600010: {
      u32 value = 0;
      value |= (0 << dmaBusy); // Is PI DMA active? No, because it's instant
      value |= (0 << ioBusy); // Is PI IO busy? No, because it's instant
      value |= (0 << 2); // PI IO error?
      value |= (mi.miIntr.pi << 3); // PI interrupt?
      return value;
    }
    case 0x04600014: return pi_bsd_dom1_lat;
    case 0x04600018: return pi_bsd_dom1_pwd;
    case 0x0460001C: return pi_bsd_dom1_pgs;
    case 0x04600020: return pi_bsd_dom1_rls;
    case 0x04600024: return pi_bsd_dom2_lat;
    case 0x04600028: return pi_bsd_dom2_pwd;
    case 0x0460002C: return pi_bsd_dom2_pgs;
    case 0x04600030: return pi_bsd_dom2_rls;
    default:
      Util::panic("Unhandled PI[{:08X}] read", addr);
  }
}

FORCE_INLINE u8 PIGetDomain(u32 address) {
  switch (address) {
    case CART_REGION_1_1:
    case CART_REGION_1_2:
      return 1;
    case CART_REGION_2_1:
    case CART_REGION_2_2:
      return 2;
    default:
      Util::panic("Unknown PI domain for address {:08X}!", address);
  }
}

FORCE_INLINE u32 PIAccessTiming(PI& pi, u8 domain, u32 length) {
  uint32_t cycles = 0;
  uint32_t latency = 0;
  uint32_t pulse_width = 0;
  uint32_t release = 0;
  uint32_t page_size = 0;
  uint32_t pages;

  switch (domain) {
    case 1:
      latency = pi.pi_bsd_dom1_lat + 1;
      pulse_width = pi.pi_bsd_dom1_pwd + 1;
      release = pi.pi_bsd_dom1_rls + 1;
      page_size = std::pow(2, (pi.pi_bsd_dom1_pgs + 2));
      break;
    case 2:
      latency = pi.pi_bsd_dom2_lat + 1;
      pulse_width = pi.pi_bsd_dom2_pwd + 1;
      release = pi.pi_bsd_dom2_rls + 1;
      page_size = std::pow(2, (pi.pi_bsd_dom2_pgs + 2));
      break;
    default:
      Util::panic("Unknown PI domain: %d\n", domain);
  }

  pages = ceil((double)length / page_size);

  cycles += (14 + latency) * pages;
  cycles += (pulse_width + release) * (length / 2);
  cycles += 5 * pages;
  return cycles * 1.5; // Converting RCP clock speed to CPU clock speed
}

template <bool toCart>
FORCE_INLINE void OnDMAComplete(Mem& mem, Registers& regs) {
  PI& pi = mem.mmio.pi;
  u32 len;
  if constexpr (toCart) {
    len = pi.rdLen;
  } else {
    len = pi.wrLen;
  }

  pi.dramAddr = pi.dramAddrInternal + len;
  pi.cartAddr = pi.cartAddrInternal + len;
  pi.dmaBusy = false;
  pi.ioBusy = false;
  InterruptRaise(mem.mmio.mi, regs, Interrupt::PI);
}

void PI::Write(Mem& mem, Registers& regs, u32 addr, u32 val) {
  MI& mi = mem.mmio.mi;
  switch(addr) {
    case 0x04600000: dramAddr = val & 0xFFFFFF; break;
    case 0x04600004: cartAddr = val; break;
    case 0x04600008: {
      u32 len = (val & 0x00FFFFFF) + 1;
      cartAddrInternal = cartAddr & 0xFFFFFFFE;
      dramAddrInternal = dramAddr & 0x007FFFFE;
      if (dramAddrInternal & 0x7) {
        len -= dramAddrInternal & 0x7;
      }
      rdLen = len;
      for (int i = 0; i < rdLen; i++) {
        mem.rom.cart[BYTE_ADDRESS(cartAddrInternal + i) & mem.rom.mask] = mem.mmio.rdp.rdram[BYTE_ADDRESS(dramAddrInternal + i) & RDRAM_DSIZE];
      }
      Util::debug("PI DMA from RDRAM to CARTRIDGE (size: {} B, {:08X} to {:08X})", len, dramAddr, cartAddr);
      dmaBusy = true;
      ioBusy = true;
      scheduler.enqueueRelative(Event{PIAccessTiming(*this, PIGetDomain(cartAddr), len), OnDMAComplete<true>});
    } break;
    case 0x0460000C: {
      u32 len = (val & 0x00FFFFFF) + 1;
      cartAddrInternal = cartAddr & 0xFFFFFFFE;
      dramAddrInternal = dramAddr & 0x007FFFFE;
      if (dramAddrInternal & 0x7) {
        len -= (dramAddrInternal & 0x7);
      }
      wrLen = len;
      for(int i = 0; i < wrLen; i++) {
        mem.mmio.rdp.rdram[BYTE_ADDRESS(dramAddrInternal + i) & RDRAM_DSIZE] = mem.rom.cart[BYTE_ADDRESS(cartAddrInternal + i) & mem.rom.mask];
      }
      dmaBusy = true;
      ioBusy = true;
      Util::debug("PI DMA from CARTRIDGE to RDRAM (size: {} B, {:08X} to {:08X})", len, cartAddr, dramAddr);
      scheduler.enqueueRelative(Event{PIAccessTiming(*this, PIGetDomain(cartAddr), len), OnDMAComplete<false>});
    } break;
    case 0x04600010:
      if(val & 2) {
        InterruptLower(mi, regs, Interrupt::PI);
      } break;
    case 0x04600014: pi_bsd_dom1_lat = val & 0xff; break;
    case 0x04600018: pi_bsd_dom1_pwd = val & 0xff; break;
    case 0x0460001C: pi_bsd_dom1_pgs = val & 0xff; break;
    case 0x04600020: pi_bsd_dom1_rls = val & 0xff; break;
    case 0x04600024: pi_bsd_dom2_lat = val & 0xff; break;
    case 0x04600028: pi_bsd_dom2_pwd = val & 0xff; break;
    case 0x0460002C: pi_bsd_dom2_pgs = val & 0xff; break;
    case 0x04600030: pi_bsd_dom2_rls = val & 0xff; break;
    default:
      Util::panic("Unhandled PI[{:08X}] write ({:08X})", val, addr);
  }
}
}