#pragma once
#include <nlohmann/json.hpp>
#include <Core.hpp>

using namespace nlohmann;

struct Settings {
  Settings(n64::Core&);
  ~Settings();

  inline float GetVolumeL() const { return volumeL; };
  inline float GetVolumeR() const { return volumeR; };
  inline bool GetLockChannels() const { return lockChannels; }

  void RenderWidget(bool& show);
private:
  float volumeL = 0.0, volumeR = 0.0;
  bool lockChannels = true;
  json settings;
};
