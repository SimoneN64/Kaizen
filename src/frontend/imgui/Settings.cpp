#include <Settings.hpp>
#include <fstream>
#include <filesystem>
#include <utilities.hpp>
#include <nfd.h>

using namespace std::filesystem;

Settings::Settings(n64::Core& core) {
  auto modes = std::fstream::in | std::fstream::out;
  auto fileExists = exists("resources/settings.json");
  if(!fileExists) {
    modes |= std::fstream::trunc;
  }
  std::fstream settingsFile{"resources/settings.json", modes};
  if(fileExists) {
    settings = json::parse(settingsFile);
    auto entryCpuType = settings["cpu"]["type"];
    if(!entryCpuType.empty()) {
      cpuType = entryCpuType.get<std::string>();
      if(cpuType == "dynarec") {
        core.cpuType = n64::CpuType::Dynarec;
      } else if(cpuType == "interpreter") {
        core.cpuType = n64::CpuType::Interpreter;
      } else {
        util::panic("Unrecognized cpu type: {}\n", cpuType);
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
}

Settings::~Settings() {
  auto modes = std::fstream::out;
  auto fileExists = exists("resources/settings.json");
  if(fileExists) {
    modes |= std::fstream::trunc;

    std::fstream settingsFile{"resources/settings.json", modes};

    settings["general"]["gamesDir"] = gamesDir;
    settings["cpu"]["type"] = cpuType;
    settings["audio"]["volumeR"] = volumeR;
    settings["audio"]["volumeL"] = volumeL;
    settings["audio"]["lockChannels"] = lockChannels;
    settingsFile << settings;
    settingsFile.close();
  }
}

void Settings::RenderWidget(bool& show) {
  if(show) {
    ImGui::OpenPopup("Settings");
    if(ImGui::BeginPopupModal("Settings", &show)) {
      const char *categories[] = {"General", "CPU", "Audio"};
      enum Category { General, CPU, Audio };
      static int category = General;
      CreateComboList("", &category, categories, 3);
      ImGui::Separator();
      switch (category) {
        case General:
          ImGui::Text("Games directory: %s", gamesDir.c_str());
          ImGui::SameLine();
          if(ImGui::Button("Select...")) {
            nfdchar_t *outpath;
            nfdresult_t result = NFD_PickFolderN(&outpath, nullptr);
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