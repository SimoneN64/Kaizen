#pragma once
#include <vector>
#include <string>
#include <atomic>

struct GameInfo {
  std::string name, region, size, status, path;
};

struct GameList {
  GameList(const std::string&);
  ~GameList() = default;

  void Create(const std::string&);
  bool RenderWidget(float, std::string&);

  [[nodiscard]] std::vector<GameInfo> GetGamesList() const { return gamesList; }
  std::atomic_bool threadDone = false;
private:
  std::vector<GameInfo> gamesList{}, notMatch{};
};
