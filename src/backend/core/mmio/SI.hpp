#pragma once
#include <common.hpp>
#include <core/mmio/Interrupt.hpp>
#include <core/mmio/MI.hpp>
#include <core/mmio/PIF.hpp>

namespace n64 {

union SIStatus {
  u32 raw{};
  struct {
    unsigned dmaBusy:1;
    unsigned ioBusy:1;
    unsigned reserved:1;
    unsigned dmaErr:1;
    unsigned:8;
    unsigned intr:1;
  };
};

struct Mem;

struct SI {
  SI();
  void Reset();
  SIStatus status{};
  u32 dramAddr{};
  u32 pifAddr{};
  bool toDram = false;

  auto Read(MI&, u32) const -> u32;
  void Write(Mem&, Registers&, u32, u32);
  void DMA(Mem&, Registers&) const;
  PIF pif;
};

// just to silence warning, it is used.
[[maybe_unused]] static void DMA(Mem& mem, Registers& regs);
#define SI_DMA_DELAY (65536 * 2)
}