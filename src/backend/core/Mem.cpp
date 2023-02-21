#include <Mem.hpp>
#include <fstream>
#include <core/registers/Registers.hpp>
#include <core/registers/Cop0.hpp>
#include <core/Interpreter.hpp>
#include <core/Dynarec.hpp>
#include <File.hpp>

namespace n64 {
Mem::Mem() {
  cart = (u8*)calloc(CART_SIZE, 1);
  sram = (u8*)calloc(SRAM_SIZE, 1);
  Reset();
}

void Mem::Reset() {
  memset(readPages, 0, PAGE_COUNT);
  memset(writePages, 0, PAGE_COUNT);

  for(int i = 0; i < RDRAM_SIZE / PAGE_SIZE; i++) {
    const auto addr = (i * PAGE_SIZE) & RDRAM_DSIZE;
    const auto pointer = (uintptr_t) &mmio.rdp.rdram[addr];
    readPages[i] = pointer;
    writePages[i] = pointer;
  }

  memset(cart, 0, CART_SIZE);
  memset(sram, 0, SRAM_SIZE);
  mmio.Reset();
}

CartInfo Mem::LoadROM(const std::string& filename) {
  std::ifstream file(filename, std::ios::binary);
  file.unsetf(std::ios::skipws);

  if(!file.is_open()) {
    Util::panic("Unable to open {}!", filename);
  }

  file.seekg(0, std::ios::end);
  size_t size = file.tellg();
  size_t sizeAdjusted = Util::NextPow2(size);
  romMask = sizeAdjusted - 1;
  file.seekg(0, std::ios::beg);

  file.read(reinterpret_cast<char*>(cart), size);

  file.close();

  CartInfo result{};

  u32 cicChecksum;
  Util::SwapN64Rom(sizeAdjusted, cart, result.crc, cicChecksum);
  memcpy(mmio.rsp.dmem, cart, 0x1000);

  SetCICType(result.cicType, cicChecksum);
  result.isPAL = IsROMPAL();

  return result;
}

u8 Mem::Read8(n64::Registers &regs, u32 paddr) {
  const auto page = paddr >> 12;
  const auto offset = paddr & 0xFFF;
  const auto pointer = readPages[page];
  SI& si = mmio.si;

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
      case CART_REGION:
        paddr = (paddr + 2) & ~2;
        return cart[BYTE_ADDRESS(paddr) & romMask];
      case 0x1FC00000 ... 0x1FC007BF:
        return si.pif.pifBootrom[BYTE_ADDRESS(paddr) - 0x1FC00000];
      case PIF_RAM_REGION:
        return si.pif.pifRam[paddr - PIF_RAM_REGION_START];
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

u16 Mem::Read16(n64::Registers &regs, u32 paddr) {
  const auto page = paddr >> 12;
  const auto offset = paddr & 0xFFF;
  const auto pointer = readPages[page];
  SI& si = mmio.si;

  if(pointer) {
    return Util::ReadAccess<u16>((u8*)pointer, HALF_ADDRESS(offset));
  } else {
    switch (paddr) {
      case 0x00000000 ... 0x007FFFFF:
        return Util::ReadAccess<u16>(mmio.rdp.rdram, HALF_ADDRESS(paddr));
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
        return Util::ReadAccess<u16>(cart, HALF_ADDRESS(paddr) & romMask);
      case 0x1FC00000 ... 0x1FC007BF:
        return Util::ReadAccess<u16>(si.pif.pifBootrom, HALF_ADDRESS(paddr) - 0x1FC00000);
      case PIF_RAM_REGION:
        return be16toh(Util::ReadAccess<u16>(si.pif.pifRam, paddr - PIF_RAM_REGION_START));
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

u32 Mem::Read32(n64::Registers &regs, u32 paddr) {
  const auto page = paddr >> 12;
  const auto offset = paddr & 0xFFF;
  const auto pointer = readPages[page];
  SI& si = mmio.si;

  if(pointer) {
    return Util::ReadAccess<u32>((u8*)pointer, offset);
  } else {
    switch(paddr) {
      case 0x00000000 ... 0x007FFFFF:
        return Util::ReadAccess<u32>(mmio.rdp.rdram, paddr);
      case 0x04000000 ... 0x0403FFFF:
        if(paddr  & 0x1000)
          return Util::ReadAccess<u32>(mmio.rsp.imem, paddr & IMEM_DSIZE);
        else
          return Util::ReadAccess<u32>(mmio.rsp.dmem, paddr & DMEM_DSIZE);
      case 0x04040000 ... 0x040FFFFF: case 0x04100000 ... 0x041FFFFF:
      case 0x04300000 ...	0x044FFFFF: case 0x04500000 ... 0x048FFFFF:
        return mmio.Read(paddr);
      case 0x10000000 ... 0x1FBFFFFF:
        return Util::ReadAccess<u32>(cart, paddr & romMask);
      case 0x1FC00000 ... 0x1FC007BF:
        return Util::ReadAccess<u32>(si.pif.pifBootrom, paddr - 0x1FC00000);
      case PIF_RAM_REGION:
        return be32toh(Util::ReadAccess<u32>(si.pif.pifRam, paddr - PIF_RAM_REGION_START));
      case 0x00800000 ... 0x03FFFFFF: case 0x04200000 ... 0x042FFFFF:
      case 0x04900000 ... 0x0FFFFFFF: case 0x1FC00800 ... 0xFFFFFFFF: return 0;
      default:
        Util::panic("Unimplemented 32-bit read at address {:08X} (PC = {:016X})\n", paddr, (u64) regs.pc);
    }
  }
}

u64 Mem::Read64(n64::Registers &regs, u32 paddr) {
  const auto page = paddr >> 12;
  const auto offset = paddr & 0xFFF;
  const auto pointer = readPages[page];
  SI& si = mmio.si;

  if(pointer) {
    return Util::ReadAccess<u64>((u8*)pointer, offset);
  } else {
    switch (paddr) {
      case 0x00000000 ... 0x007FFFFF:
        return Util::ReadAccess<u64>(mmio.rdp.rdram, paddr);
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
        return Util::ReadAccess<u64>(cart, paddr & romMask);
      case 0x1FC00000 ... 0x1FC007BF:
        return Util::ReadAccess<u64>(si.pif.pifBootrom, paddr - 0x1FC00000);
      case PIF_RAM_REGION:
        return be64toh(Util::ReadAccess<u64>(si.pif.pifRam, paddr - PIF_RAM_REGION_START));
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

void Mem::Write8(Registers& regs, n64::Dynarec& dyn, u32 paddr, u32 val) {
  dyn.InvalidatePage(BYTE_ADDRESS(paddr));
  return Write8(regs, paddr, val);
}

void Mem::Write16(Registers& regs, n64::Dynarec& dyn, u32 paddr, u32 val) {
  dyn.InvalidatePage(HALF_ADDRESS(paddr));
  return Write16(regs, paddr, val);
}

void Mem::Write32(Registers& regs, n64::Dynarec& dyn, u32 paddr, u32 val) {
  dyn.InvalidatePage(paddr);
  return Write32(regs, paddr, val);
}

void Mem::Write64(Registers& regs, n64::Dynarec& dyn, u32 paddr, u64 val) {
  dyn.InvalidatePage(paddr);
  return Write64(regs, paddr, val);
}

void Mem::Write8(Registers& regs, u32 paddr, u32 val) {
  const auto page = paddr >> 12;
  auto offset = paddr & 0xFFF;
  const auto pointer = writePages[page];
  SI& si = mmio.si;

  if(pointer) {
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
      case 0x10000000 ... 0x1FBFFFFF:
        break;
      case PIF_RAM_REGION:
        val = val << (8 * (3 - (paddr & 3)));
        paddr = (paddr - PIF_RAM_REGION_START) & ~3;
        Util::WriteAccess<u32>(si.pif.pifRam, paddr, htobe32(val));
        si.pif.ProcessPIFCommands(*this);
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

void Mem::Write16(Registers& regs, u32 paddr, u32 val) {
  const auto page = paddr >> 12;
  auto offset = paddr & 0xFFF;
  const auto pointer = writePages[page];
  SI& si = mmio.si;

  if(pointer) {
    Util::WriteAccess<u16>((u8*)pointer, HALF_ADDRESS(offset), val);
  } else {
    switch (paddr) {
      case 0x00000000 ... 0x007FFFFF:
        Util::WriteAccess<u16>(mmio.rdp.rdram, HALF_ADDRESS(paddr), val);
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
      case 0x10000000 ... 0x1FBFFFFF:
        break;
      case PIF_RAM_REGION:
        val = val << (16 * !(paddr & 2));
        paddr &= ~3;
        Util::WriteAccess<u32>(si.pif.pifRam, paddr - PIF_RAM_REGION_START, htobe32(val));
        si.pif.ProcessPIFCommands(*this);
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

void Mem::Write32(Registers& regs, u32 paddr, u32 val) {
  const auto page = paddr >> 12;
  auto offset = paddr & 0xFFF;
  const auto pointer = writePages[page];
  SI& si = mmio.si;

  if(pointer) {
    Util::WriteAccess<u32>((u8*)pointer, offset, val);
  } else {
    switch(paddr) {
      case 0x00000000 ... 0x007FFFFF:
        Util::WriteAccess<u32>(mmio.rdp.rdram, paddr, val);
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
      case 0x14000000 ... 0x1FBFFFFF: break;
      case PIF_RAM_REGION:
        Util::WriteAccess<u32>(si.pif.pifRam, paddr - PIF_RAM_REGION_START, htobe32(val));
        si.pif.ProcessPIFCommands(*this);
        break;
      case 0x00800000 ... 0x03FFFFFF: case 0x04200000 ... 0x042FFFFF:
      case 0x08000000 ... 0x0FFFFFFF: case 0x04900000 ... 0x07FFFFFF:
      case 0x1FC00800 ... 0x7FFFFFFF: case 0x80000000 ... 0xFFFFFFFF: break;
      default: Util::panic("Unimplemented 32-bit write at address {:08X} with value {:0X} (PC = {:016X})\n", paddr, val, (u64)regs.pc);
    }
  }
}

void Mem::Write64(Registers& regs, u32 paddr, u64 val) {
  const auto page = paddr >> 12;
  auto offset = paddr & 0xFFF;
  const auto pointer = writePages[page];
  SI& si = mmio.si;

  if(pointer) {
    Util::WriteAccess<u64>((u8*)pointer, offset, val);
  } else {
    switch (paddr) {
      case 0x00000000 ... 0x007FFFFF:
        Util::WriteAccess<u64>(mmio.rdp.rdram, paddr, val);
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
      case 0x10000000 ... 0x1FBFFFFF:
        break;
      case PIF_RAM_REGION:
        Util::WriteAccess<u64>(si.pif.pifRam, paddr - PIF_RAM_REGION_START, htobe64(val));
        si.pif.ProcessPIFCommands(*this);
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
}