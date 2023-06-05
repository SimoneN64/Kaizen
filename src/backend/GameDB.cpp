#include <GameDB.hpp>
#include <Mem.hpp>

namespace n64 {
void GameDB::match(Mem& mem) {
  ROM& rom = mem.rom;
  for (const auto & i : gamedb) {
    bool matches_code = i.code == rom.code;
    bool matches_region = false;

    for (int j = 0; j < i.regions.size() && !matches_region; j++) {
      if (i.regions[j] == rom.header.countryCode[0]) {
        matches_region = true;
      }
    }

    if (matches_code) {
      if (matches_region) {
        mem.saveType = i.saveType;
        mem.rom.gameNameDB = i.name;
        return;
      } else {
        Util::warn("Matched code for {}, but not region! Game supposedly exists in regions [{}] but this image has region {}",
                i.name, i.regions, rom.header.countryCode[0]);
      }
    }

  }

  Util::debug("Did not match any Game DB entries. Code: {} Region: {}", mem.rom.code, mem.rom.header.countryCode[0]);

  mem.rom.gameNameDB = "";
  mem.saveType = SAVE_NONE;
}
}