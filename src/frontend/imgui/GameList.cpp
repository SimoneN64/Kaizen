#include <GameList.hpp>
#include <filesystem>
#include <imgui.h>
#include <fstream>
#include <nlohmann/json.hpp>
#include <fmt/format.h>
#include <Mem.hpp>
#include <execution>

using namespace nlohmann;
using namespace std::filesystem;

inline std::string CountryCodeToStr(u8 code) {
  switch(code) {
    case 0x37: return "Beta";
    case 0x41: return "Asian (NTSC)";
    case 0x42: return "Brazilian";
    case 0x43: return "Chinese";
    case 0x44: return "German";
    case 0x45: return "North America";
    case 0x46: return "French";
    case 0x47: return "Gateway 64 (NTSC)";
    case 0x48: return "Dutch";
    case 0x49: return "Italian";
    case 0x4A: return "Japanese";
    case 0x4B: return "Korean";
    case 0x4C: return "Gateway 64 (PAL)";
    case 0x4E: return "Canadian";
    case 0x50: return "European (basic spec.)";
    case 0x53: return "Spanish";
    case 0x55: return "Australian";
    case 0x57: return "Scandinavian";
    case 0x58: case 0x59: return "European";
    default: return "Unrecognized";
  }
}

GameList::GameList(const std::string& path) {
  if(!path.empty()) {
    std::ifstream gameDbFile("resources/db.json");
    json gameDb = json::parse(gameDbFile);
    std::vector<u8> rom{};

    std::for_each(std::execution::par, begin(directory_iterator{path}), end(directory_iterator{path}), [&](const auto& p) {
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
        util::SwapN64RomJustCRC(sizeAdjusted, rom.data(), crc);

        std::for_each(std::execution::par, gameDb["items"].begin(), gameDb["items"].end(), [&](const auto& item) {
          const auto& crcEntry = item["crc"];
          if(!crcEntry.empty()) {
            if(crcEntry.template get<std::string>() == fmt::format("{:08X}", crc)) {
              gamesList.push_back({
                item["name"].template get<std::string>(),
                item["region"].template get<std::string>(),
                fmt::format("{:.2f} MiB", float(size) / 1024 / 1024),
                "Good",
                p.path().string()
              });
            }
          }
        });
      }
    });

    gameDbFile.close();
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