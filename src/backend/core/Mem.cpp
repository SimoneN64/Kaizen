#include <Mem.hpp>
#include <fstream>
#include <core/registers/Registers.hpp>
#include <core/registers/Cop0.hpp>
#include <core/Interpreter.hpp>
#include <core/Dynarec.hpp>
#include <File.hpp>

namespace n64 {
Mem::Mem() {
  Reset();
}

void Mem::Reset() {
  readPages.resize(PAGE_COUNT);
  writePages.resize(PAGE_COUNT);
  std::fill(readPages.begin(), readPages.end(), 0);
  std::fill(writePages.begin(), writePages.end(), 0);

  for(int i = 0; i < 2048; i++) {
    const auto addr = (i * PAGE_SIZE) & RDRAM_DSIZE;
    const auto pointer = (uintptr_t) &mmio.rdp.rdram[addr];
    readPages[i] = pointer;
    writePages[i] = pointer;
  }

  readPages[0x4000] = (uintptr_t) &mmio.rsp.dmem[0];
  readPages[0x4001] = (uintptr_t) &mmio.rsp.imem[0];
  writePages[0x4000] = (uintptr_t) &mmio.rsp.dmem[0];
  writePages[0x4001] = (uintptr_t) &mmio.rsp.imem[0];

  sram.resize(SRAM_SIZE);
  std::fill(sram.begin(), sram.end(), 0);
  romMask = 0;
  mmio.Reset();
  cart.resize(0xFC00000);
  std::fill(cart.begin(), cart.end(), 0);
}

CartInfo Mem::LoadROM(const std::string& filename) {
  std::ifstream file(filename, std::ios::binary);
  file.unsetf(std::ios::skipws);

  if(!file.is_open()) {
    Util::panic("Unable to open {}!", filename);
  }

  file.seekg(0, std::ios::end);
  auto size = file.tellg();
  auto sizeAdjusted = Util::NextPow2(size);
  romMask = sizeAdjusted - 1;
  file.seekg(0, std::ios::beg);

  std::fill(cart.begin(), cart.end(), 0);
  cart.resize(sizeAdjusted);
  cart.insert(cart.begin(), std::istream_iterator<u8>(file), std::istream_iterator<u8>());

  file.close();

  CartInfo result{};

  u32 cicChecksum;
  Util::SwapN64Rom(sizeAdjusted, cart.data(), result.crc, cicChecksum);
  memcpy(mmio.rsp.dmem, cart.data(), 0x1000);

  SetCICType(result.cicType, cicChecksum);
  result.isPAL = IsROMPAL();

  return result;
}

template <bool tlb>
bool MapVAddr(Registers& regs, TLBAccessType accessType, u64 vaddr, u32& paddr) {
  paddr = vaddr & 0x1FFFFFFF;
  if constexpr(!tlb) return true;

  switch(u32(vaddr) >> 29) {
    case 0 ... 3: case 7:
      return ProbeTLB(regs, accessType, vaddr, paddr, nullptr);
    case 4 ... 5: return true;
    case 6: Util::panic("Unimplemented virtual mapping in KSSEG! ({:08X})\n", vaddr);
    default:
      Util::panic("Should never end up in default case in map_vaddr! ({:08X})\n", vaddr);
  }

  return false;
}

template bool MapVAddr<true>(Registers& regs, TLBAccessType accessType, u64 vaddr, u32& paddr);
template bool MapVAddr<false>(Registers& regs, TLBAccessType accessType, u64 vaddr, u32& paddr);

template <bool tlb>
u8 Mem::Read8(n64::Registers &regs, u64 vaddr, s64 pc) {
  u32 paddr = vaddr;
  if (!MapVAddr<tlb>(regs, LOAD, vaddr, paddr)) {
    HandleTLBException(regs, vaddr);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, LOAD), 0, false);
  }

  const auto page = paddr >> 12;
  const auto offset = paddr & 0xFFF;
  const auto pointer = readPages[page];

  if(pointer) {
    return ((u8*)pointer)[BYTE_ADDRESS(offset)];
  } else {
    switch (paddr) {
      case 0x00000000 ... 0x007FFFFF:
        return mmio.rdp.rdram[BYTE_ADDRESS(paddr)];
      case 0x04000000 ... 0x0403FFFF:
        if (paddr  & 0x1000)
          return mmio.rsp.imem[BYTE_ADDRESS(paddr) - IMEM_REGION_START];
        else
          return mmio.rsp.dmem[BYTE_ADDRESS(paddr) - DMEM_REGION_START];
      case 0x04040000 ... 0x040FFFFF:
      case 0x04100000 ... 0x041FFFFF:
      case 0x04600000 ... 0x048FFFFF:
      case 0x04300000 ...  0x044FFFFF:
        return 0xff;
      case 0x04500000 ... 0x045FFFFF: {
        u32 w = mmio.ai.Read(paddr & ~3);
        int offs = 3 - (paddr & 3);
        return (w >> (offs * 8)) & 0xff;
      }
      case 0x10000000 ... 0x1FBFFFFF:
        paddr = (paddr + 2) & ~2;
        return cart[BYTE_ADDRESS(paddr) & romMask];
      case 0x1FC00000 ... 0x1FC007BF:
        return pifBootrom[BYTE_ADDRESS(paddr) - 0x1FC00000];
      case 0x1FC007C0 ... 0x1FC007FF:
        return pifRam[paddr - 0x1FC007C0];
      case 0x00800000 ... 0x03FFFFFF:
      case 0x04200000 ... 0x042FFFFF:
      case 0x04900000 ... 0x0FFFFFFF:
      case 0x1FC00800 ... 0xFFFFFFFF:
        return 0;
      default:
        Util::panic("Unimplemented 8-bit read at address {:08X} (PC = {:016X})\n", paddr, (u64) regs.pc);
    }
  }
}

