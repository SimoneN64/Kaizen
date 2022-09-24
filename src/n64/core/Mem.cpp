#include <Mem.hpp>
#include <fstream>
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

u32 Mem::LoadROM(const std::string& filename) {
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

  u32 crc = 0;
  util::SwapN64Rom(crc, sizeAdjusted, cart.data());
  memcpy(mmio.rsp.dmem, cart.data(), 0x1000);

  return crc;
}

template <bool tlb>
bool MapVAddr(Registers& regs, TLBAccessType accessType, u32 vaddr, u32& paddr) {
  paddr = vaddr & 0x1FFFFFFF;
  if constexpr(!tlb) return true;

  switch(vaddr >> 29) {
    case 0 ... 3: case 7:
      return ProbeTLB(regs, accessType, vaddr, paddr, nullptr);
    case 4 ... 5: return true;
    case 6: util::panic("Unimplemented virtual mapping in KSSEG! ({:08X})\n", vaddr);
    default:
      util::panic("Should never end up in default case in map_vaddr! ({:08X})\n", vaddr);
  }

  return false;
}

template bool MapVAddr<true>(Registers& regs, TLBAccessType accessType, u32 vaddr, u32& paddr);
template bool MapVAddr<false>(Registers& regs, TLBAccessType accessType, u32 vaddr, u32& paddr);

template <bool tlb>
u8 Mem::Read8(n64::Registers &regs, u32 vaddr, s64 pc) {
  u32 paddr = vaddr;
  if(!MapVAddr<tlb>(regs, LOAD, vaddr, paddr)) {
    HandleTLBException(regs, vaddr);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, LOAD), 0, pc);
  }

  switch(paddr) {
    case 0x00000000 ... 0x007FFFFF:
      return mmio.rdp.dram[BYTE_ADDRESS(paddr) & RDRAM_DSIZE];
    case 0x04000000 ... 0x0403FFFF:
      if(paddr & 0x1000)
        return mmio.rsp.imem[BYTE_ADDRESS(paddr) & IMEM_DSIZE];
      else
        return mmio.rsp.dmem[BYTE_ADDRESS(paddr) & DMEM_DSIZE];
    case 0x04040000 ... 0x040FFFFF: case 0x04100000 ... 0x041FFFFF:
    case 0x04300000 ...	0x044FFFFF: case 0x04500000 ... 0x048FFFFF:
      return mmio.Read(paddr);
    case 0x10000000 ... 0x1FBFFFFF:
      return cart[BYTE_ADDRESS(paddr) & romMask];
    case 0x1FC00000 ... 0x1FC007BF:
      return pifBootrom[BYTE_ADDRESS(paddr) & PIF_BOOTROM_DSIZE];
    case 0x1FC007C0 ... 0x1FC007FF:
      return pifRam[paddr & PIF_RAM_DSIZE];
    case 0x00800000 ... 0x03FFFFFF: case 0x04200000 ... 0x042FFFFF:
    case 0x04900000 ... 0x07FFFFFF: case 0x08000000 ... 0x0FFFFFFF:
    case 0x80000000 ... 0xFFFFFFFF: case 0x1FC00800 ... 0x7FFFFFFF: return 0;
    default: util::panic("Unimplemented 8-bit read at address {:08X} (PC = {:016X})\n", paddr, (u64)regs.pc);
  }
}

