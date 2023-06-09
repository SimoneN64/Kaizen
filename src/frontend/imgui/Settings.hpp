#pragma once
#include <nlohmann/json.hpp>
#include <common.hpp>

namespace n64 { struct Core; }
using namespace nlohmann;

struct Settings {
  Settings(n64::Core&);
  ~Settings();

  FORCE_INLINE float GetVolumeL() const { return volumeL; };
  FORCE_INLINE float GetVolumeR() const { return volumeR; };
  FORCE_INLINE bool GetLockChannels() const { return lockChannels; }

  void RenderWidget(bool& show);
private:
  float volumeL = 0.0, volumeR = 0.0;
  bool lockChannels = true;
  json settings;
};
