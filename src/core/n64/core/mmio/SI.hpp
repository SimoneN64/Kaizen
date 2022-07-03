#pragma once
#include <common.hpp>
#include <n64/core/mmio/Interrupt.hpp>
#include <n64/core/mmio/MI.hpp>
#include <n64/core/mmio/PIF.hpp>

namespace natsukashii::n64::core {

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
  SI() = default;
  SIStatus status{};
  u32 dramAddr{};
  Controller controller{};

  auto Read(MI&, u32) const -> u32;
  void Write(Mem&, Registers&, u32, u32);
};
}