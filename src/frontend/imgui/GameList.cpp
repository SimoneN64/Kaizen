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
    std::for_each(std::execution::par_unseq, begin(recursive_directory_iterator{path}), end(recursive_directory_iterator{path}), [&](const auto& p) {
      if(p.path().extension() == ".n64" || p.path().extension() == ".z64" || p.path().extension() == ".v64" ||
         p.path().extension() == ".N64" || p.path().extension() == ".Z64" || p.path().extension() == ".V64") {
        std::ifstream file(p.path().string(), std::ios::binary);
        file.unsetf(std::ios::skipws);

        if(!file.is_open()) {
          util::panic("Unable to open {}!", path);
        }

        file.seekg(0, std::ios::end);
        auto size = file.tellg();
        auto sizeAdjusted = util::NextPow2(size);
        file.seekg(0, std::ios::beg);

        std::vector<u8> cart{};

        std::fill(cart.begin(), cart.end(), 0);
        cart.resize(sizeAdjusted);
        cart.insert(cart.begin(), std::istream_iterator<u8>(file), std::istream_iterator<u8>());

        file.close();

        u32 crc, dummy;
        util::SwapN64Rom(sizeAdjusted, cart.data(), crc, dummy);

        u8 countryCode = 0;
        countryCode = cart[0x3D];

        std::ifstream gameDbFile("resources/game_db.json");
        json gameDb = json::parse(gameDbFile);
        auto entry = gameDb[fmt::format("{:08x}", crc)]["name"];

        if(!entry.empty()) {
          gamesList.push_back({entry.get<std::string>(), CountryCodeToStr(countryCode), fmt::format("{:.2f} MiB", float(size) / 1024 / 1024), p.path().string()});
        } else {
          gamesList.push_back({p.path().stem().string(), CountryCodeToStr(countryCode), fmt::format("{:.2f} MiB", float(size) / 1024 / 1024), p.path().string()});
        }
      }
    });
  }
}

bool GameList::RenderWidget(bool showMainMenuBar, float mainMenuBarHeight, std::string& rom) {
  const auto windowSize = ImGui::GetIO().DisplaySize;
  if (showMainMenuBar) {
    ImGui::SetNextWindowPos(ImVec2(0, mainMenuBarHeight));
    ImGui::SetNextWindowSize(ImVec2(windowSize.x, windowSize.y - mainMenuBarHeight));
  } else {
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(windowSize);
  }
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
  if (ImGui::BeginTable("Games List", 3, flags)) {
    ImGui::TableSetupColumn("Title");
    ImGui::TableSetupColumn("Region");
    ImGui::TableSetupColumn("Size");
    ImGui::TableHeadersRow();

    for (int row = 0; row < gamesList.size(); row++) {
      GameInfo entry = gamesList[row];

      ImGui::TableNextRow(ImGuiTableRowFlags_None);
      ImGui::PushID(row);
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

      if (ImGui::Selectable(entry.size.c_str(), false, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap, ImVec2(0.0f, 20.f))) {
        toOpen = true;
        rom = entry.path;
      }

      ImGui::PopStyleVar();
      ImGui::PopID();
    }

    ImGui::EndTable();
  }

  ImGui::End();
  ImGui::PopStyleVar();
  ImGui::PopStyleVar();

  return toOpen;
}