template <bool tlb>
u16 Mem::Read16(n64::Registers &regs, u64 vaddr, s64 pc) {
  u32 paddr = vaddr;
  if (!MapVAddr<tlb>(regs, LOAD, vaddr, paddr)) {
    HandleTLBException(regs, vaddr);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, LOAD), 0, false);
  }

  const auto page = paddr >> 12;
  const auto offset = paddr & 0xFFF;
  const auto pointer = readPages[page];

  if(pointer) {
    return Util::ReadAccess<u16>((u8*)pointer, HALF_ADDRESS(offset));
  } else {
    switch (paddr) {
      case 0x00000000 ... 0x007FFFFF:
        return Util::ReadAccess<u16>(mmio.rdp.rdram.data(), HALF_ADDRESS(paddr));
      case 0x04000000 ... 0x0403FFFF:
        if (paddr  & 0x1000)
          return Util::ReadAccess<u16>(mmio.rsp.imem, HALF_ADDRESS(paddr) & IMEM_DSIZE);
        else
          return Util::ReadAccess<u16>(mmio.rsp.dmem, HALF_ADDRESS(paddr) & DMEM_DSIZE);
      case 0x04040000 ... 0x040FFFFF:
      case 0x04100000 ... 0x041FFFFF:
      case 0x04300000 ...  0x044FFFFF:
      case 0x04500000 ... 0x048FFFFF:
        return mmio.Read(paddr);
      case 0x10000000 ... 0x1FBFFFFF:
        paddr = (paddr + 2) & ~3;
        return Util::ReadAccess<u16>(cart.data(), HALF_ADDRESS(paddr) & romMask);
      case 0x1FC00000 ... 0x1FC007BF:
        return Util::ReadAccess<u16>(pifBootrom, HALF_ADDRESS(paddr) - 0x1FC00000);
      case 0x1FC007C0 ... 0x1FC007FF:
        return be16toh(Util::ReadAccess<u16>(pifRam, paddr - 0x1FC007C0));
      case 0x00800000 ... 0x03FFFFFF:
      case 0x04200000 ... 0x042FFFFF:
      case 0x04900000 ... 0x0FFFFFFF:
      case 0x1FC00800 ... 0xFFFFFFFF:
        return 0;
      default:
        Util::panic("Unimplemented 16-bit read at address {:08X} (PC = {:016X})\n", paddr, (u64) regs.pc);
    }
  }
}

