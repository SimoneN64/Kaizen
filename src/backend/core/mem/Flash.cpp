#include <Mem.hpp>

namespace n64 {
constexpr auto FLASH_SIZE = 1_mb;

void Flash::Reset() {
  state = Idle;
  if (flash.is_mapped()) {
    std::error_code error;
    flash.sync(error);
    if (error) { Util::panic("Could not sync {}", flashPath); }
    flash.unmap();
  }
}

void Flash::Load(SaveType saveType, const std::string& path) {
  if(saveType == SAVE_FLASH_1m) {
    flashPath = fs::path(path).replace_extension(".flash").string();
    std::error_code error;
    if (flash.is_mapped()) {
      flash.sync(error);
      if (error) { Util::panic("Could not sync {}", flashPath); }
      flash.unmap();
    }

    FILE *f = fopen(flashPath.c_str(), "rb");
    if (!f) {
      f = fopen(flashPath.c_str(), "wb");
      u8* dummy = (u8*)calloc(FLASH_SIZE, 1);
      fwrite(dummy, 1, FLASH_SIZE, f);
    }

    fseek(f, 0, SEEK_END);
    size_t actualSize = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (actualSize != FLASH_SIZE) {
      Util::panic("Corrupt flash!");
    }
    fclose(f);

    flash = mio::make_mmap_sink(
      flashPath, 0, mio::map_entire_file, error);
    if (error) { Util::panic("Could not open {}", path); }
  }
}

void Flash::CommandExecute() {
  Util::debug("Flash::CommandExecute");
  switch (state) {
    case FlashState::Idle:
      break;
    case FlashState::Erase:
      for (int i = 0; i < 128; i++) {
        flash[eraseOffs + i] = 0xFF;
      }
      break;
    case FlashState::Write:
      for (int i = 0; i < 128; i++) {
        flash[writeOffs + i] = writeBuf[i];
      }
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
  status = 0x1111800800C20000LL;
}

void Flash::CommandSetWriteOffs(u32 val) {
  writeOffs = (val & 0xffff) << 7;
  status = 0x1111800400C20000LL;
}

void Flash::CommandWrite() {
  state = FlashState::Write;
}

void Flash::CommandRead() {
  state = FlashState::Read;
  status = 0x11118004F0000000;
}
}