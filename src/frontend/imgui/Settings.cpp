#include <Settings.hpp>
#include <fstream>
#include <filesystem>
#include <nfd.h>
#include <Widgets.hpp>
#include <Core.hpp>

namespace fs = std::filesystem;

#define checkjsonentry(name, type, param1, param2, defaultVal) \
  do { \
    auto name##Entry = settings[param1][param2];  \
    if(!name##Entry.empty()) {                    \
      auto value = name##Entry.get<type>();       \
      name = value;                               \
    } else {                                      \
      settingsFile.clear();                       \
      settings[param1][param2] = defaultVal;      \
      settingsFile << settings;                   \
      name = defaultVal;                          \
    }                                             \
  } while(0)


Settings::Settings(n64::Core& core) {
  auto fileExists = fs::exists("resources/settings.json");
  std::fstream settingsFile;
  if(fileExists) {
    settingsFile = std::fstream("resources/settings.json", std::fstream::in | std::fstream::out);
    settings = json::parse(settingsFile);

    checkjsonentry(volumeR, float, "audio", "volumeR", 0.5);
    checkjsonentry(volumeL, float, "audio", "volumeL", 0.5);
    checkjsonentry(lockChannels, bool, "audio", "lockChannels", true);
  } else {
    settingsFile = std::fstream("resources/settings.json", std::fstream::trunc | std::fstream::in | std::fstream::out);
    settings["audio"]["volumeR"] = 0.5;
    settings["audio"]["volumeL"] = 0.5;
    settings["audio"]["lockChannels"] = true;

    volumeR = 0.5;
    volumeL = 0.5;
    lockChannels = true;

    settingsFile << settings;
  }
  settingsFile.close();
}

Settings::~Settings() {
  auto fileExists = fs::exists("resources/settings.json");
  std::fstream settingsFile;
  if(fileExists) {
    settingsFile = std::fstream("resources/settings.json", std::fstream::trunc | std::fstream::out);

    settings["audio"]["volumeR"] = volumeR;
    settings["audio"]["volumeL"] = volumeL;
    settings["audio"]["lockChannels"] = lockChannels;
    settingsFile << settings;
  } else {
    settingsFile = std::fstream("resources/settings.json", std::fstream::out);

    settings["audio"]["volumeR"] = volumeR;
    settings["audio"]["volumeL"] = volumeL;
    settings["audio"]["lockChannels"] = lockChannels;
    settingsFile << settings;
  }

  settingsFile.close();
}

void Settings::RenderWidget(bool& show) {
  if(show) {
    ImGui::OpenPopup("Settings");
    if(ImGui::BeginPopupModal("Settings", &show)) {
      enum class SelectedSetting { Audio, COUNT };
      static SelectedSetting selectedSetting = SelectedSetting::Audio;
      const char *categories[(int)SelectedSetting::COUNT] = { "Audio" };
      CreateComboList("##", (int*)&selectedSetting, categories, (int)SelectedSetting::COUNT);
      ImGui::Separator();
      switch (selectedSetting) {
        case SelectedSetting::Audio:
            ImGui::Checkbox("Lock channels", &lockChannels);
            ImGui::SliderFloat("Volume L", &volumeL, 0, 1, "%.2f", ImGuiSliderFlags_NoInput);
            if (!lockChannels) {
              ImGui::SliderFloat("Volume R", &volumeR, 0, 1, "%.2f", ImGuiSliderFlags_NoInput);
            } else {
              volumeR = volumeL;
              ImGui::BeginDisabled();
              ImGui::SliderFloat("Volume R", &volumeR, 0, 1, "%.2f", ImGuiSliderFlags_NoInput);
              ImGui::EndDisabled();
            }
            break;
        case SelectedSetting::COUNT:
          Util::panic("BRUH");
      }

      ImGui::EndPopup();
    }
  }
}