template <bool tlb>
u32 Mem::Read32(n64::Registers &regs, u64 vaddr, s64 pc) {
  u32 paddr = vaddr;
  if (!MapVAddr<tlb>(regs, LOAD, vaddr, paddr)) {
    HandleTLBException(regs, vaddr);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, LOAD), 0, false);
  }

  const auto page = paddr >> 12;
  const auto offset = paddr & 0xFFF;
  const auto pointer = readPages[page];

  if(pointer) {
    return Util::ReadAccess<u32>((u8*)pointer, offset);
  } else {
    switch(paddr) {
      case 0x00000000 ... 0x007FFFFF:
        return Util::ReadAccess<u32>(mmio.rdp.rdram.data(), paddr);
      case 0x04000000 ... 0x0403FFFF:
        if(paddr  & 0x1000)
          return Util::ReadAccess<u32>(mmio.rsp.imem, paddr & IMEM_DSIZE);
        else
          return Util::ReadAccess<u32>(mmio.rsp.dmem, paddr & DMEM_DSIZE);
      case 0x04040000 ... 0x040FFFFF: case 0x04100000 ... 0x041FFFFF:
      case 0x04300000 ...	0x044FFFFF: case 0x04500000 ... 0x048FFFFF:
        return mmio.Read(paddr);
      case 0x10000000 ... 0x1FBFFFFF:
        return Util::ReadAccess<u32>(cart.data(), paddr & romMask);
      case 0x1FC00000 ... 0x1FC007BF:
        return Util::ReadAccess<u32>(pifBootrom, paddr - 0x1FC00000);
      case 0x1FC007C0 ... 0x1FC007FF:
        return be32toh(Util::ReadAccess<u32>(pifRam, paddr - 0x1FC007C0));
      case 0x00800000 ... 0x03FFFFFF: case 0x04200000 ... 0x042FFFFF:
      case 0x04900000 ... 0x0FFFFFFF: case 0x1FC00800 ... 0xFFFFFFFF: return 0;
      default:
        Util::panic("Unimplemented 32-bit read at address {:08X} (PC = {:016X})\n", paddr, (u64) regs.pc);
    }
  }
}

template <bool tlb>
u64 Mem::Read64(n64::Registers &regs, u64 vaddr, s64 pc) {
  u32 paddr = vaddr;
  if (!MapVAddr<tlb>(regs, LOAD, vaddr, paddr)) {
    HandleTLBException(regs, vaddr);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, LOAD), 0, false);
  }

  const auto page = paddr >> 12;
  const auto offset = paddr & 0xFFF;
  const auto pointer = readPages[page];

  if(pointer) {
    return Util::ReadAccess<u64>((u8*)pointer, offset);
  } else {
    switch (paddr) {
      case 0x00000000 ... 0x007FFFFF:
        return Util::ReadAccess<u64>(mmio.rdp.rdram.data(), paddr);
      case 0x04000000 ... 0x0403FFFF:
        if (paddr  & 0x1000)
          return Util::ReadAccess<u64>(mmio.rsp.imem, paddr & IMEM_DSIZE);
        else
          return Util::ReadAccess<u64>(mmio.rsp.dmem, paddr & DMEM_DSIZE);
      case 0x04040000 ... 0x040FFFFF:
      case 0x04100000 ... 0x041FFFFF:
      case 0x04300000 ...  0x044FFFFF:
      case 0x04500000 ... 0x048FFFFF:
        return mmio.Read(paddr);
      case 0x10000000 ... 0x1FBFFFFF:
        return Util::ReadAccess<u64>(cart.data(), paddr & romMask);
      case 0x1FC00000 ... 0x1FC007BF:
        return Util::ReadAccess<u64>(pifBootrom, paddr - 0x1FC00000);
      case 0x1FC007C0 ... 0x1FC007FF:
        return be64toh(Util::ReadAccess<u64>(pifRam, paddr - 0x1FC007C0));
      case 0x00800000 ... 0x03FFFFFF:
      case 0x04200000 ... 0x042FFFFF:
      case 0x04900000 ... 0x0FFFFFFF:
      case 0x1FC00800 ... 0xFFFFFFFF:
        return 0;
      default:
        Util::panic("Unimplemented 32-bit read at address {:08X} (PC = {:016X})\n", paddr, (u64) regs.pc);
    }
  }
}

template u8 Mem::Read8<false>(n64::Registers &regs, u64 vaddr, s64 pc);
template u8 Mem::Read8<true>(n64::Registers &regs, u64 vaddr, s64 pc);
template u16 Mem::Read16<false>(n64::Registers &regs, u64 vaddr, s64 pc);
template u16 Mem::Read16<true>(n64::Registers &regs, u64 vaddr, s64 pc);
template u32 Mem::Read32<false>(n64::Registers &regs, u64 vaddr, s64 pc);
template u32 Mem::Read32<true>(n64::Registers &regs, u64 vaddr, s64 pc);
template u64 Mem::Read64<false>(n64::Registers &regs, u64 vaddr, s64 pc);
template u64 Mem::Read64<true>(n64::Registers &regs, u64 vaddr, s64 pc);

