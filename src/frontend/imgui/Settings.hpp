#pragma once
#include <nlohmann/json.hpp>
#include <common.hpp>

namespace n64 { struct Core; }
using namespace nlohmann;

struct Settings {
  Settings(n64::Core& core);
  ~Settings();

  [[nodiscard]] FORCE_INLINE float GetVolumeL() const { return volumeL; };
  [[nodiscard]] FORCE_INLINE float GetVolumeR() const { return volumeR; };

  void RenderWidget(bool& show);
private:
  bool jit = false;
  float volumeL, volumeR;
  float oldVolumeL, oldVolumeR;
  bool lockChannels = true;
  bool mute = false;
  json settings;
};
