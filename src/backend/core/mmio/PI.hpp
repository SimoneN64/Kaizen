#pragma once
#include <common.hpp>
#include <core/mmio/Interrupt.hpp>

namespace n64 {

struct Mem;
struct Registers;

struct PI {
  PI();
  void Reset();
  auto Read(MI&, u32) const -> u32;
  void Write(Mem&, Registers&, u32, u32);
  auto BusRead8(Mem&, u32) -> u8;
  void BusWrite8(Mem&, u32, u32);
  auto BusRead16(Mem&, u32) -> u16;
  void BusWrite16(Mem&, u32, u16);
  auto BusRead32(Mem&, u32) -> u32;
  void BusWrite32(Mem&, u32, u32);
  auto BusRead64(Mem&, u32) -> u64;
  void BusWrite64(Mem&, u32, u64);
  bool ReadLatch();
  bool WriteLatch(u32 val);
  bool dmaBusy{}, ioBusy{}, toCart{};
  u32 latch;
  u32 dramAddr{}, cartAddr{}, dramAddrInternal{}, cartAddrInternal{};
  u32 rdLen{}, wrLen{};
  u32 pi_bsd_dom1_lat{}, pi_bsd_dom2_lat{};
  u32 pi_bsd_dom1_pwd{}, pi_bsd_dom2_pwd{};
  u32 pi_bsd_dom1_pgs{}, pi_bsd_dom2_pgs{};
  u32 pi_bsd_dom1_rls{}, pi_bsd_dom2_rls{};
};
}