template <bool tlb>
void Mem::Write8(Registers& regs, n64::JIT::Dynarec& dyn, u64 vaddr, u32 val, s64 pc) {
  u32 paddr = vaddr;
  if (!MapVAddr<tlb>(regs, LOAD, vaddr, paddr)) {
    HandleTLBException(regs, vaddr);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, LOAD), 0, false);
  }

  dyn.InvalidatePage(BYTE_ADDRESS(paddr));

  const auto page = paddr >> 12;
  auto offset = paddr & 0xFFF;
  const auto pointer = readPages[page];

  if(pointer) {
    if(paddr >= 0x04000000 && paddr <= 0x0403FFFF) {
      val = val << (8 * (3 - (paddr & 3)));
      offset = (offset & DMEM_DSIZE) & ~3;
    }

    ((u8*)pointer)[BYTE_ADDRESS(offset)] = val;
  } else {
    switch (paddr) {
      case 0x00000000 ... 0x007FFFFF:
        mmio.rdp.rdram[BYTE_ADDRESS(paddr)] = val;
        break;
      case 0x04000000 ... 0x0403FFFF:
        val = val << (8 * (3 - (paddr & 3)));
        paddr = (paddr & DMEM_DSIZE) & ~3;
        if (paddr & 0x1000)
          Util::WriteAccess<u32>(mmio.rsp.imem, paddr & IMEM_DSIZE, val);
        else
          Util::WriteAccess<u32>(mmio.rsp.dmem, paddr & DMEM_DSIZE, val);
        break;
      case 0x04040000 ... 0x040FFFFF:
      case 0x04100000 ... 0x041FFFFF:
      case 0x04300000 ...  0x044FFFFF:
      case 0x04500000 ... 0x048FFFFF:
        Util::panic("MMIO Write8!\n");
      case 0x10000000 ... 0x13FFFFFF:
        break;
      case 0x1FC007C0 ... 0x1FC007FF:
        val = val << (8 * (3 - (paddr & 3)));
        paddr = (paddr - 0x1FC007C0) & ~3;
        Util::WriteAccess<u32>(pifRam, paddr, htobe32(val));
        ProcessPIFCommands(pifRam, mmio.si.controller, *this);
        break;
      case 0x00800000 ... 0x03FFFFFF:
      case 0x04200000 ... 0x042FFFFF:
      case 0x08000000 ... 0x0FFFFFFF:
      case 0x04900000 ... 0x07FFFFFF:
      case 0x1FC00800 ... 0x7FFFFFFF:
      case 0x80000000 ... 0xFFFFFFFF:
        break;
      default:
        Util::panic("Unimplemented 8-bit write at address {:08X} with value {:0X} (PC = {:016X})\n", paddr, val,
                    (u64) regs.pc);
    }
  }
}

template <bool tlb>
void Mem::Write16(Registers& regs, n64::JIT::Dynarec& dyn, u64 vaddr, u32 val, s64 pc) {
  u32 paddr = vaddr;
  if (!MapVAddr<tlb>(regs, STORE, vaddr, paddr)) {
    HandleTLBException(regs, vaddr);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, STORE), 0, false);
  }

  dyn.InvalidatePage(HALF_ADDRESS(paddr));

  const auto page = paddr >> 12;
  auto offset = paddr & 0xFFF;
  const auto pointer = readPages[page];

  if(pointer) {
    if(paddr >= 0x04000000 && paddr <= 0x0403FFFF) {
      val = val << (16 * !(paddr & 2));
      offset &= ~3;
    }

    Util::WriteAccess<u16>((u8*)pointer, HALF_ADDRESS(offset), val);
  } else {
    switch (paddr) {
      case 0x00000000 ... 0x007FFFFF:
        Util::WriteAccess<u16>(mmio.rdp.rdram.data(), HALF_ADDRESS(paddr), val);
        break;
      case 0x04000000 ... 0x0403FFFF:
        val = val << (16 * !(paddr & 2));
        paddr &= ~3;
        if (paddr & 0x1000)
          Util::WriteAccess<u32>(mmio.rsp.imem, paddr & IMEM_DSIZE, val);
        else
          Util::WriteAccess<u32>(mmio.rsp.dmem, paddr & DMEM_DSIZE, val);
        break;
      case 0x04040000 ... 0x040FFFFF:
      case 0x04100000 ... 0x041FFFFF:
      case 0x04300000 ...  0x044FFFFF:
      case 0x04500000 ... 0x048FFFFF:
        Util::panic("MMIO Write16!\n");
      case 0x10000000 ... 0x13FFFFFF:
        break;
      case 0x1FC007C0 ... 0x1FC007FF:
        val = val << (16 * !(paddr & 2));
        paddr &= ~3;
        Util::WriteAccess<u32>(pifRam, paddr - 0x1FC007C0, htobe32(val));
        ProcessPIFCommands(pifRam, mmio.si.controller, *this);
        break;
      case 0x00800000 ... 0x03FFFFFF:
      case 0x04200000 ... 0x042FFFFF:
      case 0x08000000 ... 0x0FFFFFFF:
      case 0x04900000 ... 0x07FFFFFF:
      case 0x1FC00800 ... 0x7FFFFFFF:
      case 0x80000000 ... 0xFFFFFFFF:
        break;
      default:
        Util::panic("Unimplemented 16-bit write at address {:08X} with value {:0X} (PC = {:016X})\n", paddr, val,
                    (u64) regs.pc);
    }
  }
}

