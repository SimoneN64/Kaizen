#include <core/MMIO.hpp>
#include <log.hpp>
#include <core/Mem.hpp>
#include <core/registers/Registers.hpp>

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
    case RSP_REGION: return rsp.Read(addr);
    case RDP_REGION: return rdp.Read(addr);
    case MI_REGION: return mi.Read(addr);
    case VI_REGION: return vi.Read(addr);
    case AI_REGION: return ai.Read(addr);
    case PI_REGION: return pi.Read(mi, addr);
    case RI_REGION: return ri.Read(addr);
    case SI_REGION: return si.Read(mi, addr);
    default:
      Util::panic("Unhandled mmio read at addr {:08X}", addr);
  }
}

void MMIO::Write(Mem& mem, Registers& regs, u32 addr, u32 val) {
  switch (addr) {
    case RSP_REGION: rsp.Write(mem, regs, addr, val); break;
    case RDP_REGION: rdp.Write(mi, regs, rsp, addr, val); break;
    case MI_REGION: mi.Write(regs, addr, val); break;
    case VI_REGION: vi.Write(mi, regs, addr, val); break;
    case AI_REGION: ai.Write(mem, regs, addr, val); break;
    case PI_REGION: pi.Write(mem, regs, addr, val); break;
    case RI_REGION: ri.Write(addr, val); break;
    case SI_REGION: si.Write(mem, regs, addr, val); break;
    default:
      Util::panic("Unhandled mmio write at addr {:08X} with val {:08X}", addr, val);
  }
}

std::vector<u8> MMIO::Serialize() {
  std::vector<u8> res{};

  auto sPIF = si.pif.Serialize();
  constexpr u32 rdpSize = sizeof(DPC) +
                          0xFFFFF +
                          RDRAM_SIZE;
  res.resize(
    rdpSize +
    sizeof(RSP) +
    sizeof(MI) +
    sizeof(VI) +
    sizeof(SI) +
    sizeof(PI) +
    sizeof(RI) +
    sizeof(AI) +
    sizeof(u32)*2 +
    sizeof(SIStatus));

  u32 index = 0;
  memcpy(res.data(), &rsp, sizeof(RSP));
  index += sizeof(RSP);
  memcpy(res.data() + index, &rdp.dpc, sizeof(DPC));
  index += sizeof(DPC);
  memcpy(res.data() + index, rdp.cmd_buf, 0xFFFFF);
  index += 0xFFFFF;
  memcpy(res.data() + index, rdp.rdram, RDRAM_SIZE);
  index += RDRAM_SIZE;
  memcpy(res.data() + index, &mi, sizeof(MI));
  index += sizeof(MI);
  memcpy(res.data() + index, &vi, sizeof(VI));
  index += sizeof(VI);
  memcpy(res.data() + index, &ai, sizeof(AI));
  index += sizeof(AI);
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
  memcpy(rdp.rdram, data.data() + index, RDRAM_SIZE);
  index += RDRAM_SIZE;
  memcpy(&mi, data.data() + index, sizeof(MI));
  index += sizeof(MI);
  memcpy(&vi, data.data() + index, sizeof(VI));
  index += sizeof(VI);
  memcpy(&ai, data.data() + index, sizeof(AI));
  index += sizeof(AI);
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
}
