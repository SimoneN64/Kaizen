#include <Mem.hpp>
#include <fstream>
#include <util.hpp>
#include <n64/core/cpu/Registers.hpp>
#include <n64/core/cpu/registers/Cop0.hpp>
#include <n64/core/Cpu.hpp>

namespace n64 {
Mem::Mem() {
  Reset();
}

void Mem::Reset() {
  sram.resize(SRAM_SIZE);
  std::fill(sram.begin(), sram.end(), 0);
  romMask = 0;
  mmio.Reset();
}

void Mem::LoadROM(const std::string& filename) {
  std::ifstream file(filename, std::ios::binary);
  file.unsetf(std::ios::skipws);

  if(!file.is_open()) {
    util::panic("Unable to open {}!", filename);
  }

  file.seekg(0, std::ios::end);
  auto size = file.tellg();
  auto sizeAdjusted = util::NextPow2(size);
  romMask = sizeAdjusted - 1;
  file.seekg(0, std::ios::beg);

  std::fill(cart.begin(), cart.end(), 0);
  cart.resize(sizeAdjusted);
  cart.insert(cart.begin(), std::istream_iterator<u8>(file), std::istream_iterator<u8>());

  file.close();
  util::SwapN64Rom(sizeAdjusted, cart.data());
  memcpy(mmio.rsp.dmem, cart.data(), 0x1000);
}

template <bool tlb>
inline bool MapVAddr(Registers& regs, TLBAccessType accessType, u32 vaddr, u32& paddr) {
  paddr = vaddr & 0x1FFFFFFF;
  if constexpr(!tlb) return true;

  switch(vaddr >> 29) {
    case 0 ... 3: case 7:
      return ProbeTLB(regs, accessType, s64(s32(vaddr)), paddr, nullptr);
    case 4 ... 5: return true;
    case 6: util::panic("Unimplemented virtual mapping in KSSEG! ({:08X})\n", vaddr);
    default:
      util::panic("Should never end up in default case in map_vaddr! ({:08X})\n", vaddr);
  }

  return false;
}

template <class T, bool tlb>
T Mem::Read(Registers& regs, u32 vaddr, s64 pc) {
  u32 paddr = vaddr;
  if(!MapVAddr<tlb>(regs, LOAD, vaddr, paddr)) {
    HandleTLBException(regs, vaddr);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, LOAD), 0, pc);
  }

  switch(paddr) {
    case 0x00000000 ... 0x007FFFFF:
      if constexpr (sizeof(T) == 1) {
        return util::ReadAccess<T>(mmio.rdp.dram.data(), BYTE_ADDRESS(paddr) & RDRAM_DSIZE);
      } else if constexpr (sizeof(T) == 2) {
        return util::ReadAccess<T>(mmio.rdp.dram.data(), HALF_ADDRESS(paddr) & RDRAM_DSIZE);
      } else {
        return util::ReadAccess<T>(mmio.rdp.dram.data(), paddr & RDRAM_DSIZE);
      }
    case 0x04000000 ... 0x04000FFF:
      if constexpr (sizeof(T) == 1) {
        return util::ReadAccess<T>(mmio.rsp.dmem, BYTE_ADDRESS(paddr) & DMEM_DSIZE);
      } else if constexpr (sizeof(T) == 2) {
        return util::ReadAccess<T>(mmio.rsp.dmem, HALF_ADDRESS(paddr) & DMEM_DSIZE);
      } else {
        return util::ReadAccess<T>(mmio.rsp.dmem, paddr & DMEM_DSIZE);
      }
    case 0x04001000 ... 0x04001FFF:
      if constexpr (sizeof(T) == 1) {
        return util::ReadAccess<T>(mmio.rsp.imem, BYTE_ADDRESS(paddr) & IMEM_DSIZE);
      } else if constexpr (sizeof(T) == 2) {
        return util::ReadAccess<T>(mmio.rsp.imem, HALF_ADDRESS(paddr) & IMEM_DSIZE);
      } else {
        return util::ReadAccess<T>(mmio.rsp.imem, paddr & IMEM_DSIZE);
      }
    case 0x04040000 ... 0x040FFFFF: case 0x04100000 ... 0x041FFFFF:
    case 0x04300000 ...	0x044FFFFF: case 0x04500000 ... 0x048FFFFF: return mmio.Read(paddr);
    case 0x10000000 ... 0x1FBFFFFF:
      if constexpr (sizeof(T) == 1) {
        return util::ReadAccess<T>(cart.data(), BYTE_ADDRESS(paddr) & romMask);
      } else if constexpr (sizeof(T) == 2) {
        return util::ReadAccess<T>(cart.data(), HALF_ADDRESS(paddr) & romMask);
      } else {
        return util::ReadAccess<T>(cart.data(), paddr & romMask);
      }
    case 0x1FC00000 ... 0x1FC007BF:
      if constexpr (sizeof(T) == 1) {
        return util::ReadAccess<T>(pifBootrom, BYTE_ADDRESS(paddr) & PIF_BOOTROM_DSIZE);
      } else if constexpr (sizeof(T) == 2) {
        return util::ReadAccess<T>(pifBootrom, HALF_ADDRESS(paddr) & PIF_BOOTROM_DSIZE);
      } else {
        return util::ReadAccess<T>(pifBootrom, paddr & PIF_BOOTROM_DSIZE);
      }
    case 0x1FC007C0 ... 0x1FC007FF:
      if constexpr (sizeof(T) == 1) {
        return util::ReadAccess<T>(pifRam, BYTE_ADDRESS(paddr) & PIF_RAM_DSIZE);
      } else if constexpr (sizeof(T) == 2) {
        return util::ReadAccess<T>(pifRam, HALF_ADDRESS(paddr) & PIF_RAM_DSIZE);
      } else {
        return util::ReadAccess<T>(pifRam, paddr & PIF_RAM_DSIZE);
      }
    case 0x00800000 ... 0x03FFFFFF: case 0x04002000 ... 0x0403FFFF:
    case 0x04200000 ... 0x042FFFFF:
    case 0x04900000 ... 0x07FFFFFF: case 0x08000000 ... 0x0FFFFFFF:
    case 0x80000000 ... 0xFFFFFFFF: case 0x1FC00800 ... 0x7FFFFFFF: return 0;
    default: util::panic("Unimplemented {}-bit read at address {:08X} (PC = {:016X})\n", sizeof(T) * 8, paddr, regs.pc);
  }
  return 0;
}