template <bool tlb>
void Mem::Write32(Registers& regs, n64::JIT::Dynarec& dyn, u64 vaddr, u32 val, s64 pc) {
  u32 paddr = vaddr;
  if(!MapVAddr<tlb>(regs, STORE, vaddr, paddr)) {
    HandleTLBException(regs, vaddr);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, STORE), 0, false);
  }

  dyn.InvalidatePage(paddr);

  const auto page = paddr >> 12;
  auto offset = paddr & 0xFFF;
  const auto pointer = readPages[page];

  if(pointer) {
    Util::WriteAccess<u32>((u8*)pointer, offset, val);
  } else {
    switch(paddr) {
      case 0x00000000 ... 0x007FFFFF:
        Util::WriteAccess<u32>(mmio.rdp.rdram.data(), paddr, val);
        break;
      case 0x04000000 ... 0x0403FFFF:
        if(paddr & 0x1000)
          Util::WriteAccess<u32>(mmio.rsp.imem, paddr & IMEM_DSIZE, val);
        else
          Util::WriteAccess<u32>(mmio.rsp.dmem, paddr & DMEM_DSIZE, val);
        break;
      case 0x04040000 ... 0x040FFFFF: case 0x04100000 ... 0x041FFFFF:
      case 0x04300000 ...	0x044FFFFF: case 0x04500000 ... 0x048FFFFF: mmio.Write(*this, regs, paddr, val); break;
      case 0x10000000 ... 0x13FF0013: break;
      case 0x13FF0014: {
        if(val < ISVIEWER_SIZE) {
          char* message = (char*)calloc(val + 1, 1);
          memcpy(message, isviewer, val);
          fmt::print("{}", message);
          free(message);
        }
      } break;
      case 0x13FF0020 ... 0x13FFFFFF:
        Util::WriteAccess<u32>(isviewer, paddr - 0x13FF0020, htobe32(val));
        break;
      case 0x1FC007C0 ... 0x1FC007FF:
        Util::WriteAccess<u32>(pifRam, paddr - 0x1FC007C0, htobe32(val));
        ProcessPIFCommands(pifRam, mmio.si.controller, *this);
        break;
      case 0x00800000 ... 0x03FFFFFF: case 0x04200000 ... 0x042FFFFF:
      case 0x08000000 ... 0x0FFFFFFF: case 0x04900000 ... 0x07FFFFFF:
      case 0x1FC00800 ... 0x7FFFFFFF: case 0x80000000 ... 0xFFFFFFFF: break;
      default: Util::panic("Unimplemented 32-bit write at address {:08X} with value {:0X} (PC = {:016X})\n", paddr, val, (u64)regs.pc);
    }
  }
}

