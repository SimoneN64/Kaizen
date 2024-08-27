#include <Mem.hpp>
#include <cassert>

namespace n64 {
constexpr auto FLASH_SIZE = 1_mb;

Flash::Flash(mio::mmap_sink &saveData) : saveData(saveData) {}

void Flash::Reset() {
  state = FlashState::Idle;
  writeOffs = {};
  state = {};
  status = {};
  eraseOffs = {};
  writeBuf = {};
}

void Flash::Load(SaveType saveType, const std::string &path) {
  if (saveType == SAVE_FLASH_1m) {
    fs::path flashPath_ = path;
    if (!savePath.empty()) {
      flashPath_ = savePath / flashPath_.filename();
    }
    flashPath = flashPath_.replace_extension(".flash").string();
    std::error_code error;
    if (saveData.is_mapped()) {
      saveData.sync(error);
      if (error) {
        Util::panic("Could not sync {}", flashPath);
      }
      saveData.unmap();
    }

    auto flashVec = Util::ReadFileBinary(flashPath);
    if (flashVec.empty()) {
      std::vector<u8> dummy{};
      dummy.resize(FLASH_SIZE);
      Util::WriteFileBinary(dummy, flashPath);
      flashVec = Util::ReadFileBinary(flashPath);
    }

    if (flashVec.size() != FLASH_SIZE) {
      Util::panic("Corrupt SRAM!");
    }

    saveData = mio::make_mmap_sink(flashPath, 0, mio::map_entire_file, error);
    if (error) {
      Util::panic("Could not make mmap {}", flashPath);
    }
  }
}

void Flash::CommandExecute() {
  Util::trace("Flash::CommandExecute");
  switch (state) {
  case FlashState::Idle:
    break;
  case FlashState::Erase:
    if (saveData.is_mapped()) {
      for (int i = 0; i < 128; i++) {
        saveData[eraseOffs + i] = 0xFF;
      }
    } else {
      Util::panic("Accessing flash when not mapped!");
    }
    break;
  case FlashState::Write:
    if (saveData.is_mapped()) {
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

void Flash::CommandSetEraseOffs(u32 val) { eraseOffs = (val & 0xffff) << 7; }

void Flash::CommandErase() {
  state = FlashState::Erase;
  status = 0x1111800800C20000LL;
}

void Flash::CommandSetWriteOffs(u32 val) {
  writeOffs = (val & 0xffff) << 7;
  status = 0x1111800400C20000LL;
}

void Flash::CommandWrite() { state = FlashState::Write; }

void Flash::CommandRead() {
  state = FlashState::Read;
  status = 0x11118004F0000000;
}

std::vector<u8> Flash::Serialize() {
  std::vector<u8> res{};

  res.resize(sizeof(state) + sizeof(status) + sizeof(eraseOffs) + sizeof(writeOffs) + 128);

  u32 index = 0;
  memcpy(res.data() + index, &state, sizeof(state));
  index += sizeof(state);
  memcpy(res.data() + index, &status, sizeof(status));
  index += sizeof(status);
  memcpy(res.data() + index, &eraseOffs, sizeof(eraseOffs));
  index += sizeof(eraseOffs);
  memcpy(res.data() + index, &writeOffs, sizeof(writeOffs));
  index += sizeof(writeOffs);
  std::copy(writeBuf.begin(), writeBuf.end(), res.begin() + index);

  return res;
}

void Flash::Deserialize(const std::vector<u8> &data) {
  u32 index = 0;
  memcpy(&state, data.data() + index, sizeof(state));
  index += sizeof(state);
  memcpy(&status, data.data() + index, sizeof(status));
  index += sizeof(status);
  memcpy(&eraseOffs, data.data() + index, sizeof(eraseOffs));
  index += sizeof(eraseOffs);
  memcpy(&writeOffs, data.data() + index, sizeof(writeOffs));
  index += sizeof(writeOffs);
  std::copy(data.begin() + index, data.begin() + index + 128, writeBuf.begin());
}

template <>
void Flash::Write<u32>(u32 index, u32 val) {
  if (index > 0) {
    u8 cmd = val >> 24;
    switch (cmd) {
    case FLASH_COMMAND_EXECUTE:
      CommandExecute();
      break;
    case FLASH_COMMAND_STATUS:
      CommandStatus();
      break;
    case FLASH_COMMAND_SET_ERASE_OFFSET:
      CommandSetEraseOffs(val);
      break;
    case FLASH_COMMAND_ERASE:
      CommandErase();
      break;
    case FLASH_COMMAND_SET_WRITE_OFFSET:
      CommandSetWriteOffs(val);
      break;
    case FLASH_COMMAND_WRITE:
      CommandWrite();
      break;
    case FLASH_COMMAND_READ:
      CommandRead();
      break;
    default:
      Util::warn("Invalid flash command: {:02X}", cmd);
    }
  } else {
    Util::warn("Flash Write of {:08X} @ {:08X}", val, index);
  }
}

template <>
void Flash::Write<u8>(u32 index, u8 val) {
  switch (state) {
  case FlashState::Idle:
    Util::panic("Invalid FlashState::Idle with Write<u8>");
  case FlashState::Status:
    Util::panic("Invalid FlashState::Status with Write<u8>");
  case FlashState::Erase:
    Util::panic("Invalid FlashState::Erase with Write<u8>");
  case FlashState::Read:
    Util::panic("Invalid FlashState::Read with Write<u8>");
  case FlashState::Write:
    assert(index <= 0x7F && "Out of range flash Write8");
    writeBuf[index] = val;
    break;
  default:
    Util::warn("Invalid flash state on Write<u8>: {:02X}", static_cast<u8>(state));
  }
}

template <>
u8 Flash::Read<u8>(u32 index) const {
  switch (state) {
  case FlashState::Idle:
    Util::panic("Flash read byte while in state FLASH_STATE_IDLE");
  case FlashState::Write:
    Util::panic("Flash read byte while in state FLASH_STATE_WRITE");
  case FlashState::Read:
    {
      if (saveData.is_mapped()) {
        u8 value = saveData[index];
        Util::trace("Flash read byte in state read: index {:08X} = {:02X}", index, value);
        return value;
      } else {
        Util::panic("Accessing flash when not mapped!");
      }
    }
  case FlashState::Status:
    {
      u32 offset = (7 - (index % 8)) * 8;
      u8 value = (status >> offset) & 0xFF;
      Util::trace("Flash read byte in state status: index {:08X} = {:02X}", index, value);
      return value;
    }
  default:
    Util::panic("Flash read byte while in unknown state");
  }
}

template <>
u32 Flash::Read<u32>(u32) const {
  return status >> 32;
}
} // namespace n64
