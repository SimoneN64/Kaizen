#pragma once
#include <vector>
#include <string>

struct GameInfo {
  std::string name, region, size, path;
};

struct GameList {
  GameList(const std::string&);
  ~GameList() = default;

  bool RenderWidget(bool, float, std::string&);

  std::vector<GameInfo> GetGamesList() const { return gamesList; }
private:
  std::vector<GameInfo> gamesList{};
};