template <bool tlb>
void Mem::Write64(Registers& regs, n64::JIT::Dynarec& dyn, u64 vaddr, u64 val, s64 pc) {
  u32 paddr = vaddr;
  if(!MapVAddr<tlb>(regs, STORE, vaddr, paddr)) {
    HandleTLBException(regs, vaddr);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, STORE), 0, false);
  }

  dyn.InvalidatePage(paddr);

  const auto page = paddr >> 12;
  auto offset = paddr & 0xFFF;
  const auto pointer = readPages[page];

  if(pointer) {
    if(paddr >= 0x04000000 && paddr <= 0x0403FFFF) {
      val >>= 32;
    }
    Util::WriteAccess<u64>((u8*)pointer, offset, val);
  } else {
    switch (paddr) {
      case 0x00000000 ... 0x007FFFFF:
        Util::WriteAccess<u64>(mmio.rdp.rdram.data(), paddr, val);
        break;
      case 0x04000000 ... 0x0403FFFF:
        val >>= 32;
        if (paddr & 0x1000)
          Util::WriteAccess<u32>(mmio.rsp.imem, paddr & IMEM_DSIZE, val);
        else
          Util::WriteAccess<u32>(mmio.rsp.dmem, paddr & DMEM_DSIZE, val);
        break;
      case 0x04040000 ... 0x040FFFFF:
      case 0x04100000 ... 0x041FFFFF:
      case 0x04300000 ...  0x044FFFFF:
      case 0x04500000 ... 0x048FFFFF:
        Util::panic("MMIO Write64!\n");
      case 0x10000000 ... 0x13FFFFFF:
        break;
      case 0x1FC007C0 ... 0x1FC007FF:
        Util::WriteAccess<u64>(pifRam, paddr - 0x1FC007C0, htobe64(val));
        ProcessPIFCommands(pifRam, mmio.si.controller, *this);
        break;
      case 0x00800000 ... 0x03FFFFFF:
      case 0x04200000 ... 0x042FFFFF:
      case 0x08000000 ... 0x0FFFFFFF:
      case 0x04900000 ... 0x07FFFFFF:
      case 0x1FC00800 ... 0x7FFFFFFF:
      case 0x80000000 ... 0xFFFFFFFF:
        break;
      default:
        Util::panic("Unimplemented 64-bit write at address {:08X} with value {:0X} (PC = {:016X})\n", paddr, val,
                    (u64) regs.pc);
    }
  }
}

template void Mem::Write8<false>(Registers& regs, JIT::Dynarec&, u64 vaddr, u32 val, s64 pc);
template void Mem::Write8<true>(Registers& regs, JIT::Dynarec&, u64 vaddr, u32 val, s64 pc);
template void Mem::Write16<false>(Registers& regs, JIT::Dynarec&, u64 vaddr, u32 val, s64 pc);
template void Mem::Write16<true>(Registers& regs, JIT::Dynarec&, u64 vaddr, u32 val, s64 pc);
template void Mem::Write32<false>(Registers& regs, JIT::Dynarec&, u64 vaddr, u32 val, s64 pc);
template void Mem::Write32<true>(Registers& regs, JIT::Dynarec&, u64 vaddr, u32 val, s64 pc);
template void Mem::Write64<false>(Registers& regs, JIT::Dynarec&, u64 vaddr, u64 val, s64 pc);
template void Mem::Write64<true>(Registers& regs, JIT::Dynarec&, u64 vaddr, u64 val, s64 pc);

template <bool tlb>
void Mem::Write8(Registers& regs, u64 vaddr, u32 val, s64 pc) {
  u32 paddr = vaddr;
  if (!MapVAddr<tlb>(regs, LOAD, vaddr, paddr)) {
    HandleTLBException(regs, vaddr);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, LOAD), 0, false);
  }

  const auto page = paddr >> 12;
  auto offset = paddr & 0xFFF;
  const auto pointer = readPages[page];

  if(pointer) {
    if(paddr >= 0x04000000 && paddr <= 0x0403FFFF) {
      val = val << (8 * (3 - (paddr & 3)));
      offset = (offset & DMEM_DSIZE) & ~3;
    }

    ((u8*)pointer)[BYTE_ADDRESS(offset)] = val;
  } else {
    switch (paddr) {
      case 0x00000000 ... 0x007FFFFF:
        mmio.rdp.rdram[BYTE_ADDRESS(paddr)] = val;
        break;
      case 0x04000000 ... 0x0403FFFF:
        val = val << (8 * (3 - (paddr & 3)));
        paddr = (paddr & DMEM_DSIZE) & ~3;
        if (paddr & 0x1000)
          Util::WriteAccess<u32>(mmio.rsp.imem, paddr & IMEM_DSIZE, val);
        else
          Util::WriteAccess<u32>(mmio.rsp.dmem, paddr & DMEM_DSIZE, val);
        break;
      case 0x04040000 ... 0x040FFFFF:
      case 0x04100000 ... 0x041FFFFF:
      case 0x04300000 ...  0x044FFFFF:
      case 0x04500000 ... 0x048FFFFF:
        Util::panic("MMIO Write8!\n");
      case 0x10000000 ... 0x13FFFFFF:
        break;
      case 0x1FC007C0 ... 0x1FC007FF:
        val = val << (8 * (3 - (paddr & 3)));
        paddr = (paddr - 0x1FC007C0) & ~3;
        Util::WriteAccess<u32>(pifRam, paddr, htobe32(val));
        ProcessPIFCommands(pifRam, mmio.si.controller, *this);
        break;
      case 0x00800000 ... 0x03FFFFFF:
      case 0x04200000 ... 0x042FFFFF:
      case 0x08000000 ... 0x0FFFFFFF:
      case 0x04900000 ... 0x07FFFFFF:
      case 0x1FC00800 ... 0x7FFFFFFF:
      case 0x80000000 ... 0xFFFFFFFF:
        break;
      default:
        Util::panic("Unimplemented 8-bit write at address {:08X} with value {:0X} (PC = {:016X})\n", paddr, val,
                    (u64) regs.pc);
    }
  }
}