template <bool tlb>
u16 Mem::Read16(n64::Registers &regs, u32 vaddr, s64 pc) {
  u32 paddr = vaddr;
  if(!MapVAddr<tlb>(regs, LOAD, vaddr, paddr)) {
    HandleTLBException(regs, vaddr);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, LOAD), 0, pc);
  }

  switch(paddr) {
    case 0x00000000 ... 0x007FFFFF:
      return util::ReadAccess<u16>(mmio.rdp.dram.data(), HALF_ADDRESS(paddr) & RDRAM_DSIZE);
    case 0x04000000 ... 0x0403FFFF:
      if(paddr & 0x1000)
        return util::ReadAccess<u16>(mmio.rsp.imem, HALF_ADDRESS(paddr) & IMEM_DSIZE);
      else
        return util::ReadAccess<u16>(mmio.rsp.dmem, HALF_ADDRESS(paddr) & DMEM_DSIZE);
    case 0x04040000 ... 0x040FFFFF: case 0x04100000 ... 0x041FFFFF:
    case 0x04300000 ...	0x044FFFFF: case 0x04500000 ... 0x048FFFFF:
      return mmio.Read(paddr);
    case 0x10000000 ... 0x1FBFFFFF:
      return util::ReadAccess<u16>(cart.data(), HALF_ADDRESS(paddr) & romMask);
    case 0x1FC00000 ... 0x1FC007BF:
      return util::ReadAccess<u16>(pifBootrom, HALF_ADDRESS(paddr) & PIF_BOOTROM_DSIZE);
    case 0x1FC007C0 ... 0x1FC007FF:
      return be16toh(util::ReadAccess<u16>(pifRam, paddr & PIF_RAM_DSIZE));
    case 0x00800000 ... 0x03FFFFFF: case 0x04200000 ... 0x042FFFFF:
    case 0x04900000 ... 0x07FFFFFF: case 0x08000000 ... 0x0FFFFFFF:
    case 0x80000000 ... 0xFFFFFFFF: case 0x1FC00800 ... 0x7FFFFFFF: return 0;
    default: util::panic("Unimplemented 16-bit read at address {:08X} (PC = {:016X})\n", paddr, (u64)regs.pc);
  }
}

template <bool tlb>
u32 Mem::Read32(n64::Registers &regs, u32 vaddr, s64 pc) {
  u32 paddr = vaddr;
  if(!MapVAddr<tlb>(regs, LOAD, vaddr, paddr)) {
    HandleTLBException(regs, vaddr);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, LOAD), 0, pc);
  }

  switch(paddr) {
    case 0x00000000 ... 0x007FFFFF:
      return util::ReadAccess<u32>(mmio.rdp.dram.data(), paddr & RDRAM_DSIZE);
    case 0x04000000 ... 0x0403FFFF:
      if(paddr & 0x1000)
        return util::ReadAccess<u32>(mmio.rsp.imem, paddr & IMEM_DSIZE);
      else
        return util::ReadAccess<u32>(mmio.rsp.dmem, paddr & DMEM_DSIZE);
    case 0x04040000 ... 0x040FFFFF: case 0x04100000 ... 0x041FFFFF:
    case 0x04300000 ...	0x044FFFFF: case 0x04500000 ... 0x048FFFFF:
      return mmio.Read(paddr);
    case 0x10000000 ... 0x1FBFFFFF:
      return util::ReadAccess<u32>(cart.data(), paddr & romMask);
    case 0x1FC00000 ... 0x1FC007BF:
      return util::ReadAccess<u32>(pifBootrom, paddr & PIF_BOOTROM_DSIZE);
    case 0x1FC007C0 ... 0x1FC007FF:
      return be32toh(util::ReadAccess<u32>(pifRam, paddr & PIF_RAM_DSIZE));
    case 0x00800000 ... 0x03FFFFFF: case 0x04200000 ... 0x042FFFFF:
    case 0x04900000 ... 0x07FFFFFF: case 0x08000000 ... 0x0FFFFFFF:
    case 0x80000000 ... 0xFFFFFFFF: case 0x1FC00800 ... 0x7FFFFFFF: return 0;
    default: util::panic("Unimplemented 32-bit read at address {:08X} (PC = {:016X})\n", paddr, (u64)regs.pc);
  }
}

