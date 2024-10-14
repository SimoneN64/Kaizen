#include <GameDB.hpp>
#include <Mem.hpp>

namespace n64 {
void GameDB::match(Mem &mem) {
  const ROM &rom = mem.rom;
  for (const auto &[code, regions, saveType, name] : gamedb) {
    const bool matches_code = code == rom.code;
    bool matches_region = false;

    for (int j = 0; j < regions.size() && !matches_region; j++) {
      if (regions[j] == rom.header.countryCode[0]) {
        matches_region = true;
      }
    }

    if (matches_code) {
      if (matches_region) {
        mem.saveType = saveType;
        mem.rom.gameNameDB = name;
        return;
      }

      Util::warn(
        "Matched code for {}, but not region! Game supposedly exists in regions [{}] but this image has region {}",
        name, regions, rom.header.countryCode[0]);
      mem.saveType = saveType;
      mem.rom.gameNameDB = name;
      return;
    }
  }

  Util::warn("Did not match any Game DB entries. Code: {} Region: {}", mem.rom.code, mem.rom.header.countryCode[0]);

  mem.rom.gameNameDB = "";
  mem.saveType = SAVE_NONE;
}
} // namespace n64
