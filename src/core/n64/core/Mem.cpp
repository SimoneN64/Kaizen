#include <Mem.hpp>
#include <fstream>
#include <util.hpp>
#include <n64/core/cpu/Registers.hpp>
#include <Cop0.hpp>
#include <n64/core/Cpu.hpp>

namespace natsukashii::n64::core {
Mem::Mem() {
  rdram.resize(RDRAM_SIZE);
  sram.resize(SRAM_SIZE);
}

void Mem::LoadROM(const std::string& filename) {
  std::ifstream file(filename, std::ios::binary);
  file.unsetf(std::ios::skipws);

  if(!file.is_open()) {
    util::panic("Unable to open {}!", filename);
  }

  file.seekg(std::ios::end);
  auto size = file.tellg();
  auto size_adjusted = util::NextPow2(size);
  romMask = size_adjusted - 1;
  file.seekg(std::ios::beg);

  std::vector<u8> rom;
  rom.reserve(size_adjusted);
  file.read(reinterpret_cast<char*>(rom.data()), size);

  file.close();
  util::SwapN64Rom(size, rom.data());
  memcpy(dmem, rom.data(), 0x1000);
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
    case 0x00000000 ... 0x007FFFFF: return util::ReadAccess<T>(rdram.data(), paddr & RDRAM_DSIZE);
    case 0x04000000 ... 0x04000FFF: return util::ReadAccess<T>(mmio.rsp.dmem, paddr & DMEM_DSIZE);
    case 0x04001000 ... 0x04001FFF: return util::ReadAccess<T>(mmio.rsp.imem, paddr & IMEM_DSIZE);
    case 0x04040000 ... 0x040FFFFF: case 0x04100000 ... 0x041FFFFF:
    case 0x04300000 ...	0x044FFFFF: case 0x04500000 ... 0x048FFFFF: return mmio.Read(paddr);
    case 0x10000000 ... 0x1FBFFFFF: return util::ReadAccess<T>(cart.data(), paddr & romMask);
    case 0x1FC00000 ... 0x1FC007BF: return util::ReadAccess<T>(pifBootrom, paddr & PIF_BOOTROM_DSIZE);
    case 0x1FC007C0 ... 0x1FC007FF: return util::ReadAccess<T>(pifRam, paddr & PIF_RAM_DSIZE);
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

template <class T, bool tlb>
void Mem::Write(Registers& regs, u32 vaddr, T val, s64 pc) {
  u32 paddr = vaddr;
  if(!MapVAddr<tlb>(regs, STORE, vaddr, paddr)) {
    HandleTLBException(regs, vaddr);
    FireException(regs, GetTLBExceptionCode(regs.cop0.tlbError, STORE), 0, pc);
  }

  switch(paddr) {
    case 0x00000000 ... 0x007FFFFF: util::WriteAccess<T>(rdram.data(), paddr & RDRAM_DSIZE, val); break;
    case 0x04000000 ... 0x04000FFF: util::WriteAccess<T>(mmio.rsp.dmem, paddr & DMEM_DSIZE, val); break;
    case 0x04001000 ... 0x04001FFF: util::WriteAccess<T>(mmio.rsp.imem, paddr & IMEM_DSIZE, val); break;
    case 0x04040000 ... 0x040FFFFF: case 0x04100000 ... 0x041FFFFF:
    case 0x04300000 ...	0x044FFFFF: case 0x04500000 ... 0x048FFFFF: mmio.Read(paddr); break;
    case 0x10000000 ... 0x1FBFFFFF: util::WriteAccess<T>(cart.data(), paddr & romMask, val); break;
    case 0x1FC00000 ... 0x1FC007BF: util::WriteAccess<T>(pifBootrom, paddr & PIF_BOOTROM_DSIZE, val); break;
    case 0x1FC007C0 ... 0x1FC007FF: util::WriteAccess<T>(pifRam, paddr & PIF_RAM_DSIZE, val); break;
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
}