#pragma once
#include <common.hpp>
#include <vector>

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
    unsigned xbusDmemDma:1;
    unsigned freeze:1;
    unsigned flush:1;
    unsigned startGclk:1;
    unsigned tmemBusy:1;
    unsigned pipeBusy:1;
    unsigned cmdBusy:1;
    unsigned cbufReady:1;
    unsigned dmaBusy:1;
    unsigned endValid:1;
    unsigned startValid:1;
  };
  u32 raw;
};

struct DPC {
  DPCStatus status;
  u32 start;
  u32 current;
  u32 end;
  u32 clock;
  u32 tmem;
};

struct RDP {
  DPC dpc;
  u32 cmd_buf[0xFFFFF]{};

  RDP();
  void Reset();

  std::vector<u8> dram;
  [[nodiscard]] auto Read(u32 addr) const -> u32;
  void Write(MI& mi, Registers& regs, RSP& rsp, u32 addr, u32 val);
  void WriteStatus(MI& mi, Registers& regs, RSP& rsp, u32 val);
  void RunCommand(MI& mi, Registers& regs, RSP& rsp);
  void OnFullSync(MI& mi, Registers& regs);

  inline void WriteStart(u32 val) {
    if(!dpc.status.startValid) {
      dpc.start = val & 0xFFFFF8;
    }
    dpc.status.startValid = true;
  }

  inline void WriteEnd(MI& mi, Registers& regs, RSP& rsp, u32 val) {
    dpc.end = val & 0xFFFFF8;
    if(dpc.status.startValid) {
      dpc.current = dpc.start;
      dpc.status.startValid = false;
    }
    RunCommand(mi, regs, rsp);
  }
};
} // n64
