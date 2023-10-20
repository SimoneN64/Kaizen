#include <Settings.hpp>
#include <fstream>
#include <filesystem>
#include <Widgets.hpp>
#include <Core.hpp>

namespace fs = std::filesystem;

#define checknestedjsonentry(name, type, param1, param2, defaultVal) \
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

#define checkjsonentry(name, type, param, defaultVal) \
  do { \
    auto name##Entry = settings[param];  \
    if(!name##Entry.empty()) {                    \
      auto value = name##Entry.get<type>();       \
      (name) = value;                             \
    } else {                                      \
      settingsFile.clear();                       \
      settings[param] = defaultVal;               \
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

    checknestedjsonentry(oldVolumeL, float, "audio", "volumeL", 0.5);
    checknestedjsonentry(oldVolumeR, float, "audio", "volumeR", 0.5);
    checknestedjsonentry(mute, bool, "audio", "mute", false);
    volumeL = mute ? 0 : oldVolumeL;
    volumeR = mute ? 0 : oldVolumeR;
    checknestedjsonentry(lockChannels, bool, "audio", "lockChannels", true);
    checknestedjsonentry(jit, bool, "cpu", "enableJIT", false);
    checkjsonentry(selectedLanguage, int, "language", Language::ENGLISH);
  } else {
    settingsFile = std::fstream("resources/settings.json", std::fstream::trunc | std::fstream::in | std::fstream::out);
    settings["audio"]["volumeR"] = 0.5;
    settings["audio"]["volumeL"] = 0.5;
    settings["audio"]["lockChannels"] = true;
    settings["audio"]["mute"] = false;
    settings["cpu"]["enableJIT"] = false;
    settings["language"] = Language::ENGLISH;

    oldVolumeR = volumeR = 0.5;
    oldVolumeL = volumeL = 0.5;
    lockChannels = true;
    mute = false;
    jit = false;

    settingsFile << settings;
  }

  Language::SetLanguage(languageStrings, selectedLanguage);

  if(jit) {
    core.cpu = std::make_unique<n64::JIT>();
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
  settings["language"] = selectedLanguage;
  settingsFile << settings;

  settingsFile.close();
}

void Settings::RenderWidget(bool& show) {
  if(show) {
    ImGui::OpenPopup("Settings");
    if(ImGui::BeginPopupModal(languageStrings[Language::EMULATION_ITEM_SETTINGS].c_str(), &show)) {
      enum class SelectedSetting { CPU, Audio, Interface, COUNT };
      static SelectedSetting selectedSetting = SelectedSetting::CPU;
      const char *categories[(int)SelectedSetting::COUNT] = {
        languageStrings[Language::SETTINGS_CATEGORY_CPU].c_str(),
        languageStrings[Language::SETTINGS_CATEGORY_AUDIO].c_str(),
        languageStrings[Language::SETTINGS_CATEGORY_INTERFACE].c_str() };

      CreateComboList("##", (int*)&selectedSetting, categories, (int)SelectedSetting::COUNT);
      ImGui::Separator();
      switch (selectedSetting) {
        case SelectedSetting::Audio:
            ImGui::Checkbox(languageStrings[Language::SETTINGS_OPTION_LOCK_CHANNELS].c_str(), &lockChannels);
            ImGui::Checkbox(languageStrings[Language::SETTINGS_OPTION_MUTE].c_str(), &mute);
            if(mute) {
              volumeL = 0;
              volumeR = 0;

              ImGui::BeginDisabled();
              ImGui::SliderFloat(languageStrings[Language::SETTINGS_OPTION_VOLUME_L].c_str(), &oldVolumeL, 0, 1, "%.2f", ImGuiSliderFlags_NoInput);
              if (lockChannels) {
                oldVolumeR = oldVolumeL;
              }
              ImGui::SliderFloat(languageStrings[Language::SETTINGS_OPTION_VOLUME_R].c_str(), &oldVolumeR, 0, 1, "%.2f", ImGuiSliderFlags_NoInput);
              ImGui::EndDisabled();
            }
            else {
              volumeL = oldVolumeL;
              volumeR = oldVolumeR;

              ImGui::SliderFloat(languageStrings[Language::SETTINGS_OPTION_VOLUME_L].c_str(), &volumeL, 0, 1, "%.2f", ImGuiSliderFlags_NoInput);
              if (!lockChannels) {
                ImGui::SliderFloat(languageStrings[Language::SETTINGS_OPTION_VOLUME_R].c_str(), &volumeR, 0, 1, "%.2f", ImGuiSliderFlags_NoInput);
              }
              else {
                volumeR = volumeL;
                ImGui::BeginDisabled();
                ImGui::SliderFloat(languageStrings[Language::SETTINGS_OPTION_VOLUME_R].c_str(), &volumeR, 0, 1, "%.2f", ImGuiSliderFlags_NoInput);
                ImGui::EndDisabled();
              }

              oldVolumeL = volumeL;
              oldVolumeR = volumeR;
            }

            break;
        case SelectedSetting::CPU:
          ImGui::Checkbox(languageStrings[Language::SETTINGS_OPTION_ENABLE_JIT].c_str(), &jit);
          break;
        case SelectedSetting::Interface: {
          const char* languages[Language::AVAILABLE_LANGS_COUNT] = {
            Language::languages[0].c_str(),
            Language::languages[1].c_str()
          };
          CreateComboList("##", &selectedLanguage, languages, Language::AVAILABLE_LANGS_COUNT);
        } break;
        case SelectedSetting::COUNT:
          Util::panic("BRUH");
      }

      ImGui::EndPopup();
    }
  }
}