template <bool tlb>
void Mem::Write16(Registers& regs, u64 vaddr, u32 val, s64 pc) {
  u32 paddr = vaddr;
  if (!MapVAddr<tlb>(regs, STORE, vaddr, paddr)) {
    HandleTLBException(regs, vaddr);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, STORE), 0, false);
  }

  const auto page = paddr >> 12;
  auto offset = paddr & 0xFFF;
  const auto pointer = readPages[page];

  if(pointer) {
    if(paddr >= 0x04000000 && paddr <= 0x0403FFFF) {
      val = val << (16 * !(paddr & 2));
      offset &= ~3;
    }

    Util::WriteAccess<u16>((u8*)pointer, HALF_ADDRESS(offset), val);
  } else {
    switch (paddr) {
      case 0x00000000 ... 0x007FFFFF:
        Util::WriteAccess<u16>(mmio.rdp.rdram.data(), HALF_ADDRESS(paddr), val);
        break;
      case 0x04000000 ... 0x0403FFFF:
        val = val << (16 * !(paddr & 2));
        paddr &= ~3;
        if (paddr & 0x1000)
          Util::WriteAccess<u32>(mmio.rsp.imem, paddr & IMEM_DSIZE, val);
        else
          Util::WriteAccess<u32>(mmio.rsp.dmem, paddr & DMEM_DSIZE, val);
        break;
      case 0x04040000 ... 0x040FFFFF:
      case 0x04100000 ... 0x041FFFFF:
      case 0x04300000 ...  0x044FFFFF:
      case 0x04500000 ... 0x048FFFFF:
        Util::panic("MMIO Write16!\n");
      case 0x10000000 ... 0x13FFFFFF:
        break;
      case 0x1FC007C0 ... 0x1FC007FF:
        val = val << (16 * !(paddr & 2));
        paddr &= ~3;
        Util::WriteAccess<u32>(pifRam, paddr - 0x1FC007C0, htobe32(val));
        ProcessPIFCommands(pifRam, mmio.si.controller, *this);
        break;
      case 0x00800000 ... 0x03FFFFFF:
      case 0x04200000 ... 0x042FFFFF:
      case 0x08000000 ... 0x0FFFFFFF:
      case 0x04900000 ... 0x07FFFFFF:
      case 0x1FC00800 ... 0x7FFFFFFF:
      case 0x80000000 ... 0xFFFFFFFF:
        break;
      default:
        Util::panic("Unimplemented 16-bit write at address {:08X} with value {:0X} (PC = {:016X})\n", paddr, val,
                    (u64) regs.pc);
    }
  }
}

