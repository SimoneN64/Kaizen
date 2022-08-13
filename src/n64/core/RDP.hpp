#pragma once
#include <common.hpp>

namespace n64 {

struct RSP;
struct MI;
struct Registers;

union DPCStatusWrite {
  u32 raw;
  struct {
    unsigned clearXbusDmemDma:1;
    unsigned setXbusDmemDma:1;
    unsigned clearFreeze:1;
    unsigned setFreeze:1;
    unsigned clearFlush:1;
    unsigned setFlush:1;
    unsigned clearTmem:1;
    unsigned clearPipe:1;
    unsigned clearCmd:1;
    unsigned clearClock:1;
  };
};

union DPCStatus {
  struct {
    unsigned xbusDmemDma;
    unsigned freeze;
    unsigned flush;
    unsigned startGclk;
    unsigned tmemBusy;
    unsigned pipeBusy;
    unsigned cmdBusy;
    unsigned cbufReady;
    unsigned dmaBusy;
    unsigned endValid;
    unsigned startValid;
  };
  u32 raw;
};

struct DPC {
  DPCStatus status;
  u32 start;
  u32 current;
  u32 end;
};

struct RDP {
  DPC dpc{.status{.raw = 0x80}};
  u32 cmd_buf[0xFFFFF]{};

  RDP();
  void Reset();

  auto Read(u32 addr) const -> u32;
  void Write(u32 addr, u32 val);
  void StatusWrite(u32 val);
  void RunCommand(MI& mi, Registers& regs, RSP& rsp);
  void OnFullSync();
};
} // natsukashii
