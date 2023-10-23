#include <Settings.hpp>
#include <fstream>
#include <filesystem>
#include <Widgets.hpp>
#include <Core.hpp>

namespace fs = std::filesystem;
#define GET_TRANSLATED_STRING(x) languageStrings[(x)]

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

void Settings::RenderWidget(const int& mWw, const int& mWh, bool& show) {
  if(show) {
    ImGui::OpenPopup("##settings");
    const float posX = (float)mWw * (1.f / 32.f), posY = (float)mWh * (1.f / 32.f) + 20;
    const float sizeX = (float)mWw * (30.f / 32.f), sizeY = (float)mWh * (30.f / 32.f) - 20;
    ImGui::SetNextWindowPos({ posX, posY });
    ImGui::SetNextWindowSize({ sizeX, sizeY });
    if(ImGui::BeginPopupModal("##settings", &show, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
      if (ImGui::BeginTabBar("##categories")) {
        if (ImGui::BeginTabItem(GET_TRANSLATED_STRING(Language::SETTINGS_CATEGORY_CPU))) {
          ImGui::Checkbox(GET_TRANSLATED_STRING(Language::SETTINGS_OPTION_ENABLE_JIT), &jit);
          ImGui::EndTabItem();
        }
        
        if (ImGui::BeginTabItem(GET_TRANSLATED_STRING(Language::SETTINGS_CATEGORY_AUDIO))) {
          ImGui::Checkbox(GET_TRANSLATED_STRING(Language::SETTINGS_OPTION_LOCK_CHANNELS), &lockChannels);
          ImGui::Checkbox(GET_TRANSLATED_STRING(Language::SETTINGS_OPTION_MUTE), &mute);
          if (mute) {
            volumeL = 0;
            volumeR = 0;

            ImGui::BeginDisabled();
            ImGui::SliderFloat(GET_TRANSLATED_STRING(Language::SETTINGS_OPTION_VOLUME_L), &oldVolumeL, 0, 1, "%.2f", ImGuiSliderFlags_NoInput);
            if (lockChannels) {
              oldVolumeR = oldVolumeL;
            }
            ImGui::SliderFloat(GET_TRANSLATED_STRING(Language::SETTINGS_OPTION_VOLUME_R), &oldVolumeR, 0, 1, "%.2f", ImGuiSliderFlags_NoInput);
            ImGui::EndDisabled();
          }
          else {
            volumeL = oldVolumeL;
            volumeR = oldVolumeR;

            ImGui::SliderFloat(GET_TRANSLATED_STRING(Language::SETTINGS_OPTION_VOLUME_L), &volumeL, 0, 1, "%.2f", ImGuiSliderFlags_NoInput);
            if (!lockChannels) {
              ImGui::SliderFloat(GET_TRANSLATED_STRING(Language::SETTINGS_OPTION_VOLUME_R), &volumeR, 0, 1, "%.2f", ImGuiSliderFlags_NoInput);
            }
            else {
              volumeR = volumeL;
              ImGui::BeginDisabled();
              ImGui::SliderFloat(GET_TRANSLATED_STRING(Language::SETTINGS_OPTION_VOLUME_R), &volumeR, 0, 1, "%.2f", ImGuiSliderFlags_NoInput);
              ImGui::EndDisabled();
            }

            oldVolumeL = volumeL;
            oldVolumeR = volumeR;
          }

          ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem(GET_TRANSLATED_STRING(Language::SETTINGS_CATEGORY_INTERFACE))) {
          static auto currentLang = selectedLanguage;
          const char* languages[Language::AVAILABLE_LANGS_COUNT] = {
            Language::languages[Language::ENGLISH],
            Language::languages[Language::ITALIAN]
          };
          ImGui::Text("%s:", GET_TRANSLATED_STRING(Language::SETTINGS_OPTION_LANGUAGE));
          CreateComboList("##language", (int*)&selectedLanguage, languages, (int)Language::AVAILABLE_LANGS_COUNT);
          ImGui::Separator();

          if (currentLang != selectedLanguage) {
            currentLang = selectedLanguage;
            Language::SetLanguage(languageStrings, selectedLanguage);
          }

          ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
      }

      const auto style = ImGui::GetStyle();
      ImGui::SetCursorPos({
        ImGui::GetWindowWidth() - ImGui::CalcTextSize(GET_TRANSLATED_STRING(Language::SETTINGS_CLOSE)).x - style.FramePadding.x * 5,
        ImGui::GetWindowHeight() - ImGui::CalcTextSize(GET_TRANSLATED_STRING(Language::SETTINGS_CLOSE)).y - style.FramePadding.y * 5,
      });
      if (ImGui::Button(GET_TRANSLATED_STRING(Language::SETTINGS_CLOSE))) show = false;

      ImGui::EndPopup();
    }
  }
}