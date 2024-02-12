#pragma once
#include <backend/core/Interpreter.hpp>
#include <string>
#include <set>

struct Window;
struct Event;

struct Breakpoint {
  u32 addr;
  bool ghost = false;
  bool operator<(const Breakpoint& rhs) const {
    return addr < rhs.addr;
  }

  bool operator==(const Breakpoint& rhs) const {
    return addr == rhs.addr;
  }
};

namespace n64 {
struct Core {
  ~Core() { Stop(); }
  Core();
  void Stop();
  void LoadROM(const std::string&);
  void Run(float volumeL, float volumeR);
  void Serialize();
  void Deserialize();
  void TogglePause() { pause = !pause; }
  bool isInstrJump(u32 addr);
  template <bool rsp = false>
  void Step();

  inline void insertGhostBkp(const Breakpoint& bkp) {
    if (isInstrJump(bkp.addr)) {
      auto pos = std::find(bkps.begin(), bkps.end(), bkp);
      if (pos == bkps.end()) {
        bkps.insert(bkp);
      }

      insertGhostBkp(Breakpoint{ bkp.addr + 8, true });
    }
  }

  inline void removeGhostBkp(const Breakpoint& bkp) {
    if (isInstrJump(bkp.addr)) {
      auto pos = std::find(bkps.begin(), bkps.end(), bkp);
      if (pos != bkps.end()) {
        bkps.erase(pos);
      }

      removeGhostBkp(Breakpoint{ bkp.addr + 8, true });
    }
  }

  inline void toggleBkp(const Breakpoint& bkp) {
    auto pos = std::find(bkps.begin(), bkps.end(), bkp);
    if (pos != bkps.end()) {
      bkps.erase(pos);
      removeGhostBkp(bkp);
    } else {
      bkps.insert(bkp);
      insertGhostBkp(bkp);
    }
  }

  inline bool hasToBreak(u32 addr) {
    return std::find_if(bkps.begin(), bkps.end(),
      [&addr](const Breakpoint& search) {
        return search.addr == addr;
      }) != bkps.end();
  }

  [[nodiscard]] VI& GetVI() const { return cpu->mem.mmio.vi; }

  std::set<Breakpoint> bkps{};

  bool broken = false;
  bool pause = true;
  bool render = false;
  u32 cycles = 0;
  bool romLoaded = false;
  std::string rom;
  std::unique_ptr<BaseCPU> cpu;
  std::vector<u8> serialized[10]{};
  size_t memSize{}, cpuSize{}, verSize{};
  int slot = 0;
};

extern u32 extraCycles;
void CpuStall(u32 cycles);
u32 PopStalledCycles();
}
