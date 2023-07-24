#include <Settings.hpp>
#include <fstream>
#include <filesystem>
#include <Widgets.hpp>
#include <Core.hpp>

namespace fs = std::filesystem;

#define checkjsonentry(name, type, param1, param2, defaultVal) \
  do { \
    auto name##Entry = settings[param1][param2];  \
    if(!name##Entry.empty()) {                    \
      auto value = name##Entry.get<type>();       \
      (name) = value;                             \
    } else {                                      \
      settingsFile.clear();                       \
      settings[param1][param2] = defaultVal;      \
      settingsFile << settings;                   \
      (name) = defaultVal;                        \
    }                                             \
  } while(0)


Settings::Settings(n64::Core& core) {
  auto fileExists = fs::exists("resources/settings.json");
  std::fstream settingsFile;
  if(fileExists) {
    settingsFile = std::fstream("resources/settings.json", std::fstream::in | std::fstream::out);
    settings = json::parse(settingsFile);

    checkjsonentry(oldVolumeL, float, "audio", "volumeL", 0.5);
    volumeL = oldVolumeL;
    checkjsonentry(oldVolumeR, float, "audio", "volumeR", 0.5);
    volumeR = oldVolumeR;
    checkjsonentry(mute, bool, "audio", "mute", false);
    checkjsonentry(lockChannels, bool, "audio", "lockChannels", true);
    checkjsonentry(jit, bool, "cpu", "enableJIT", false);
  } else {
    settingsFile = std::fstream("resources/settings.json", std::fstream::trunc | std::fstream::in | std::fstream::out);
    settings["audio"]["volumeR"] = 0.5;
    settings["audio"]["volumeL"] = 0.5;
    settings["audio"]["lockChannels"] = true;
    settings["audio"]["mute"] = false;
    settings["cpu"]["enableJIT"] = false;

    oldVolumeR = volumeR = 0.5;
    oldVolumeL = volumeL = 0.5;
    lockChannels = true;
    mute = false;
    jit = false;

    settingsFile << settings;
  }

  if(jit) {
    Util::panic("JIT is unimplemented!");
  } else {
    core.cpu = std::make_unique<n64::Interpreter>();
  }
  settingsFile.close();
}

Settings::~Settings() {
  auto fileExists = fs::exists("resources/settings.json");
  std::fstream settingsFile;
  if(fileExists) {
    settingsFile = std::fstream("resources/settings.json", std::fstream::trunc | std::fstream::out);
  } else {
    settingsFile = std::fstream("resources/settings.json", std::fstream::out);
  }

  settings["audio"]["volumeR"] = oldVolumeR;
  settings["audio"]["volumeL"] = oldVolumeL;
  settings["audio"]["lockChannels"] = lockChannels;
  settings["audio"]["mute"] = mute;
  settings["cpu"]["enableJIT"] = jit;
  settingsFile << settings;

  settingsFile.close();
}

void Settings::RenderWidget(bool& show) {
  if(show) {
    ImGui::OpenPopup("Settings");
    if(ImGui::BeginPopupModal("Settings", &show)) {
      enum class SelectedSetting { CPU, Audio, COUNT };
      static SelectedSetting selectedSetting = SelectedSetting::CPU;
      const char *categories[(int)SelectedSetting::COUNT] = { "CPU", "Audio" };
      CreateComboList("##", (int*)&selectedSetting, categories, (int)SelectedSetting::COUNT);
      ImGui::Separator();
      switch (selectedSetting) {
        case SelectedSetting::Audio:
            ImGui::Checkbox("Lock channels", &lockChannels);
            ImGui::Checkbox("Mute", &mute);
            if(mute) {
              volumeL = 0;
              volumeR = 0;

              ImGui::BeginDisabled();
              ImGui::SliderFloat("Volume L", &oldVolumeL, 0, 1, "%.2f", ImGuiSliderFlags_NoInput);
              if (lockChannels) {
                oldVolumeR = oldVolumeL;
              }
              ImGui::SliderFloat("Volume R", &oldVolumeR, 0, 1, "%.2f", ImGuiSliderFlags_NoInput);
              ImGui::EndDisabled();
            } else {
              volumeL = oldVolumeL;
              volumeR = oldVolumeR;

              ImGui::SliderFloat("Volume L", &volumeL, 0, 1, "%.2f", ImGuiSliderFlags_NoInput);
              if (!lockChannels) {
                ImGui::SliderFloat("Volume R", &volumeR, 0, 1, "%.2f", ImGuiSliderFlags_NoInput);
              } else {
                volumeR = volumeL;
                ImGui::BeginDisabled();
                ImGui::SliderFloat("Volume R", &volumeR, 0, 1, "%.2f", ImGuiSliderFlags_NoInput);
                ImGui::EndDisabled();
              }

              oldVolumeL = volumeL;
              oldVolumeR = volumeR;
            }

            break;
        case SelectedSetting::CPU:
          ImGui::Checkbox("Enable JIT", &jit);
          break;
        case SelectedSetting::COUNT:
          Util::panic("BRUH");
      }

      ImGui::EndPopup();
    }
  }
}