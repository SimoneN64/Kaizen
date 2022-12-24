#include <GameList.hpp>
#include <filesystem>
#include <imgui.h>
#include <fstream>
#include <nlohmann/json.hpp>
#include <fmt/format.h>
#include <RomHelpers.hpp>
#include <File.hpp>
#include <thread>

using namespace nlohmann;
namespace fs = std::filesystem;

GameList::GameList(const std::string& path) {
  if(!path.empty()) {
    std::thread searchThread([path, this]() {
      std::ifstream gameDbFile("resources/db.json");
      json gameDb = json::parse(gameDbFile);
      std::vector<u8> rom{};
      for(const auto& p : fs::recursive_directory_iterator{path}) {
        const auto filename = p.path().string();
        if(p.path().extension() == ".n64" || p.path().extension() == ".z64" || p.path().extension() == ".v64" ||
           p.path().extension() == ".N64" || p.path().extension() == ".Z64" || p.path().extension() == ".V64") {
          std::ifstream file(filename, std::ios::binary);
          file.unsetf(std::ios::skipws);

          if(!file.is_open()) {
            util::panic("Unable to open {}!", filename);
          }

          file.seekg(0, std::ios::end);
          auto size = file.tellg();
          auto sizeAdjusted = util::NextPow2(size);
          file.seekg(0, std::ios::beg);

          std::fill(rom.begin(), rom.end(), 0);
          rom.resize(sizeAdjusted);
          rom.insert(rom.begin(), std::istream_iterator<u8>(file), std::istream_iterator<u8>());
          file.close();

          u32 crc{};
          util::GetRomCRC(sizeAdjusted, rom.data(), crc);

          bool found = false;

          for(const auto& item : gameDb["items"]) {
            const auto& crcEntry = item["crc"];
            if(!crcEntry.empty()) {
              if(crcEntry.get<std::string>() == fmt::format("{:08X}", crc)) {
                found = true;
                gamesList.push_back(GameInfo{
                  item["name"].get<std::string>(),
                  item["region"].get<std::string>(),
                  fmt::format("{:.2f} MiB", float(size) / 1024 / 1024),
                  "Good",
                  p.path().string()
                });
              }
            }
          }

          if(!found) {
            gamesList.push_back(GameInfo{
              p.path().stem().string(),
              "Unknown",
              fmt::format("{:.2f} MiB", float(size) / 1024 / 1024),
              "Unknown",
              p.path().string()
            });
          }
        }
      };

      gameDbFile.close();
    });

    searchThread.detach();
  }
}

bool GameList::RenderWidget(float mainMenuBarHeight, std::string& rom) {
  const auto windowSize = ImGui::GetIO().DisplaySize;
  ImGui::SetNextWindowPos(ImVec2(0, mainMenuBarHeight));
  ImGui::SetNextWindowSize(ImVec2(windowSize.x, windowSize.y - mainMenuBarHeight));
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));
  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);

  ImGui::Begin(
    "Games list",
    nullptr,
    ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
    ImGuiWindowFlags_NoBringToFrontOnFocus
  );

  static ImGuiTableFlags flags =
    ImGuiTableFlags_Resizable
    | ImGuiTableFlags_RowBg
    | ImGuiTableFlags_BordersOuterV
    | ImGuiTableFlags_SizingStretchProp;

  bool toOpen = false;
  if (ImGui::BeginTable("Games List", 4, flags)) {
    ImGui::TableSetupColumn("Title");
    ImGui::TableSetupColumn("Region");
    ImGui::TableSetupColumn("Status");
    ImGui::TableSetupColumn("Size");
    ImGui::TableHeadersRow();

    int i = 0;

    for (const auto& entry : gamesList) {
      ImGui::TableNextRow(ImGuiTableRowFlags_None);
      ImGui::PushID(i);
      ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, ImVec2(0.0f, 0.5f));
      ImGui::TableSetColumnIndex(0);

      if (ImGui::Selectable(entry.name.c_str(), false, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap, ImVec2(0.0f, 20.f))) {
        toOpen = true;
        rom = entry.path;
      }

      ImGui::TableSetColumnIndex(1);

      if (ImGui::Selectable(entry.region.c_str(), false, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap, ImVec2(0.0f, 20.f))) {
        toOpen = true;
        rom = entry.path;
      }

      ImGui::TableSetColumnIndex(2);

      if (ImGui::Selectable(entry.status.c_str(), false, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap, ImVec2(0.0f, 20.f))) {
        toOpen = true;
        rom = entry.path;
      }

      ImGui::TableSetColumnIndex(3);

      if (ImGui::Selectable(entry.size.c_str(), false, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap, ImVec2(0.0f, 20.f))) {
        toOpen = true;
        rom = entry.path;
      }

      ImGui::PopStyleVar();
      ImGui::PopID();
      i++;
    }

    ImGui::EndTable();
  }

  ImGui::End();
  ImGui::PopStyleVar();
  ImGui::PopStyleVar();

  return toOpen;
}