template <bool tlb>
void Mem::Write32(Registers& regs, u64 vaddr, u32 val, s64 pc) {
  u32 paddr = vaddr;
  if(!MapVAddr<tlb>(regs, STORE, vaddr, paddr)) {
    HandleTLBException(regs, vaddr);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, STORE), 0, false);
  }

  const auto page = paddr >> 12;
  auto offset = paddr & 0xFFF;
  const auto pointer = readPages[page];

  if(pointer) {
    Util::WriteAccess<u32>((u8*)pointer, offset, val);
  } else {
    switch(paddr) {
      case 0x00000000 ... 0x007FFFFF:
        Util::WriteAccess<u32>(mmio.rdp.rdram.data(), paddr, val);
        break;
      case 0x04000000 ... 0x0403FFFF:
        if(paddr & 0x1000)
          Util::WriteAccess<u32>(mmio.rsp.imem, paddr & IMEM_DSIZE, val);
        else
          Util::WriteAccess<u32>(mmio.rsp.dmem, paddr & DMEM_DSIZE, val);
        break;
      case 0x04040000 ... 0x040FFFFF: case 0x04100000 ... 0x041FFFFF:
      case 0x04300000 ...	0x044FFFFF: case 0x04500000 ... 0x048FFFFF: mmio.Write(*this, regs, paddr, val); break;
      case 0x10000000 ... 0x13FF0013: break;
      case 0x13FF0014: {
        if(val < ISVIEWER_SIZE) {
          char* message = (char*)calloc(val + 1, 1);
          memcpy(message, isviewer, val);
          fmt::print("{}", message);
          free(message);
        }
      } break;
      case 0x13FF0020 ... 0x13FFFFFF:
        Util::WriteAccess<u32>(isviewer, paddr - 0x13FF0020, htobe32(val));
        break;
      case 0x1FC007C0 ... 0x1FC007FF:
        Util::WriteAccess<u32>(pifRam, paddr - 0x1FC007C0, htobe32(val));
        ProcessPIFCommands(pifRam, mmio.si.controller, *this);
        break;
      case 0x00800000 ... 0x03FFFFFF: case 0x04200000 ... 0x042FFFFF:
      case 0x08000000 ... 0x0FFFFFFF: case 0x04900000 ... 0x07FFFFFF:
      case 0x1FC00800 ... 0x7FFFFFFF: case 0x80000000 ... 0xFFFFFFFF: break;
      default: Util::panic("Unimplemented 32-bit write at address {:08X} with value {:0X} (PC = {:016X})\n", paddr, val, (u64)regs.pc);
    }
  }
}

template <bool tlb>
void Mem::Write64(Registers& regs, u64 vaddr, u64 val, s64 pc) {
  u32 paddr = vaddr;
  if(!MapVAddr<tlb>(regs, STORE, vaddr, paddr)) {
    HandleTLBException(regs, vaddr);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, STORE), 0, false);
  }

  const auto page = paddr >> 12;
  auto offset = paddr & 0xFFF;
  const auto pointer = readPages[page];

  if(pointer) {
    if(paddr >= 0x04000000 && paddr <= 0x0403FFFF) {
      val >>= 32;
    }
    Util::WriteAccess<u64>((u8*)pointer, offset, val);
  } else {
    switch (paddr) {
      case 0x00000000 ... 0x007FFFFF:
        Util::WriteAccess<u64>(mmio.rdp.rdram.data(), paddr, val);
        break;
      case 0x04000000 ... 0x0403FFFF:
        val >>= 32;
        if (paddr & 0x1000)
          Util::WriteAccess<u32>(mmio.rsp.imem, paddr & IMEM_DSIZE, val);
        else
          Util::WriteAccess<u32>(mmio.rsp.dmem, paddr & DMEM_DSIZE, val);
        break;
      case 0x04040000 ... 0x040FFFFF:
      case 0x04100000 ... 0x041FFFFF:
      case 0x04300000 ...  0x044FFFFF:
      case 0x04500000 ... 0x048FFFFF:
        Util::panic("MMIO Write64!\n");
      case 0x10000000 ... 0x13FFFFFF:
        break;
      case 0x1FC007C0 ... 0x1FC007FF:
        Util::WriteAccess<u64>(pifRam, paddr - 0x1FC007C0, htobe64(val));
        ProcessPIFCommands(pifRam, mmio.si.controller, *this);
        break;
      case 0x00800000 ... 0x03FFFFFF:
      case 0x04200000 ... 0x042FFFFF:
      case 0x08000000 ... 0x0FFFFFFF:
      case 0x04900000 ... 0x07FFFFFF:
      case 0x1FC00800 ... 0x7FFFFFFF:
      case 0x80000000 ... 0xFFFFFFFF:
        break;
      default:
        Util::panic("Unimplemented 64-bit write at address {:08X} with value {:0X} (PC = {:016X})\n", paddr, val,
                    (u64) regs.pc);
    }
  }
}

template void Mem::Write8<false>(Registers& regs, u64 vaddr, u32 val, s64 pc);
template void Mem::Write8<true>(Registers& regs, u64 vaddr, u32 val, s64 pc);
template void Mem::Write16<false>(Registers& regs, u64 vaddr, u32 val, s64 pc);
template void Mem::Write16<true>(Registers& regs, u64 vaddr, u32 val, s64 pc);
template void Mem::Write32<false>(Registers& regs, u64 vaddr, u32 val, s64 pc);
template void Mem::Write32<true>(Registers& regs, u64 vaddr, u32 val, s64 pc);
template void Mem::Write64<false>(Registers& regs, u64 vaddr, u64 val, s64 pc);
template void Mem::Write64<true>(Registers& regs, u64 vaddr, u64 val, s64 pc);
}