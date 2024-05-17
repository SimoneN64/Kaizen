#include <core/mmio/PI.hpp>
#include <log.hpp>
#include <Core.hpp>
#include <Scheduler.hpp>

namespace n64 {
PI::PI(Mem& mem, Registers& regs) : mem(mem), regs(regs) {
  Reset();
}

void PI::Reset() {
  dmaBusy = false;
  ioBusy = false;
  latch = 0;
  dramAddr = 0;
  cartAddr = 0;
  dramAddrInternal = 0;
  cartAddrInternal = 0;
  rdLen = 0;
  wrLen = 0;
  piBsdDom1Lat = 0;
  piBsdDom2Lat = 0;
  piBsdDom1Pwd = 0;
  piBsdDom2Pwd = 0;
  piBsdDom1Pgs = 0;
  piBsdDom2Pgs = 0;
  piBsdDom1Rls = 0;
  piBsdDom2Rls = 0;
}

bool PI::WriteLatch(u32 value) {
  if (ioBusy) {
    return false;
  } else {
    ioBusy = true;
    latch = value;
    scheduler.EnqueueRelative(100, PI_BUS_WRITE_COMPLETE);
    return true;
  }
}

bool PI::ReadLatch() {
  if (ioBusy) [[unlikely]] {
    ioBusy = false;
    regs.CpuStall(scheduler.Remove(PI_BUS_WRITE_COMPLETE));
    return false;
  }
  return true;
}

template<> auto PI::BusRead<u8, true>(u32 addr) -> u8 {
  switch (addr) {
    case REGION_PI_UNKNOWN:
      Util::panic("Reading byte from address 0x{:08X} in unsupported region: REGION_PI_UNKNOWN - This is the N64DD, returning FF because it is not emulated", addr);
    case REGION_PI_64DD_REG:
      Util::panic("Reading byte from address 0x{:08X} in unsupported region: REGION_PI_64DD_REG - This is the N64DD, returning FF because it is not emulated", addr);
    case REGION_PI_64DD_ROM:
      Util::warn("Reading byte from address 0x{:08X} in unsupported region: REGION_PI_64DD_ROM - This is the N64DD, returning FF because it is not emulated", addr);
      return 0xFF;
    case REGION_PI_SRAM:
      return mem.BackupRead<u8>(addr - SREGION_PI_SRAM);
    case REGION_PI_ROM: {
      // round to nearest 4 byte boundary, keeping old LSB
      u32 index = BYTE_ADDRESS(addr) - SREGION_PI_ROM;
      if (index >= mem.rom.cart.size()) {
        Util::warn("Address 0x{:08X} accessed an index {}/0x{:X} outside the bounds of the ROM! ({}/0x{:016X})", addr, index, index, mem.rom.cart.size(), mem.rom.cart.size());
        return 0xFF;
      }
      return mem.rom.cart[index];
    }
    default:
      Util::panic("Should never end up here! Access to address {:08X} which did not match any PI bus regions!", addr);
  }
}

template<> auto PI::BusRead<u8, false>(u32 addr) -> u8 {
  if (!ReadLatch()) [[unlikely]] {
    return latch >> 24;
  }

  switch (addr) {
    case REGION_PI_UNKNOWN:
      Util::panic("Reading byte from address 0x{:08X} in unsupported region: REGION_PI_UNKNOWN - This is the N64DD, returning FF because it is not emulated", addr);
    case REGION_PI_64DD_REG:
      Util::panic("Reading byte from address 0x{:08X} in unsupported region: REGION_PI_64DD_REG - This is the N64DD, returning FF because it is not emulated", addr);
    case REGION_PI_64DD_ROM:
      Util::warn("Reading byte from address 0x{:08X} in unsupported region: REGION_PI_64DD_ROM - This is the N64DD, returning FF because it is not emulated", addr);
      return 0xFF;
    case REGION_PI_SRAM:
      return mem.BackupRead<u8>(addr - SREGION_PI_SRAM);
    case REGION_PI_ROM: {
      addr = (addr + 2) & ~2;
      // round to nearest 4 byte boundary, keeping old LSB
      u32 index = BYTE_ADDRESS(addr) - SREGION_PI_ROM;
      if (index >= mem.rom.cart.size()) {
        Util::warn("Address 0x{:08X} accessed an index {}/0x{:X} outside the bounds of the ROM! ({}/0x{:016X})", addr, index, index, mem.rom.cart.size(), mem.rom.cart.size());
        return 0xFF;
      }
      return mem.rom.cart[index];
    }
    default:
      Util::panic("Should never end up here! Access to address {:08X} which did not match any PI bus regions!", addr);
  }
}

template<> void PI::BusWrite<u8, true>(u32 addr, u32 val) {
  switch (addr) {
    case REGION_PI_UNKNOWN:
      Util::panic("Writing byte 0x{:02X} to address 0x{:08X} in unsupported region: REGION_PI_UNKNOWN", val, addr);
    case REGION_PI_64DD_REG:
      if (addr == 0x05000020) {
        fprintf(stderr, "%c", val);
      } else {
        Util::warn("Writing byte 0x{:02X} to address 0x{:08X} in region: REGION_PI_64DD_ROM, this is the 64DD, ignoring!", val, addr);
      }
      break;
    case REGION_PI_64DD_ROM:
      Util::panic("Writing byte 0x{:02X} to address 0x{:08X} in unsupported region: REGION_PI_64DD_ROM", val, addr);
    case REGION_PI_SRAM:
      mem.BackupWrite<u8>(addr - SREGION_PI_SRAM, val);
      break;
    case REGION_PI_ROM:
      Util::warn("Writing byte 0x{:02X} to address 0x{:08X} in unsupported region: REGION_PI_ROM", val, addr);
      break;
    default:
      Util::panic("Should never end up here! Access to address {:08X} which did not match any PI bus regions!", addr);
  }
}

template<> void PI::BusWrite<u8, false>(u32 addr, u32 val) {
  u8 latch_shift = 24 - (addr & 1) * 8;

  if (!WriteLatch(val << latch_shift) && addr != 0x05000020) [[unlikely]] {
    return;
  }

  BusWrite<u8, true>(addr, val);
}

template <> auto PI::BusRead<u16, false>(u32 addr) -> u16 {
  if (!ReadLatch()) [[unlikely]] {
    return latch >> 16;
  }

  switch (addr) {
    case REGION_PI_UNKNOWN:
      Util::panic("Reading half from address 0x{:08X} in unsupported region: REGION_PI_UNKNOWN - This is the N64DD, returning FF because it is not emulated", addr);
    case REGION_PI_64DD_REG:
      Util::panic("Reading half from address 0x{:08X} in unsupported region: REGION_PI_64DD_REG - This is the N64DD, returning FF because it is not emulated", addr);
    case REGION_PI_64DD_ROM:
      Util::panic("Reading half from address 0x{:08X} in unsupported region: REGION_PI_64DD_ROM - This is the N64DD, returning FF because it is not emulated", addr);
    case REGION_PI_SRAM:
      Util::panic("Reading half from address 0x{:08X} in unsupported region: REGION_PI_SRAM", addr);
    case REGION_PI_ROM: {
      addr = (addr + 2) & ~3;
      u32 index = HALF_ADDRESS(addr) - SREGION_PI_ROM;
      if (index > mem.rom.cart.size() - 1) {
        Util::panic("Address 0x{:08X} accessed an index {}/0x{:X} outside the bounds of the ROM!", addr, index, index);
      }
      return Util::ReadAccess<u16>(mem.rom.cart, index);
    }
    default:
      Util::panic("Should never end up here! Access to address {:08X} which did not match any PI bus regions!", addr);
  }
}

template <> auto PI::BusRead<u16, true>(u32 addr) -> u16 {
  return BusRead<u16, false>(addr);
}

template <> void PI::BusWrite<u16, false>(u32 addr, u32 val) {
  if (!WriteLatch(val << 16)) [[unlikely]] {
    return;
  }

  switch (addr) {
    case REGION_PI_UNKNOWN:
      Util::panic("Writing half 0x{:04X} to address 0x{:08X} in unsupported region: REGION_PI_UNKNOWN", val, addr);
    case REGION_PI_64DD_REG:
      Util::panic("Writing half 0x{:04X} to address 0x{:08X} in region: REGION_PI_64DD_ROM, this is the 64DD, ignoring!", val, addr);
    case REGION_PI_64DD_ROM:
      Util::panic("Writing half 0x{:04X} to address 0x{:08X} in unsupported region: REGION_PI_64DD_ROM", val, addr);
    case REGION_PI_SRAM:
      Util::panic("Writing half 0x{:04X} to address 0x{:08X} in unsupported region: REGION_PI_SRAM", val, addr);
    case REGION_PI_ROM:
      Util::warn("Writing half 0x{:04X} to address 0x{:08X} in unsupported region: REGION_PI_ROM", val, addr);
      break;
    default:
      Util::panic("Should never end up here! Access to address {:08X} which did not match any PI bus regions!", addr);
  }
}

template <> void PI::BusWrite<u16, true>(u32 addr, u32 val) {
  BusWrite<u16, false>(addr, val);
}

template <> auto PI::BusRead<u32, false>(u32 addr) -> u32 {
  if (!ReadLatch()) [[unlikely]] {
    return latch;
  }

  switch (addr) {
    case REGION_PI_UNKNOWN:
      Util::warn("Reading word from address 0x{:08X} in unsupported region: REGION_PI_UNKNOWN - This is the N64DD, returning FF because it is not emulated", addr);
      return 0xFF;
    case REGION_PI_64DD_REG:
      Util::warn("Reading word from address 0x{:08X} in unsupported region: REGION_PI_64DD_REG - This is the N64DD, returning FF because it is not emulated", addr);
      return 0xFF;
    case REGION_PI_64DD_ROM:
      Util::warn("Reading word from address 0x{:08X} in unsupported region: REGION_PI_64DD_ROM - This is the N64DD, returning FF because it is not emulated", addr);
      return 0xFF;
    case REGION_PI_SRAM:
      return mem.BackupRead<u32>(addr);
    case REGION_PI_ROM: {
      u32 index = addr - SREGION_PI_ROM;
      if (index > mem.rom.cart.size() - 3) { // -3 because we're reading an entire word
        switch (addr) {
          case REGION_CART_ISVIEWER_BUFFER:
            return htobe32(Util::ReadAccess<u32>(mem.isviewer, addr - SREGION_CART_ISVIEWER_BUFFER));
          case CART_ISVIEWER_FLUSH:
            Util::panic("Read from ISViewer flush!");
          default: break;
        }
        Util::warn("Address 0x{:08X} accessed an index {}/0x{:X} outside the bounds of the ROM!", addr, index, index);
        return 0;
      } else {
        return Util::ReadAccess<u32>(mem.rom.cart, index);
      }
    }
    default:
      Util::panic("Should never end up here! Access to address {:08X} which did not match any PI bus regions!", addr);
  }
}

template <> auto PI::BusRead<u32, true>(u32 addr) -> u32 {
  return BusRead<u32, false>(addr);
}

template <> void PI::BusWrite<u32, false>(u32 addr, u32 val) {
  switch (addr) {
    case REGION_PI_UNKNOWN:
      if (!WriteLatch(val)) [[unlikely]] {
        return;
      }
      Util::warn("Writing word 0x{:08X} to address 0x{:08X} in unsupported region: REGION_PI_UNKNOWN", val, addr);
      return;
    case REGION_PI_64DD_REG:
      if (!WriteLatch(val)) [[unlikely]] {
        return;
      }
      Util::warn("Writing word 0x{:08X} to address 0x{:08X} in region: REGION_PI_64DD_ROM, this is the 64DD, ignoring!", val, addr);
      return;
    case REGION_PI_64DD_ROM:
      if (!WriteLatch(val)) [[unlikely]] {
        return;
      }
      Util::warn("Writing word 0x{:08X} to address 0x{:08X} in unsupported region: REGION_PI_64DD_ROM", val, addr);
      return;
    case REGION_PI_SRAM:
      if (!WriteLatch(val)) [[unlikely]] {
        return;
      }
      mem.BackupWrite<u32>(addr - SREGION_PI_SRAM, val);
      return;
    case REGION_PI_ROM:
      switch (addr) {
        case REGION_CART_ISVIEWER_BUFFER:
          Util::WriteAccess<u32>(mem.isviewer, addr - SREGION_CART_ISVIEWER_BUFFER, be32toh(val));
          break;
        case CART_ISVIEWER_FLUSH: {
          if (val < CART_ISVIEWER_SIZE) {
            std::string message(val + 1, 0);
            std::copy(mem.isviewer.begin(), mem.isviewer.begin() + val, message.begin());
            Util::print("{}", message);
          } else {
            Util::panic("ISViewer buffer size is emulated at {} bytes, but received a flush command for {} bytes!", CART_ISVIEWER_SIZE, val);
          }
          break;
        }
        default:
          if (!WriteLatch(val)) [[unlikely]] {
            Util::warn("Couldn't latch PI bus, ignoring write to REGION_PI_ROM");
            return;
          }
          Util::warn("Writing word 0x{:08X} to address 0x{:08X} in unsupported region: REGION_PI_ROM", val, addr);
      }
      return;
    default:
      Util::panic("Should never end up here! Access to address {:08X} which did not match any PI bus regions!", addr);
  }
}

template <>
void PI::BusWrite<u32, true>(u32 addr, u32 val) {
  BusWrite<u32, false>(addr, val);
}

template <> auto PI::BusRead<u64, false>(u32 addr) -> u64 {
  if (!ReadLatch()) [[unlikely]] {
    return (u64)latch << 32;
  }

  switch (addr) {
    case REGION_PI_UNKNOWN:
      Util::panic("Reading dword from address 0x{:08X} in unsupported region: REGION_PI_UNKNOWN", addr);
    case REGION_PI_64DD_REG:
      Util::panic("Reading dword from address 0x{:08X} in unsupported region: REGION_PI_64DD_REG", addr);
    case REGION_PI_64DD_ROM:
      Util::panic("Reading dword from address 0x{:08X} in unsupported region: REGION_PI_64DD_ROM", addr);
    case REGION_PI_SRAM:
      Util::panic("Reading dword from address 0x{:08X} in unsupported region: REGION_PI_SRAM", addr);
    case REGION_PI_ROM: {
      u32 index = addr - SREGION_PI_ROM;
      if (index > mem.rom.cart.size() - 7) { // -7 because we're reading an entire dword
        Util::panic("Address 0x{:08X} accessed an index {}/0x{:X} outside the bounds of the ROM!", addr, index, index);
      }
      return Util::ReadAccess<u64>(mem.rom.cart, index);
    }
    default:
      Util::panic("Should never end up here! Access to address {:08X} which did not match any PI bus regions!", addr);
  }
}

template <> auto PI::BusRead<u64, true>(u32 addr) -> u64 {
  return BusRead<u64, false>(addr);
}

template <> void PI::BusWrite<false>(u32 addr, u64 val) {
  if (!WriteLatch(val >> 32)) [[unlikely]] {
    return;
  }

  switch (addr) {
    case REGION_PI_UNKNOWN:
      Util::panic("Writing dword 0x{:016X} to address 0x{:08X} in unsupported region: REGION_PI_UNKNOWN", val, addr);
    case REGION_PI_64DD_REG:
      Util::panic("Writing dword 0x{:016X} to address 0x{:08X} in unsupported region: REGION_PI_64DD_REG", val, addr);
    case REGION_PI_64DD_ROM:
      Util::panic("Writing dword 0x{:016X} to address 0x{:08X} in unsupported region: REGION_PI_64DD_ROM", val, addr);
    case REGION_PI_SRAM:
      Util::panic("Writing dword 0x{:016X} to address 0x{:08X} in unsupported region: REGION_PI_SRAM", val, addr);
    case REGION_PI_ROM:
      Util::warn("Writing dword 0x{:016X} to address 0x{:08X} in unsupported region: REGION_PI_ROM", val, addr);
      break;
    default:
      Util::panic("Should never end up here! Access to address %08X which did not match any PI bus regions!", addr);
  }
}

template <> void PI::BusWrite<true>(u32 addr, u64 val) {
  BusWrite<false>(addr, val);
}

auto PI::Read(u32 addr) const -> u32 {
  switch(addr) {
    case 0x04600000: return dramAddr;
    case 0x04600004: return cartAddr;
    case 0x04600008: return rdLen;
    case 0x0460000C: return wrLen;
    case 0x04600010: {
      u32 value = 0;
      value |= (dmaBusy << 0); // Is PI DMA active? No, because it's instant
      value |= (ioBusy << 1); // Is PI IO busy? No, because it's instant
      value |= (0 << 2); // PI IO error?
      value |= (mem.mmio.mi.miIntr.pi << 3); // PI interrupt?
      return value;
    }
    case 0x04600014: return piBsdDom1Lat;
    case 0x04600018: return piBsdDom1Pwd;
    case 0x0460001C: return piBsdDom1Pgs;
    case 0x04600020: return piBsdDom1Rls;
    case 0x04600024: return piBsdDom2Lat;
    case 0x04600028: return piBsdDom2Pwd;
    case 0x0460002C: return piBsdDom2Pgs;
    case 0x04600030: return piBsdDom2Rls;
    default:
      Util::panic("Unhandled PI[{:08X}] read", addr);
  }
}

u8 PI::GetDomain(u32 address) {
  switch (address) {
    case REGION_PI_UNKNOWN:
    case REGION_PI_64DD_ROM:
    case REGION_PI_ROM:
      return 1;
    case REGION_PI_64DD_REG:
    case REGION_PI_SRAM:
      return 2;
    default:
      Util::panic("Unknown PI domain for address {:08X}!", address);
  }
}

u32 PI::AccessTiming(u8 domain, u32 length) const {
  uint32_t cycles = 0;
  uint32_t latency = 0;
  uint32_t pulse_width = 0;
  uint32_t release = 0;
  uint32_t page_size = 0;
  uint32_t pages;

  switch (domain) {
    case 1:
      latency = piBsdDom1Lat + 1;
      pulse_width = piBsdDom1Pwd + 1;
      release = piBsdDom1Rls + 1;
      page_size = std::pow(2, (piBsdDom1Pgs + 2));
      break;
    case 2:
      latency = piBsdDom2Lat + 1;
      pulse_width = piBsdDom2Pwd + 1;
      release = piBsdDom2Rls + 1;
      page_size = std::pow(2, (piBsdDom2Pgs + 2));
      break;
    default:
      Util::panic("Unknown PI domain: {}\n", domain);
  }

  pages = ceil((double)length / page_size);

  cycles += (14 + latency) * pages;
  cycles += (pulse_width + release) * (length / 2);
  cycles += 5 * pages;
  return cycles * 1.5; // Converting RCP clock speed to CPU clock speed
}

void PI::Write(u32 addr, u32 val) {
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

      for (int i = 0; i < len; i++) {
        u32 addr = BYTE_ADDRESS(dramAddrInternal + i) & RDRAM_DSIZE;
        if (addr < RDRAM_SIZE) {
          BusWrite<u8, true>(cartAddrInternal + i, mem.mmio.rdp.rdram[addr]);
        }
        else {
          BusWrite<u8, true>(cartAddrInternal + i, 0);
        }
      }
      Util::trace("PI DMA from RDRAM to CARTRIDGE (size: {} B, {:08X} to {:08X})", len, dramAddr, cartAddr);
      dmaBusy = true;
      toCart = true;
      scheduler.EnqueueRelative(AccessTiming(GetDomain(cartAddr), len), PI_DMA_COMPLETE);
    } break;
    case 0x0460000C: {
      u32 len = (val & 0x00FFFFFF) + 1;
      cartAddrInternal = cartAddr & 0xFFFFFFFE;
      dramAddrInternal = dramAddr & 0x007FFFFE;
      if (dramAddrInternal & 0x7) {
        len -= (dramAddrInternal & 0x7);
      }
      wrLen = len;

      if(mem.saveType == SAVE_FLASH_1m && cartAddrInternal >= SREGION_PI_SRAM && cartAddrInternal < 0x08010000) {
        cartAddrInternal = SREGION_PI_SRAM | ((cartAddrInternal & 0xFFFFF) << 1);
      }

      for(u32 i = 0; i < len; i++) {
        u32 addr = BYTE_ADDRESS(dramAddrInternal + i) & RDRAM_DSIZE;
        if (addr < RDRAM_SIZE) {
          mem.mmio.rdp.rdram[addr] = BusRead<u8, true>(cartAddrInternal + i);
        }
      }
      dmaBusy = true;
      Util::trace("PI DMA from CARTRIDGE to RDRAM (size: {} B, {:08X} to {:08X})", len, cartAddr, dramAddr);
      toCart = false;
      scheduler.EnqueueRelative(AccessTiming(GetDomain(cartAddr), len), PI_DMA_COMPLETE);
    } break;
    case 0x04600010:
      if(val & 2) {
        mi.InterruptLower(MI::Interrupt::PI);
      } break;
    case 0x04600014: piBsdDom1Lat = val & 0xff; break;
    case 0x04600018: piBsdDom1Pwd = val & 0xff; break;
    case 0x0460001C: piBsdDom1Pgs = val & 0xff; break;
    case 0x04600020: piBsdDom1Rls = val & 0xff; break;
    case 0x04600024: piBsdDom2Lat = val & 0xff; break;
    case 0x04600028: piBsdDom2Pwd = val & 0xff; break;
    case 0x0460002C: piBsdDom2Pgs = val & 0xff; break;
    case 0x04600030: piBsdDom2Rls = val & 0xff; break;
    default:
      Util::panic("Unhandled PI[{:08X}] write ({:08X})", val, addr);
  }
}
}