template u8 Mem::Read<u8>(Registers& regs, u32 vaddr, s64 pc);
template u16 Mem::Read<u16>(Registers& regs, u32 vaddr, s64 pc);
template u32 Mem::Read<u32>(Registers& regs, u32 vaddr, s64 pc);
template u64 Mem::Read<u64>(Registers& regs, u32 vaddr, s64 pc);
template u8 Mem::Read<u8, false>(Registers& regs, u32 vaddr, s64 pc);
template u16 Mem::Read<u16, false>(Registers& regs, u32 vaddr, s64 pc);
template u32 Mem::Read<u32, false>(Registers& regs, u32 vaddr, s64 pc);
template u64 Mem::Read<u64, false>(Registers& regs, u32 vaddr, s64 pc);
template s8 Mem::Read<s8>(Registers& regs, u32 vaddr, s64 pc);
template s16 Mem::Read<s16>(Registers& regs, u32 vaddr, s64 pc);
template s32 Mem::Read<s32>(Registers& regs, u32 vaddr, s64 pc);
template s64 Mem::Read<s64>(Registers& regs, u32 vaddr, s64 pc);
template s8 Mem::Read<s8, false>(Registers& regs, u32 vaddr, s64 pc);
template s16 Mem::Read<s16, false>(Registers& regs, u32 vaddr, s64 pc);
template s32 Mem::Read<s32, false>(Registers& regs, u32 vaddr, s64 pc);
template s64 Mem::Read<s64, false>(Registers& regs, u32 vaddr, s64 pc);

