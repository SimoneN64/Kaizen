#pragma once
#include <nlohmann/json.hpp>
#include <Core.hpp>

using namespace nlohmann;

struct Settings {
  Settings(n64::Core&);
  ~Settings();

  float GetVolumeL() const { return volumeL; };
  float GetVolumeR() const { return volumeR; };
  bool GetLockChannels() const { return lockChannels; }
  std::string GetCpuType() const { return cpuType; }

  void RenderWidget(bool& show);
private:
  std::string cpuType = "interpreter";
  float volumeL = 0.0, volumeR = 0.0;
  bool lockChannels = true;
  json settings;
};
