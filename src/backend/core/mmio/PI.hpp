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
  u32 dramAddr{}, cartAddr{};
  u32 rdLen{}, wrLen{};
  u32 pi_bsd_dom1_lat{}, pi_bsd_dom2_lat{};
  u32 pi_bsd_dom1_pwd{}, pi_bsd_dom2_pwd{};
  u32 pi_bsd_dom1_pgs{}, pi_bsd_dom2_pgs{};
  u32 pi_bsd_dom1_rls{}, pi_bsd_dom2_rls{};
};
}