template <bool tlb>
u64 Mem::Read64(n64::Registers &regs, u32 vaddr, s64 pc) {
  u32 paddr = vaddr;
  if(!MapVAddr<tlb>(regs, LOAD, vaddr, paddr)) {
    HandleTLBException(regs, vaddr);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, LOAD), 0, pc);
  }

  switch(paddr) {
    case 0x00000000 ... 0x007FFFFF:
      return util::ReadAccess<u64>(mmio.rdp.dram.data(), paddr & RDRAM_DSIZE);
    case 0x04000000 ... 0x0403FFFF:
      if(paddr & 0x1000)
        return util::ReadAccess<u64>(mmio.rsp.imem, paddr & IMEM_DSIZE);
      else
        return util::ReadAccess<u64>(mmio.rsp.dmem, paddr & DMEM_DSIZE);
    case 0x04040000 ... 0x040FFFFF: case 0x04100000 ... 0x041FFFFF:
    case 0x04300000 ...	0x044FFFFF: case 0x04500000 ... 0x048FFFFF:
      return mmio.Read(paddr);
    case 0x10000000 ... 0x1FBFFFFF:
      return util::ReadAccess<u64>(cart.data(), paddr & romMask);
    case 0x1FC00000 ... 0x1FC007BF:
      return util::ReadAccess<u64>(pifBootrom, paddr & PIF_BOOTROM_DSIZE);
    case 0x1FC007C0 ... 0x1FC007FF:
      return be64toh(util::ReadAccess<u64>(pifRam, paddr & PIF_RAM_DSIZE));
    case 0x00800000 ... 0x03FFFFFF: case 0x04200000 ... 0x042FFFFF:
    case 0x04900000 ... 0x07FFFFFF: case 0x08000000 ... 0x0FFFFFFF:
    case 0x80000000 ... 0xFFFFFFFF: case 0x1FC00800 ... 0x7FFFFFFF: return 0;
    default: util::panic("Unimplemented 32-bit read at address {:08X} (PC = {:016X})\n", paddr, (u64)regs.pc);
  }
}

template u8 Mem::Read8<false>(n64::Registers &regs, u32 vaddr, s64 pc);
template u8 Mem::Read8<true>(n64::Registers &regs, u32 vaddr, s64 pc);
template u16 Mem::Read16<false>(n64::Registers &regs, u32 vaddr, s64 pc);
template u16 Mem::Read16<true>(n64::Registers &regs, u32 vaddr, s64 pc);
template u32 Mem::Read32<false>(n64::Registers &regs, u32 vaddr, s64 pc);
template u32 Mem::Read32<true>(n64::Registers &regs, u32 vaddr, s64 pc);
template u64 Mem::Read64<false>(n64::Registers &regs, u32 vaddr, s64 pc);
template u64 Mem::Read64<true>(n64::Registers &regs, u32 vaddr, s64 pc);

template <bool tlb>
void Mem::Write8(Registers& regs, u32 vaddr, u32 val, s64 pc) {
  u32 paddr = vaddr;
  if(!MapVAddr<tlb>(regs, STORE, vaddr, paddr)) {
    HandleTLBException(regs, vaddr);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, STORE), 0, pc);
  }

  switch(paddr) {
    case 0x00000000 ... 0x007FFFFF:
      mmio.rdp.dram[BYTE_ADDRESS(paddr) & RDRAM_DSIZE] = val;
      break;
    case 0x04000000 ... 0x0403FFFF:
      val = val << (8 * (3 - (paddr & 3)));
      paddr = (paddr & DMEM_DSIZE) & ~3;
      if(paddr & 0x1000)
        util::WriteAccess<u32>(mmio.rsp.imem, paddr & IMEM_DSIZE, val);
      else
        util::WriteAccess<u32>(mmio.rsp.dmem, paddr & DMEM_DSIZE, val);
      break;
    case 0x04040000 ... 0x040FFFFF: case 0x04100000 ... 0x041FFFFF:
    case 0x04300000 ...	0x044FFFFF: case 0x04500000 ... 0x048FFFFF: mmio.Write(*this, regs, paddr, val); break;
    case 0x10000000 ... 0x13FFFFFF: break;
    case 0x1FC007C0 ... 0x1FC007FF:
      val = val << (8 * (3 - (paddr & 3)));
      paddr = (paddr & PIF_RAM_DSIZE) & ~3;
      util::WriteAccess<u32>(pifRam, paddr & PIF_RAM_DSIZE, htobe32(val));
      ProcessPIFCommands(pifRam, mmio.si.controller, *this);
      break;
    case 0x00800000 ... 0x03FFFFFF: case 0x04200000 ... 0x042FFFFF:
    case 0x08000000 ... 0x0FFFFFFF: case 0x04900000 ... 0x07FFFFFF:
    case 0x1FC00800 ... 0x7FFFFFFF: case 0x80000000 ... 0xFFFFFFFF: break;
    default: util::panic("Unimplemented 8-bit write at address {:08X} with value {:0X} (PC = {:016X})\n", paddr, val, (u64)regs.pc);
  }
}

