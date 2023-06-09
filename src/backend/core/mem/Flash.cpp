#include <Mem.hpp>

namespace n64 {
void Flash::Load(SaveType saveType, fs::path path) {
  if(saveType == SAVE_FLASH_1m) {
    if(saveData) {
      memset(saveData, 0xff, 1_mb);
    } else {
      saveData = (u8 *) malloc(1_mb);
      memset(saveData, 0xff, 1_mb);
    }
    saveDataPath = path.replace_extension(".flash").string();
    FILE *f = fopen(saveDataPath.c_str(), "rb");
    if (!f) {
      f = fopen(saveDataPath.c_str(), "wb");
      fwrite(saveData, 1, 1_mb, f);
      fclose(f);
      f = fopen(saveDataPath.c_str(), "rb");
    }

    fseek(f, 0, SEEK_END);
    size_t actualSize = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (actualSize != 1_mb) {
      Util::panic("Corrupt flash!");
    }

    fread(saveData, 1, 1_mb, f);
    fclose(f);
  }
}

void Flash::CommandExecute() {
  Util::debug("Flash::CommandExecute");
  switch (state) {
    case FlashState::Idle:
      break;
    case FlashState::Erase:
      for (int i = 0; i < 128; i++) {
        saveData[eraseOffs + i] = 0xFF;
      }
      saveDataDirty = true;
      break;
    case FlashState::Write:
      for (int i = 0; i < 128; i++) {
        saveData[writeOffs + i] = writeBuf[i];
      }
      saveDataDirty = true;
      break;
    case FlashState::Read:
      Util::panic("Execute command when flash in read state");
      break;
    case FlashState::Status:
      break;
  }
}

void Flash::CommandStatus() {
  state = FlashState::Status;
  status = 0x1111800100C20000;
}

void Flash::CommandSetEraseOffs(u32 val) {
  eraseOffs = (val & 0xffff) << 7;
}

void Flash::CommandErase() {
  state = FlashState::Erase;
  status = 0x11118004F0000000;
}

void Flash::CommandSetWriteOffs(u32 val) {
  writeOffs = (val & 0xffff) << 7;
}

void Flash::CommandWrite() {
  state = FlashState::Write;
  status = 0x1111800400C20000LL;
}

void Flash::CommandRead() {
  state = FlashState::Read;
  status = 0x11118004F0000000;
}
}