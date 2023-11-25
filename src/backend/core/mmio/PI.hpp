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
  template<typename T, bool isDma>
  auto BusRead(Mem&, u32) -> T;
  template<typename T, bool isDma>
  void BusWrite(Mem&, u32, T);
  bool ReadLatch();
  bool WriteLatch(u32 val);

  static u8 GetDomain(u32 address);
  u32 AccessTiming(u8 domain, u32 length) const;
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