template <bool tlb>
void Mem::Write16(Registers& regs, u32 vaddr, u32 val, s64 pc) {
  u32 paddr = vaddr;
  if(!MapVAddr<tlb>(regs, STORE, vaddr, paddr)) {
    HandleTLBException(regs, vaddr);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, STORE), 0, pc);
  }

  switch(paddr) {
    case 0x00000000 ... 0x007FFFFF:
      util::WriteAccess<u16>(mmio.rdp.dram.data(), HALF_ADDRESS(paddr) & RDRAM_DSIZE, val);
      break;
    case 0x04000000 ... 0x0403FFFF:
      val = val << (16 * !(paddr & 2));
      paddr &= ~3;
      if(paddr & 0x1000)
        util::WriteAccess<u32>(mmio.rsp.imem, paddr & IMEM_DSIZE, val);
      else
        util::WriteAccess<u32>(mmio.rsp.dmem, paddr & DMEM_DSIZE, val);
      break;
    case 0x04040000 ... 0x040FFFFF: case 0x04100000 ... 0x041FFFFF:
    case 0x04300000 ...	0x044FFFFF: case 0x04500000 ... 0x048FFFFF: mmio.Write(*this, regs, paddr, val); break;
    case 0x10000000 ... 0x13FFFFFF: break;
    case 0x1FC007C0 ... 0x1FC007FF:
      val = val << (16 * !(paddr & 2));
      paddr &= ~3;
      util::WriteAccess<u32>(pifRam, paddr & PIF_RAM_DSIZE, htobe32(val));
      ProcessPIFCommands(pifRam, mmio.si.controller, *this);
      break;
    case 0x00800000 ... 0x03FFFFFF: case 0x04200000 ... 0x042FFFFF:
    case 0x08000000 ... 0x0FFFFFFF: case 0x04900000 ... 0x07FFFFFF:
    case 0x1FC00800 ... 0x7FFFFFFF: case 0x80000000 ... 0xFFFFFFFF: break;
    default: util::panic("Unimplemented 16-bit write at address {:08X} with value {:0X} (PC = {:016X})\n", paddr, val, (u64)regs.pc);
  }
}

