#include <Settings.hpp>
#include <fstream>
#include <filesystem>
#include <nfd.h>
#include <Widgets.hpp>

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
    auto entryCpuType = settings["cpu"]["type"];
    if(!entryCpuType.empty()) {
      cpuType = entryCpuType.get<std::string>();
      if(cpuType == "jit") {
        core.cpuType = n64::CpuType::JIT;
      } else if(cpuType == "interpreter") {
        core.cpuType = n64::CpuType::Interpreter;
      } else {
        Util::panic("Unrecognized cpu type: {}", cpuType);
      }
    } else {
      settingsFile.clear();
      settings["cpu"]["type"] = "interpreter";
      settingsFile << settings;
      core.cpuType = n64::CpuType::Interpreter;
    }

    checkjsonentry(volumeR, float, "audio", "volumeR", 0.5);
    checkjsonentry(volumeL, float, "audio", "volumeL", 0.5);
    checkjsonentry(lockChannels, bool, "audio", "lockChannels", true);
  } else {
    settingsFile = std::fstream("resources/settings.json", std::fstream::trunc | std::fstream::in | std::fstream::out);
    settings["cpu"]["type"] = "interpreter";
    settings["audio"]["volumeR"] = 0.5;
    settings["audio"]["volumeL"] = 0.5;
    settings["audio"]["lockChannels"] = true;

    core.cpuType = n64::CpuType::Interpreter;
    volumeR = 0.5;
    volumeL = 0.5;
    lockChannels = true;

    settingsFile << settings;
  }
  settingsFile.close();

  switch(core.cpuType) {
    case n64::CpuType::Interpreter:
      core.cpu = std::make_unique<n64::Interpreter>();
      break;
    case n64::CpuType::JIT:
      core.cpu = std::make_unique<n64::JIT>();
      break;
    case n64::CpuType::COUNT:
      Util::panic("BRUH!");
  }
}

Settings::~Settings() {
  auto fileExists = fs::exists("resources/settings.json");
  std::fstream settingsFile;
  if(fileExists) {
    settingsFile = std::fstream("resources/settings.json", std::fstream::trunc | std::fstream::out);

    settings["cpu"]["type"] = cpuType;
    settings["audio"]["volumeR"] = volumeR;
    settings["audio"]["volumeL"] = volumeL;
    settings["audio"]["lockChannels"] = lockChannels;
    settingsFile << settings;
  } else {
    settingsFile = std::fstream("resources/settings.json", std::fstream::out);

    settings["cpu"]["type"] = cpuType;
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
      enum class SelectedSetting { CPU, Audio, COUNT };
      static SelectedSetting selectedSetting = SelectedSetting::Audio;
      const char *categories[(int)SelectedSetting::COUNT] = { "CPU", "Audio" };
      CreateComboList("##", (int*)&selectedSetting, categories, (int)SelectedSetting::COUNT);
      ImGui::Separator();
      switch (selectedSetting) {
        case SelectedSetting::CPU: {
            const char* cpuTypes[(int)n64::CpuType::COUNT] = { "Interpreter", "JIT" };
            static n64::CpuType currentType = n64::CpuType::Interpreter;
            if (cpuType == "jit") currentType = n64::CpuType::JIT;

            if (CreateComboList("Core type", (int*)&currentType, cpuTypes, (int)n64::CpuType::COUNT)) {
              switch (currentType) {
                case n64::CpuType::Interpreter:
                  cpuType = "interpreter";
                  break;
                case n64::CpuType::JIT:
                  cpuType = "jit";
                  break;
                case n64::CpuType::COUNT:
                  Util::panic("BRUH!");
              }
            }
          } break;
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
          Util::panic("BRUH!");
      }

      ImGui::EndPopup();
    }
  }
}