#include <Settings.hpp>
#include <fstream>
#include <filesystem>
#include <imgui.h>

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
      settings["cpu"]["type"] = "dynarec";
      settingsFile << settings;
      core.cpuType = n64::CpuType::Dynarec;
    }

    auto volumeREntry = settings["audio"]["volumeR"];
    if(!volumeREntry.empty()) {
      auto value = volumeREntry.get<float>();
      volumeR = value;
    } else {
      settingsFile.clear();
      settings["audio"]["volumeR"] = 0.5;
      settingsFile << settings;
      volumeR = 0.5;
    }
    auto volumeLEntry = settings["audio"]["volumeL"];
    if(!volumeLEntry.empty()) {
      auto value = volumeLEntry.get<float>();
      volumeL = value;
    } else {
      settingsFile.clear();
      settings["audio"]["volumeL"] = 0.5;
      settingsFile << settings;
      volumeL = 0.5;
    }
    auto lockChannelsEntry = settings["audio"]["lockChannels"];
    if(!lockChannelsEntry.empty()) {
      auto value = lockChannelsEntry.get<bool>();
      lockChannels = value;
    } else {
      settingsFile.clear();
      settings["audio"]["lockChannels"] = true;
      settingsFile << settings;
      lockChannels = true;
    }
  } else {
    settings["cpu"]["type"] = "dynarec";
    settings["audio"]["volumeR"] = 0.5;
    settings["audio"]["volumeL"] = 0.5;
    settings["audio"]["lockChannels"] = true;

    core.cpuType = n64::CpuType::Dynarec;
    volumeR = 0.5;
    volumeL = 0.5;
    lockChannels = true;

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
      static enum { CPU, Audio } category = CPU;
      if(ImGui::Button("CPU")) {
        category = CPU;
      }
      ImGui::SameLine();
      if(ImGui::Button("Audio")) {
        category = Audio;
      }
      ImGui::Separator();
      if(category == Audio) {
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
      } else if(category == CPU) {
        const char* items[] = { "JIT", "Interpreter" };
        static int currentIndex = [this]() {
          if(cpuType == "dynarec") return 0;
          else if(cpuType == "interpreter") return 1;
          return 0;
        }();

        if(ImGui::BeginCombo("CPU type", items[currentIndex])) {
          for (int n = 0; n < 2; n++) {
            const bool is_selected = (currentIndex == n);
            if (ImGui::Selectable(items[n], is_selected)) {
              currentIndex = n;
            }

            if (is_selected) {
              ImGui::SetItemDefaultFocus();
            }
          }

          if(currentIndex == 0) {
            cpuType = "dynarec";
          }
          if(currentIndex == 1) {
            cpuType = "interpreter";
          }
          ImGui::EndCombo();
        }
      }
      ImGui::EndPopup();
    }

    SetLockChannels(lockChannels);
    SetVolumeL(volumeL);
    SetVolumeR(volumeR);
  }
}