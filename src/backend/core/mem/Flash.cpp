#include <Mem.hpp>

namespace n64 {
constexpr auto FLASH_SIZE = 1_mb;

Flash::Flash(mio::mmap_sink &saveData) : saveData(saveData) {}

void Flash::Reset() {
  state = FlashState::Idle;
}

void Flash::Load(SaveType saveType, const std::string& path) {
  if(saveType == SAVE_FLASH_1m) {
    flashPath = fs::path(path).replace_extension(".flash").string();
    std::error_code error;
    if (saveData.is_mapped()) {
      saveData.sync(error);
      if (error) { Util::panic("Could not sync {}", flashPath); }
      saveData.unmap();
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

    saveData = mio::make_mmap_sink(
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
      if(saveData.is_mapped()) {
        for (int i = 0; i < 128; i++) {
          saveData[eraseOffs + i] = 0xFFi8;
        }
      } else {
        Util::panic("Accessing flash when not mapped!");
      }
      break;
    case FlashState::Write:
      if(saveData.is_mapped()) {
        for (int i = 0; i < 128; i++) {
          saveData[writeOffs + i] = writeBuf[i];
        }
      } else {
        Util::panic("Accessing flash when not mapped!");
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

std::vector<u8> Flash::Serialize() {
  std::vector<u8> res{};

  res.resize(
  sizeof(state) +
  sizeof(status) +
  sizeof(eraseOffs) +
  sizeof(writeOffs) +
  128);

  u32 index = 0;
  memcpy(res.data() + index, &state, sizeof(state));
  index += sizeof(state);
  memcpy(res.data() + index, &status, sizeof(status));
  index += sizeof(status);
  memcpy(res.data() + index, &eraseOffs, sizeof(eraseOffs));
  index += sizeof(eraseOffs);
  memcpy(res.data() + index, &writeOffs, sizeof(writeOffs));
  index += sizeof(writeOffs);
  memcpy(res.data() + index, writeBuf, 128);

  return res;
}

void Flash::Deserialize(const std::vector<u8>& data) {
  u32 index = 0;
  memcpy(&state, data.data() + index, sizeof(state));
  index += sizeof(state);
  memcpy(&status, data.data() + index, sizeof(status));
  index += sizeof(status);
  memcpy(&eraseOffs, data.data() + index, sizeof(eraseOffs));
  index += sizeof(eraseOffs);
  memcpy(&writeOffs, data.data() + index, sizeof(writeOffs));
  index += sizeof(writeOffs);
  memcpy(writeBuf, data.data() + index, 128);
}

template <> void Flash::Write<u32>(u32 index, u32 val) {
  if(index > 0) {
    u8 cmd = val >> 24;
    switch(cmd) {
      case FLASH_COMMAND_EXECUTE: CommandExecute(); break;
      case FLASH_COMMAND_STATUS: CommandStatus(); break;
      case FLASH_COMMAND_SET_ERASE_OFFSET: CommandSetEraseOffs(val); break;
      case FLASH_COMMAND_ERASE: CommandErase(); break;
      case FLASH_COMMAND_SET_WRITE_OFFSET: CommandSetWriteOffs(val); break;
      case FLASH_COMMAND_WRITE: CommandWrite(); break;
      case FLASH_COMMAND_READ: CommandRead(); break;
      default: Util::warn("Invalid flash command: {:02X}", cmd);
    }
  } else {
    Util::warn("Flash Write of {:08X} @ {:08X}", val, index);
  }
}

template <> void Flash::Write<u8>(u32 index, u8 val) {
  switch(state) {
    case FlashState::Idle: Util::panic("Invalid FlashState::Idle with Write<u8>");
    case FlashState::Status: Util::panic("Invalid FlashState::Status with Write<u8>");
    case FlashState::Erase: Util::panic("Invalid FlashState::Erase with Write<u8>");
    case FlashState::Write:
      writeBuf[index] = val;
      break;
    case FlashState::Read: Util::panic("Invalid FlashState::Read with Write<u8>");
    default: Util::warn("Invalid flash state on Write<u8>: {:02X}", static_cast<u8>(state));
  }
}

template <> u8 Flash::Read<u8>(u32 index) const {
  switch (state) {
    case FlashState::Idle: Util::panic("Flash read byte while in state FLASH_STATE_IDLE");
    case FlashState::Write: Util::panic("Flash read byte while in state FLASH_STATE_WRITE");
    case FlashState::Read: {
      if(saveData.is_mapped()) {
        u8 value = saveData[index];
        Util::debug("Flash read byte in state read: index {:08X} = {:02X}", index, value);
        return value;
      } else {
        Util::panic("Accessing flash when not mapped!");
      }
    }
    case FlashState::Status: {
      u32 offset = (7 - (index % 8)) * 8;
      u8 value = (status >> offset) & 0xFF;
      Util::debug("Flash read byte in state status: index {:08X} = {:02X}", index, value);
      return value;
    }
    default: Util::panic("Flash read byte while in unknown state");
  }
}

template <> u32 Flash::Read<u32>(u32) const {
  return status >> 32;
}
}