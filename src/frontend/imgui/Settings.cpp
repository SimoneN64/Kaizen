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
      if(cpuType == "dynarec") {
        core.cpuType = n64::CpuType::Dynarec;
      } else if(cpuType == "interpreter") {
        core.cpuType = n64::CpuType::Interpreter;
      } else {
        Util::panic("Unrecognized cpu type: {}\n", cpuType);
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
    checkjsonentry(gamesDir, std::string, "general", "gamesDir", "");
  } else {
    settingsFile = std::fstream("resources/settings.json", std::fstream::trunc | std::fstream::in | std::fstream::out);
    settings["general"]["gamesDir"] = "";
    settings["cpu"]["type"] = "interpreter";
    settings["audio"]["volumeR"] = 0.5;
    settings["audio"]["volumeL"] = 0.5;
    settings["audio"]["lockChannels"] = true;

    core.cpuType = n64::CpuType::Interpreter;
    volumeR = 0.5;
    volumeL = 0.5;
    lockChannels = true;
    gamesDir = "";

    settingsFile << settings;
  }
  settingsFile.close();

  switch(core.cpuType) {
    case n64::CpuType::Interpreter:
      core.cpuInterp = new n64::Interpreter;
      break;
    case n64::CpuType::Dynarec:
      core.cpuDynarec = new n64::JIT::Dynarec;
      break;
    case n64::CpuType::NONE:
      Util::panic("BRUH\n");
  }
}

Settings::~Settings() {
  auto fileExists = fs::exists("resources/settings.json");
  std::fstream settingsFile;
  if(fileExists) {
    settingsFile = std::fstream("resources/settings.json", std::fstream::trunc | std::fstream::out);

    settings["general"]["gamesDir"] = gamesDir;
    settings["cpu"]["type"] = cpuType;
    settings["audio"]["volumeR"] = volumeR;
    settings["audio"]["volumeL"] = volumeL;
    settings["audio"]["lockChannels"] = lockChannels;
    settingsFile << settings;
  } else {
    settingsFile = std::fstream("resources/settings.json", std::fstream::out);

    settings["general"]["gamesDir"] = gamesDir;
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
      const char *categories[] = {"General", "CPU", "Audio"};
      enum Category { General, CPU, Audio };
      static int category = General;
      CreateComboList("##", &category, categories, 3);
      ImGui::Separator();
      switch (category) {
        case General:
          ImGui::Text("Games directory: %s", gamesDir.c_str());
          ImGui::SameLine();
          if(ImGui::Button("Select...")) {
            nfdchar_t *outpath;
            nfdresult_t result = NFD_PickFolder(&outpath, nullptr);
            if (result == NFD_OKAY) {
              gamesDir = outpath;
              NFD_FreePath(outpath);
            }
          }
          break;
        case CPU: {
          const char *cpuTypes[] = {"JIT", "Interpreter"};
          static int currentType = 0;
          if (cpuType == "interpreter") currentType = 1;

          if (CreateComboList("Core type", &currentType, cpuTypes, 2)) {
            if(currentType == 0) cpuType = "dynarec";
            if(currentType == 1) cpuType = "interpreter";
          }
        } break;
        case Audio:
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
      }
      ImGui::EndPopup();
    }
  }
}