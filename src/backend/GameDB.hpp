#pragma once
#include <string>

namespace n64 {
enum SaveType {
  SAVE_NONE,
  SAVE_EEPROM_4k,
  SAVE_EEPROM_16k,
  SAVE_FLASH_1m,
  SAVE_SRAM_256k
};

struct Mem;
struct GameDBEntry {
  std::string code;
  std::string regions;
  SaveType saveType;
  const char *name;
};

namespace GameDB {
void match(Mem &mem);
}
}