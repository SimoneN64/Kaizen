#include <Mem.hpp>
#include <fstream>
#include <core/registers/Registers.hpp>
#include <core/registers/Cop0.hpp>
#include <core/Interpreter.hpp>
#include <core/JIT.hpp>
#include <File.hpp>

namespace n64 {
Mem::Mem() {
  rom.cart = (u8*)calloc(CART_SIZE, 1);
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

  memset(rom.cart, 0, CART_SIZE);
  memset(sram, 0, SRAM_SIZE);
  mmio.Reset();
}

inline void SetROMCIC(u32 checksum, ROM& rom) {
  switch (checksum) {
    case 0xEC8B1325: rom.cicType = CIC_NUS_7102; break; // 7102
    case 0x1DEB51A9: rom.cicType = CIC_NUS_6101; break; // 6101
    case 0xC08E5BD6: rom.cicType = CIC_NUS_6102_7101; break;
    case 0x03B8376A: rom.cicType = CIC_NUS_6103_7103; break;
    case 0xCF7F41DC: rom.cicType = CIC_NUS_6105_7105; break;
    case 0xD1059C6A: rom.cicType = CIC_NUS_6106_7106; break;
    default:
      Util::warn("Could not determine CIC TYPE! Checksum: 0x{:08X} is unknown!\n", checksum);
      rom.cicType = UNKNOWN_CIC_TYPE;
      break;
  }
}

void Mem::LoadROM(const std::string& filename) {
  std::ifstream file(filename, std::ios::binary);
  file.unsetf(std::ios::skipws);

  if(!file.is_open()) {
    Util::panic("Unable to open {}!", filename);
  }

  file.seekg(0, std::ios::end);
  size_t size = file.tellg();
  file.seekg(0, std::ios::beg);

  size_t sizeAdjusted = Util::NextPow2(size);
  rom.mask = sizeAdjusted - 1;
  u8* buf = (u8*)malloc(sizeAdjusted);
  file.read(reinterpret_cast<char*>(buf), size);
  file.close();

  u32 endianness = be32toh(*reinterpret_cast<u32*>(buf));
  Util::SwapN64Rom<true>(sizeAdjusted, buf, endianness);

  memcpy(rom.cart, buf, sizeAdjusted);
  rom.size = sizeAdjusted;
  memcpy(&rom.header, buf, sizeof(ROMHeader));
  memcpy(rom.gameNameCart, rom.header.imageName, sizeof(rom.header.imageName));

  rom.header.clockRate = be32toh(rom.header.clockRate);
  rom.header.programCounter = be32toh(rom.header.programCounter);
  rom.header.release = be32toh(rom.header.release);
  rom.header.crc1 = be32toh(rom.header.crc1);
  rom.header.crc2 = be32toh(rom.header.crc2);
  rom.header.unknown = be64toh(rom.header.unknown);
  rom.header.unknown2 = be32toh(rom.header.unknown2);
  rom.header.manufacturerId = be32toh(rom.header.manufacturerId);
  rom.header.cartridgeId = be16toh(rom.header.cartridgeId);

  rom.code[0] = rom.header.manufacturerId & 0xFF;
  rom.code[1] = (rom.header.cartridgeId >> 8) & 0xFF;
  rom.code[2] = rom.header.cartridgeId & 0xFF;
  rom.code[3] = '\0';

  for (int i = sizeof(rom.header.imageName) - 1; rom.gameNameCart[i] == ' '; i--) {
    rom.gameNameCart[i] = '\0';
  }

  u32 checksum = Util::crc32(0, &rom.cart[0x40], 0x9c0);
  SetROMCIC(checksum, rom);
  endianness = be32toh(*reinterpret_cast<u32*>(rom.cart));
  Util::SwapN64Rom(sizeAdjusted, rom.cart, endianness);
  rom.pal = IsROMPAL();
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
        return rom.cart[BYTE_ADDRESS(paddr) & rom.mask];
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
        return Util::ReadAccess<u16>(rom.cart, HALF_ADDRESS(paddr) & rom.mask);
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
        return Util::ReadAccess<u32>(rom.cart, paddr & rom.mask);
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
        return Util::ReadAccess<u64>(rom.cart, paddr & rom.mask);
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

void Mem::Write8(Registers& regs, JIT& dyn, u32 paddr, u32 val) {
  dyn.InvalidatePage(BYTE_ADDRESS(paddr));
  return Write8(regs, paddr, val);
}

void Mem::Write16(Registers& regs, JIT& dyn, u32 paddr, u32 val) {
  dyn.InvalidatePage(HALF_ADDRESS(paddr));
  return Write16(regs, paddr, val);
}

void Mem::Write32(Registers& regs, JIT& dyn, u32 paddr, u32 val) {
  dyn.InvalidatePage(paddr);
  return Write32(regs, paddr, val);
}

void Mem::Write64(Registers& regs, JIT& dyn, u32 paddr, u64 val) {
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
        Util::debug("SRAM 8 bit write {:02X}\n", val);
        break;
      case 0x04900000 ... 0x07FFFFFF:
      case 0x1FC00000 ... 0x1FC007BF:
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
      case 0x1FC00000 ... 0x1FC007BF:
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
      case 0x1FC00000 ... 0x1FC007BF: case 0x1FC00800 ... 0x7FFFFFFF:
      case 0x80000000 ... 0xFFFFFFFF: break;
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
      case 0x1FC00000 ... 0x1FC007BF:
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