template <bool tlb>
void Mem::Write32(Registers& regs, u32 vaddr, u32 val, s64 pc) {
  u32 paddr = vaddr;
  if(!MapVAddr<tlb>(regs, STORE, vaddr, paddr)) {
    HandleTLBException(regs, vaddr);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, STORE), 0, pc);
  }

  switch(paddr) {
    case 0x00000000 ... 0x007FFFFF:
      util::WriteAccess<u32>(mmio.rdp.dram.data(), paddr & RDRAM_DSIZE, val);
      break;
    case 0x04000000 ... 0x0403FFFF:
      if(paddr & 0x1000)
        util::WriteAccess<u32>(mmio.rsp.imem, paddr & IMEM_DSIZE, val);
      else
        util::WriteAccess<u32>(mmio.rsp.dmem, paddr & DMEM_DSIZE, val);
      break;
    case 0x04040000 ... 0x040FFFFF: case 0x04100000 ... 0x041FFFFF:
    case 0x04300000 ...	0x044FFFFF: case 0x04500000 ... 0x048FFFFF: mmio.Write(*this, regs, paddr, val); break;
    case 0x10000000 ... 0x13FF0013: break;
    case 0x13FF0014: {
      if(val < ISVIEWER_SIZE) {
        char* message = (char*)calloc(val + 1, 1);
        memcpy(message, isviewer, val);
        printf("%s", message);
        free(message);
      }
    } break;
    case 0x13FF0020 ... 0x13FFFFFF:
      util::WriteAccess<u32>(isviewer, paddr & ISVIEWER_DSIZE, htobe32(val));
      break;
    case 0x1FC007C0 ... 0x1FC007FF:
      util::WriteAccess<u32>(pifRam, paddr & PIF_RAM_DSIZE, htobe32(val));
      ProcessPIFCommands(pifRam, mmio.si.controller, *this);
      break;
    case 0x00800000 ... 0x03FFFFFF: case 0x04200000 ... 0x042FFFFF:
    case 0x08000000 ... 0x0FFFFFFF: case 0x04900000 ... 0x07FFFFFF:
    case 0x1FC00800 ... 0x7FFFFFFF: case 0x80000000 ... 0xFFFFFFFF: break;
    default: util::panic("Unimplemented 32-bit write at address {:08X} with value {:0X} (PC = {:016X})\n", paddr, val, (u64)regs.pc);
  }
}

template <bool tlb>
void Mem::Write64(Registers& regs, u32 vaddr, u64 val, s64 pc) {
  u32 paddr = vaddr;
  if(!MapVAddr<tlb>(regs, STORE, vaddr, paddr)) {
    HandleTLBException(regs, vaddr);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, STORE), 0, pc);
  }

  switch(paddr) {
    case 0x00000000 ... 0x007FFFFF:
      util::WriteAccess<u64>(mmio.rdp.dram.data(), paddr & RDRAM_DSIZE, val);
      break;
    case 0x04000000 ... 0x0403FFFF:
      val >>= 32;
      if(paddr & 0x1000)
        util::WriteAccess<u32>(mmio.rsp.imem, paddr & IMEM_DSIZE, val);
      else
        util::WriteAccess<u32>(mmio.rsp.dmem, paddr & DMEM_DSIZE, val);
      break;
    case 0x04040000 ... 0x040FFFFF: case 0x04100000 ... 0x041FFFFF:
    case 0x04300000 ...	0x044FFFFF: case 0x04500000 ... 0x048FFFFF: mmio.Write(*this, regs, paddr, val); break;
    case 0x10000000 ... 0x13FFFFFF: break;
    case 0x1FC007C0 ... 0x1FC007FF:
      util::WriteAccess<u64>(pifRam, paddr & PIF_RAM_DSIZE, htobe64(val));
      ProcessPIFCommands(pifRam, mmio.si.controller, *this);
      break;
    case 0x00800000 ... 0x03FFFFFF: case 0x04200000 ... 0x042FFFFF:
    case 0x08000000 ... 0x0FFFFFFF: case 0x04900000 ... 0x07FFFFFF:
    case 0x1FC00800 ... 0x7FFFFFFF: case 0x80000000 ... 0xFFFFFFFF: break;
    default: util::panic("Unimplemented 64-bit write at address {:08X} with value {:0X} (PC = {:016X})\n", paddr, val, (u64)regs.pc);
  }
}

template void Mem::Write8<false>(Registers& regs, u32 vaddr, u32 val, s64 pc);
template void Mem::Write8<true>(Registers& regs, u32 vaddr, u32 val, s64 pc);
template void Mem::Write16<false>(Registers& regs, u32 vaddr, u32 val, s64 pc);
template void Mem::Write16<true>(Registers& regs, u32 vaddr, u32 val, s64 pc);
template void Mem::Write32<false>(Registers& regs, u32 vaddr, u32 val, s64 pc);
template void Mem::Write32<true>(Registers& regs, u32 vaddr, u32 val, s64 pc);
template void Mem::Write64<false>(Registers& regs, u32 vaddr, u64 val, s64 pc);
template void Mem::Write64<true>(Registers& regs, u32 vaddr, u64 val, s64 pc);
}