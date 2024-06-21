#pragma once
#include <common.hpp>

namespace n64 {

struct Mem;
struct Registers;

struct PI {
  PI(Mem&, Registers&);
  void Reset();
  auto Read(u32) const -> u32;
  void Write(u32, u32);

  template<typename T, bool isDma>
  void BusWrite(u32, u32);
  template<bool isDma>
  void BusWrite(u32, u64);

  template<typename T, bool isDma>
  auto BusRead(u32) -> T;

  bool ReadLatch();
  bool WriteLatch(u32 val);

  static u8 GetDomain(u32 address);
  [[nodiscard]] u32 AccessTiming(u8 domain, u32 length) const;
  bool dmaBusy{}, ioBusy{}, toCart{};
  u32 latch{};
  u32 dramAddr{}, cartAddr{};
  u32 rdLen{}, wrLen{};
  u32 piBsdDom1Lat{}, piBsdDom2Lat{};
  u32 piBsdDom1Pwd{}, piBsdDom2Pwd{};
  u32 piBsdDom1Pgs{}, piBsdDom2Pgs{};
  u32 piBsdDom1Rls{}, piBsdDom2Rls{};
private:
  Mem& mem;
  Registers& regs;
};
}