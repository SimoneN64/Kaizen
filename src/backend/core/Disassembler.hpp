#pragma once
#include <capstone/capstone.h>
#include <utils/log.hpp>
#include <utils/MemoryHelpers.hpp>
#include <array>

struct Disassembler {
  struct DisassemblyResult {
    bool success = false;
    std::string full;
    u64 address;
    std::string mnemonic;
    std::array<std::string, 3> ops{};
  };

  static Disassembler &instance(bool rsp = false) {
    static Disassembler ret(rsp);
    return ret;
  }

  DisassemblyResult Disassemble(const u32 address, const u32 instruction) {
    return details ? DisassembleDetailed(address, instruction) : DisassembleSimple(address, instruction);
  }

  ~Disassembler() { cs_close(&handle); }

private:
  DisassemblyResult DisassembleDetailed(u32 address, u32 instruction);
  DisassemblyResult DisassembleSimple(u32 address, u32 instruction);

  explicit Disassembler(const bool rsp) : rsp(rsp) {
    if (cs_open(CS_ARCH_MIPS, static_cast<cs_mode>((rsp ? CS_MODE_32 : CS_MODE_64) | CS_MODE_BIG_ENDIAN), &handle) !=
        CS_ERR_OK) {
      Util::panic("Could not initialize {} disassembler!", rsp ? "RSP" : "CPU");
    }

    if (cs_option(handle, CS_OPT_DETAIL, CS_OPT_ON) != CS_ERR_OK) {
      Util::warn("Could not enable disassembler's details!");
      details = false;
    }
  }

  bool rsp = false;
  bool details = true;
  csh handle{};
};
