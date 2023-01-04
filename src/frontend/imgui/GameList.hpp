#pragma once
#include <vector>
#include <string>
#include <atomic>

enum GameInfoID {
  Name_ID,
  Region_ID,
  Size_ID,
  Status_ID
};

struct GameInfo {
  std::string name, region, size, status, path;
};

struct GameList {
  GameList(const std::string&);
  ~GameList() = default;

  bool RenderWidget(float, std::string&);

  [[nodiscard]] std::vector<GameInfo> GetGamesList() const { return gamesList; }
private:
  std::vector<GameInfo> gamesList{}, notMatch{};
  std::atomic_bool threadDone = false;
};