template <class T, bool tlb>
void Mem::Write(Registers& regs, u32 vaddr, T val, s64 pc) {
  u32 paddr = vaddr;
  if(!MapVAddr<tlb>(regs, STORE, vaddr, paddr)) {
    HandleTLBException(regs, vaddr);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, STORE), 0, pc);
  }

  switch(paddr) {
    case 0x00000000 ... 0x007FFFFF:
      if constexpr (sizeof(T) == 1) {
        util::WriteAccess<T>(mmio.rdp.dram.data(), BYTE_ADDRESS(paddr) & RDRAM_DSIZE, val);
      } else if constexpr (sizeof(T) == 2) {
        util::WriteAccess<T>(mmio.rdp.dram.data(), HALF_ADDRESS(paddr) & RDRAM_DSIZE, val);
      } else {
        util::WriteAccess<T>(mmio.rdp.dram.data(), paddr & RDRAM_DSIZE, val);
      }
      break;
    case 0x04000000 ... 0x04000FFF:
      if constexpr (sizeof(T) == 1) {
        util::WriteAccess<T>(mmio.rsp.dmem, BYTE_ADDRESS(paddr) & DMEM_DSIZE, val);
      } else if constexpr (sizeof(T) == 2) {
        util::WriteAccess<T>(mmio.rsp.dmem, HALF_ADDRESS(paddr) & DMEM_DSIZE, val);
      } else {
        util::WriteAccess<T>(mmio.rsp.dmem, paddr & DMEM_DSIZE, val);
      }
      break;
    case 0x04001000 ... 0x04001FFF:
      if constexpr (sizeof(T) == 1) {
        util::WriteAccess<T>(mmio.rsp.imem, BYTE_ADDRESS(paddr) & IMEM_DSIZE, val);
      } else if constexpr (sizeof(T) == 2) {
        util::WriteAccess<T>(mmio.rsp.imem, HALF_ADDRESS(paddr) & IMEM_DSIZE, val);
      } else {
        util::WriteAccess<T>(mmio.rsp.imem, paddr & IMEM_DSIZE, val);
      }
      break;
    case 0x04040000 ... 0x040FFFFF: case 0x04100000 ... 0x041FFFFF:
    case 0x04300000 ...	0x044FFFFF: case 0x04500000 ... 0x048FFFFF: mmio.Write(*this, regs, paddr, val); break;
    case 0x1FC007C0 ... 0x1FC007FF:
      if constexpr (sizeof(T) == 1) {
        util::WriteAccess<T>(pifRam, BYTE_ADDRESS(paddr) & PIF_RAM_DSIZE, val);
      } else if constexpr (sizeof(T) == 2) {
        util::WriteAccess<T>(pifRam, HALF_ADDRESS(paddr) & PIF_RAM_DSIZE, val);
      } else {
        util::WriteAccess<T>(pifRam, paddr & PIF_RAM_DSIZE, val);
      }
      break;
    case 0x00800000 ... 0x03FFFFFF: case 0x04002000 ... 0x0403FFFF:
    case 0x04200000 ... 0x042FFFFF:
    case 0x04900000 ... 0x07FFFFFF: case 0x08000000 ... 0x0FFFFFFF:
    case 0x80000000 ... 0xFFFFFFFF: case 0x1FC00800 ... 0x7FFFFFFF: break;
    default: util::panic("Unimplemented {}-bit write at address {:08X} with value {:0X} (PC = {:016X})\n", sizeof(T) * 8, paddr, val, regs.pc);
  }
}

template void Mem::Write<u8>(Registers& regs, u32 vaddr, u8 val, s64 pc);
template void Mem::Write<u16>(Registers& regs, u32 vaddr, u16 val, s64 pc);
template void Mem::Write<u32>(Registers& regs, u32 vaddr, u32 val, s64 pc);
template void Mem::Write<u64>(Registers& regs, u32 vaddr, u64 val, s64 pc);
template void Mem::Write<u8, false>(Registers& regs, u32 vaddr, u8 val, s64 pc);
template void Mem::Write<u16, false>(Registers& regs, u32 vaddr, u16 val, s64 pc);
template void Mem::Write<u32, false>(Registers& regs, u32 vaddr, u32 val, s64 pc);
template void Mem::Write<u64, false>(Registers& regs, u32 vaddr, u64 val, s64 pc);
template void Mem::Write<s8>(Registers& regs, u32 vaddr, s8 val, s64 pc);
template void Mem::Write<s16>(Registers& regs, u32 vaddr, s16 val, s64 pc);
template void Mem::Write<s32>(Registers& regs, u32 vaddr, s32 val, s64 pc);
template void Mem::Write<s64>(Registers& regs, u32 vaddr, s64 val, s64 pc);
template void Mem::Write<s8, false>(Registers& regs, u32 vaddr, s8 val, s64 pc);
template void Mem::Write<s16, false>(Registers& regs, u32 vaddr, s16 val, s64 pc);
template void Mem::Write<s32, false>(Registers& regs, u32 vaddr, s32 val, s64 pc);
template void Mem::Write<s64, false>(Registers& regs, u32 vaddr, s64 val, s64 pc);
}