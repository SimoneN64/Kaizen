#include <core/MMIO.hpp>
#include <core/Mem.hpp>

namespace n64 {
void MMIO::Reset() {
  rsp.Reset();
  rdp.Reset();
  mi.Reset();
  vi.Reset();
  ai.Reset();
  pi.Reset();
  ri.Reset();
  si.Reset();
}

u32 MMIO::Read(u32 addr) {
  switch (addr) {
  case RSP_REGION:
    return rsp.Read(addr);
  case RDP_REGION:
    return rdp.Read(addr);
  case MI_REGION:
    return mi.Read(addr);
  case VI_REGION:
    return vi.Read(addr);
  case AI_REGION:
    return ai.Read(addr);
  case PI_REGION:
    return pi.Read(addr);
  case RI_REGION:
    return ri.Read(addr);
  case SI_REGION:
    return si.Read(addr);
  default:
    Util::panic("Unhandled mmio read at addr {:08X}", addr);
  }
}

void MMIO::Write(const u32 addr, const u32 val) {
  switch (addr) {
  case RSP_REGION:
    rsp.Write(addr, val);
    break;
  case RDP_REGION:
    rdp.Write(addr, val);
    break;
  case MI_REGION:
    mi.Write(addr, val);
    break;
  case VI_REGION:
    vi.Write(addr, val);
    break;
  case AI_REGION:
    ai.Write(addr, val);
    break;
  case PI_REGION:
    pi.Write(addr, val);
    break;
  case RI_REGION:
    ri.Write(addr, val);
    break;
  case SI_REGION:
    si.Write(addr, val);
    break;
  default:
    Util::panic("Unhandled mmio write at addr {:08X} with val {:08X}", addr, val);
  }
}

std::vector<u8> MMIO::Serialize() {
  std::vector<u8> res{};

  auto sPIF = si.pif.Serialize();
  constexpr u32 rdpSize = sizeof(DPC) + 0xFFFFF + RDRAM_SIZE;
  res.resize(rdpSize + sizeof(RSP) + sizeof(MI) + sizeof(VI) + sizeof(SI) + sizeof(PI) + sizeof(RI) + sizeof(AI) +
             sizeof(u32) * 2 + sizeof(SIStatus));

  u32 index = 0;
  memcpy(res.data(), &rsp, sizeof(RSP));
  index += sizeof(RSP);
  memcpy(res.data() + index, &rdp.dpc, sizeof(DPC));
  index += sizeof(DPC);
  memcpy(res.data() + index, rdp.cmd_buf, 0xFFFFF);
  index += 0xFFFFF;
  std::ranges::copy(rdp.rdram, res.begin() + index);
  index += RDRAM_SIZE;
  memcpy(res.data() + index, &mi, sizeof(MI));
  index += sizeof(MI);
  memcpy(res.data() + index, &vi, sizeof(VI));
  index += sizeof(VI);
  memcpy(res.data() + index, &ai.dmaEnable, sizeof(ai.dmaEnable));
  index += sizeof(ai.dmaEnable);
  memcpy(res.data() + index, &ai.dacRate, sizeof(ai.dacRate));
  index += sizeof(ai.dacRate);
  memcpy(res.data() + index, &ai.bitrate, sizeof(ai.bitrate));
  index += sizeof(ai.bitrate);
  memcpy(res.data() + index, &ai.dmaCount, sizeof(ai.dmaCount));
  index += sizeof(ai.dmaCount);
  memcpy(res.data() + index, &ai.dmaLen, sizeof(ai.dmaLen));
  index += sizeof(ai.dmaLen);
  memcpy(res.data() + index, &ai.dmaAddr, sizeof(ai.dmaAddr));
  index += sizeof(ai.dmaAddr);
  memcpy(res.data() + index, &ai.dmaAddrCarry, sizeof(ai.dmaAddrCarry));
  index += sizeof(ai.dmaAddrCarry);
  memcpy(res.data() + index, &ai.cycles, sizeof(ai.cycles));
  index += sizeof(ai.cycles);
  memcpy(res.data() + index, &ai.dac, sizeof(ai.dac));
  index += sizeof(ai.dac);
  memcpy(res.data() + index, &pi, sizeof(PI));
  index += sizeof(PI);
  memcpy(res.data() + index, &ri, sizeof(RI));
  index += sizeof(RI);
  memcpy(res.data() + index, &si.dramAddr, sizeof(u32));
  index += sizeof(u32);
  memcpy(res.data() + index, &si.pifAddr, sizeof(u32));
  index += sizeof(u32);
  memcpy(res.data() + index, &si.status, sizeof(SIStatus));

  res.insert(res.end(), sPIF.begin(), sPIF.end());

  return res;
}

void MMIO::Deserialize(const std::vector<u8> &data) {
  u32 index = 0;
  memcpy(&rsp, data.data(), sizeof(RSP));
  index += sizeof(RSP);
  memcpy(&rdp.dpc, data.data() + index, sizeof(DPC));
  index += sizeof(DPC);
  memcpy(rdp.cmd_buf, data.data() + index, 0xFFFFF);
  index += 0xFFFFF;
  std::copy_n(data.begin() + index, RDRAM_SIZE, rdp.rdram.begin());
  index += RDRAM_SIZE;
  memcpy(&mi, data.data() + index, sizeof(MI));
  index += sizeof(MI);
  memcpy(&vi, data.data() + index, sizeof(VI));
  index += sizeof(VI);
  memcpy(&ai.dmaEnable, data.data() + index, sizeof(ai.dmaEnable));
  index += sizeof(ai.dmaEnable);
  memcpy(&ai.dacRate, data.data() + index, sizeof(ai.dacRate));
  index += sizeof(ai.dacRate);
  memcpy(&ai.bitrate, data.data() + index, sizeof(ai.bitrate));
  index += sizeof(ai.bitrate);
  memcpy(&ai.dmaCount, data.data() + index, sizeof(ai.dmaCount));
  index += sizeof(ai.dmaCount);
  memcpy(&ai.dmaLen, data.data() + index, sizeof(ai.dmaLen));
  index += sizeof(ai.dmaLen);
  memcpy(&ai.dmaAddr, data.data() + index, sizeof(ai.dmaAddr));
  index += sizeof(ai.dmaAddr);
  memcpy(&ai.dmaAddrCarry, data.data() + index, sizeof(ai.dmaAddrCarry));
  index += sizeof(ai.dmaAddrCarry);
  memcpy(&ai.cycles, data.data() + index, sizeof(ai.cycles));
  index += sizeof(ai.cycles);
  memcpy(&ai.dac, data.data() + index, sizeof(ai.dac));
  index += sizeof(ai.dac);
  memcpy(&pi, data.data() + index, sizeof(PI));
  index += sizeof(PI);
  memcpy(&ri, data.data() + index, sizeof(RI));
  index += sizeof(RI);
  memcpy(&si.dramAddr, data.data() + index, sizeof(u32));
  index += sizeof(u32);
  memcpy(&si.pifAddr, data.data() + index, sizeof(u32));
  index += sizeof(u32);
  memcpy(&si.status, data.data() + index, sizeof(SIStatus));
}
} // namespace n64
