#pragma once
#include <ParallelRDPWrapper.hpp>
#include <backend/core/Interpreter.hpp>
#include <backend/core/JIT.hpp>
#include <string>

namespace n64 {
struct Core {
  explicit Core();
  void Stop();
  void LoadROM(const std::string &);
  [[nodiscard]] bool LoadTAS(const fs::path &) const;
  void Run(float volumeL, float volumeR);
  void Serialize();
  void Deserialize();
  void TogglePause() { pause = !pause; }
  [[nodiscard]] VI &GetVI() const { return cpu->GetMem().mmio.vi; }

  u32 breakpoint = 0;

  bool pause = true;
  bool render = false;
  u32 cycles = 0;
  bool romLoaded = false;
  std::string rom;
  std::unique_ptr<BaseCPU> cpu;
  std::vector<u8> serialized[10]{};
  size_t memSize{}, cpuSize{}, verSize{};
  int slot = 0;
  ParallelRDP parallel;
};
} // namespace n64
