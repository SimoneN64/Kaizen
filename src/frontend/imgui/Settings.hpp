#pragma once
#include <nlohmann/json.hpp>
#include <common.hpp>

namespace n64 { struct Core; }
using namespace nlohmann;

struct Settings {
  Settings();
  ~Settings();

  [[nodiscard]] FORCE_INLINE float GetVolumeL() const { return volumeL; };
  [[nodiscard]] FORCE_INLINE float GetVolumeR() const { return volumeR; };
  [[nodiscard]] FORCE_INLINE bool GetLockChannels() const { return lockChannels; }

  void RenderWidget(bool& show);
private:
  float volumeL = 0.0, volumeR = 0.0;
  bool lockChannels = true;
  bool mute = false;
  json settings;
};
