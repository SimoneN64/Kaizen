#include <core/mmio/PIF.hpp>
#include <core/Mem.hpp>
#include <core/registers/Registers.hpp>
#include <log.hpp>
#include <SDL_keyboard.h>
#include <cic_nus_6105/n64_cic_nus_6105.hpp>
#include <cassert>
#include <MupenMovie.hpp>

#define MEMPAK_SIZE 32768

namespace n64 {
void PIF::Reset() {
  memset(joybusDevices, 0, sizeof(JoybusDevice) * 6);
  memset(bootrom, 0, PIF_BOOTROM_SIZE);
  memset(ram, 0, PIF_RAM_SIZE);

  if (mempak)
    free(mempak);

  mempak = (u8*)calloc(MEMPAK_SIZE, 1);

  if (eeprom)
    free(eeprom);
}

void PIF::LoadMempak(fs::path path) {
  if (mempak)
    free(mempak);
  mempak = (u8*)calloc(MEMPAK_SIZE, 1);
  mempakPath = path.replace_extension(".mempak").string();
  FILE* f = fopen(mempakPath.c_str(), "rb");
  if (!f) {
    f = fopen(mempakPath.c_str(), "wb");
    fwrite(mempak, 1, MEMPAK_SIZE, f);
    fclose(f);
    f = fopen(mempakPath.c_str(), "rb");
  }

  fseek(f, 0, SEEK_END);
  size_t actualSize = ftell(f);
  fseek(f, 0, SEEK_SET);
  if (actualSize != MEMPAK_SIZE) {
    Util::panic("Corrupt mempak!");
  }

  fread(mempak, 1, MEMPAK_SIZE, f);
  fclose(f);
}

inline size_t getSaveSize(SaveType saveType) {
  switch (saveType) {
    case SAVE_NONE:
      return 0;
    case SAVE_EEPROM_4k:
      return 512;
    case SAVE_EEPROM_16k:
      return 2048;
    case SAVE_SRAM_256k:
      return 32768;
    case SAVE_FLASH_1m:
      return 131072;
    default:
      Util::panic("Unknown save type!");
  }
}

void PIF::LoadEeprom(SaveType saveType, fs::path path) {
  if (eeprom)
    free(eeprom);

  eepromSize = getSaveSize(saveType);
  eeprom = (u8*)calloc(eepromSize, 1);
  eepromPath = path.replace_extension(".eeprom").string();
  FILE* f = fopen(eepromPath.c_str(), "rb");
  if (!f) {
    f = fopen(eepromPath.c_str(), "wb");
    fwrite(eeprom, 1, eepromSize, f);
    fclose(f);
    f = fopen(eepromPath.c_str(), "rb");
  }

  fseek(f, 0, SEEK_END);
  size_t actualSize = ftell(f);
  fseek(f, 0, SEEK_SET);
  if (actualSize != eepromSize) {
    Util::panic("Corrupt eeprom!");
  }

  fread(eeprom, 1, eepromSize, f);
  fclose(f);
}

PIF::~PIF() {
  FILE* f = fopen(mempakPath.c_str(), "wb");
  fwrite(mempak, 1, MEMPAK_SIZE, f);
  fclose(f);
  f = fopen(eepromPath.c_str(), "wb");
  fwrite(eeprom, 1, eepromSize, f);
  fclose(f);
}

enum CMDIndexes {
  CMD_LEN = 0,
  CMD_RES_LEN,
  CMD_IDX,
  CMD_START
};

void PIF::CICChallenge() {
  u8 challenge[30];
  u8 response[30];

  // Split 15 bytes into 30 nibbles
  for (int i = 0; i < 15; i++) {
    challenge[i * 2 + 0] = (ram[0x30 + i] >> 4) & 0x0F;
    challenge[i * 2 + 1] = (ram[0x30 + i] >> 0) & 0x0F;
  }

  n64_cic_nus_6105((char*)challenge, (char*)response, CHL_LEN - 2);

  for (int i = 0; i < 15; i++) {
    ram[0x30 + i] = (response[i * 2] << 4) + response[i * 2 + 1];
  }
}

inline u8 data_crc(const u8* data) {
  u8 crc = 0;
  for (int i = 0; i <= 32; i++) {
    for (int j = 7; j >= 0; j--) {
      u8 xor_val = ((crc & 0x80) != 0) ? 0x85 : 0x00;

      crc <<= 1;
      if (i < 32) {
        if ((data[i] & (1 << j)) != 0) {
          crc |= 1;
        }
      }

      crc ^= xor_val;
    }
  }

  return crc;
}

void PIF::ProcessCommands(Mem &mem) {
  u8 control = ram[63];
  if (control & 1) {
    channel = 0;
    int i = 0;
    while (i < 63) {
      u8* cmd = &ram[i++];
      u8 cmdlen = cmd[CMD_LEN] & 0x3F;

      if (cmdlen == 0) {
        channel++;
      } else if (cmdlen == 0x3D) {
        channel++;
      } else if (cmdlen == 0x3E) {
        break;
      } else if (cmdlen == 0x3F) {
        continue;
      } else {
        u8 r = ram[i++];
        if (r == 0xFE) {
          break;
        }
        u8 reslen = r & 0x3F;
        u8* res = &ram[i + cmdlen];

        switch (cmd[CMD_IDX]) {
          case 0xff:
            ControllerID(res);
            channel++;
            break;
          case 0:
            ControllerID(res);
            channel++;
            break;
          case 1:
            UpdateController();
            if(!ReadButtons(res)) {
              cmd[1] |= 0x80;
            }
            channel++;
            break;
          case 2:
            MempakRead(cmd, res);
            break;
          case 3:
            MempakWrite(cmd, res);
            break;
          case 4:
            EepromRead(cmd, res, mem);
            break;
          case 5:
            EepromWrite(cmd, res, mem);
            break;
          default:
            Util::panic("Invalid PIF command: {:X}", cmd[2]);
        }

        i += cmdlen + reslen;
      }
    }
  }

  if (control & 0x02) {
    CICChallenge();
    ram[63] &= ~2;
  }

  if (control & 0x08) {
    ram[63] &= ~8;
  }

  if (control & 0x30) {
    ram[63] = 0x80;
  }
}

void PIF::MempakRead(u8* cmd, u8* res) const {
  u16 offset = cmd[3] << 8;
  offset |= cmd[4];

  // low 5 bits are the CRC
  //byte crc = offset & 0x1F;
  // offset must be 32-byte aligned
  offset &= ~0x1F;

  switch (getAccessoryType()) {
    case ACCESSORY_NONE:
      break;
    case ACCESSORY_MEMPACK:
      if (offset <= MEMPAK_SIZE - 0x20) {
        for (int i = 0; i < 32; i++) {
          res[i] = mempak[offset + i];
        }
      }
      break;
    case ACCESSORY_RUMBLE_PACK:
      for (int i = 0; i < 32; i++) {
        res[i] = 0x80;
      }
      break;
  }

  // CRC byte
  res[32] = data_crc(&res[0]);
}

void PIF::MempakWrite(u8* cmd, u8* res) const {
  // First two bytes in the command are the offset
  u16 offset = cmd[3] << 8;
  offset |= cmd[4];

  // low 5 bits are the CRC
  //byte crc = offset & 0x1F;
  // offset must be 32-byte aligned
  offset &= ~0x1F;

  switch (getAccessoryType()) {
    case ACCESSORY_NONE:
      break;
    case ACCESSORY_MEMPACK:
      if (offset <= MEMPAK_SIZE - 0x20) {
        for (int i = 0; i < 32; i++) {
          mempak[offset + i] = cmd[5 + i];
        }
      }
      break;
    case ACCESSORY_RUMBLE_PACK: break;
  }
  // CRC byte
  res[0] = data_crc(&cmd[5]);
}

void PIF::EepromRead(u8* cmd, u8* res, const Mem& mem) const {
  assert(mem.saveType == SAVE_EEPROM_4k || mem.saveType == SAVE_EEPROM_16k);
  if (channel == 4) {
    u8 offset = cmd[3];
    if ((offset * 8) >= getSaveSize(mem.saveType)) {
      Util::panic("Out of range EEPROM read! offset: {:02X}", offset);
    }

    for (int i = 0; i < 8; i++) {
      res[i] = eeprom[(offset * 8) + i];
    }
  }
  else {
    Util::panic("EEPROM read on bad channel {}", channel);
  }
}

void PIF::EepromWrite(u8* cmd, u8* res, const Mem& mem) const {
  assert(mem.saveType == SAVE_EEPROM_4k || mem.saveType == SAVE_EEPROM_16k);
  if (channel == 4) {
    u8 offset = cmd[3];
    if ((offset * 8) >= getSaveSize(mem.saveType)) {
      Util::panic("Out of range EEPROM write! offset: {:02X}", offset);
    }

    for (int i = 0; i < 8; i++) {
      eeprom[(offset * 8) + i] = cmd[4 + i];
    }

    res[0] = 0; // Error byte, I guess it always succeeds?
  }
  else {
    Util::panic("EEPROM write on bad channel {}", channel);
  }
}

#define GET_BUTTON(gamecontroller, i) SDL_GameControllerGetButton(gamecontroller, i)
#define GET_AXIS(gamecontroller, axis) SDL_GameControllerGetAxis(gamecontroller, axis)

void PIF::UpdateController() {
  if(gamepadConnected) {
    bool A = GET_BUTTON(gamepad, SDL_CONTROLLER_BUTTON_A);
    bool B = GET_BUTTON(gamepad, SDL_CONTROLLER_BUTTON_X);
    bool Z = GET_AXIS(gamepad, SDL_CONTROLLER_AXIS_TRIGGERLEFT) == SDL_JOYSTICK_AXIS_MAX;
    bool START = GET_BUTTON(gamepad, SDL_CONTROLLER_BUTTON_START);
    bool DUP = GET_BUTTON(gamepad, SDL_CONTROLLER_BUTTON_DPAD_UP);
    bool DDOWN = GET_BUTTON(gamepad, SDL_CONTROLLER_BUTTON_DPAD_DOWN);
    bool DLEFT = GET_BUTTON(gamepad, SDL_CONTROLLER_BUTTON_DPAD_LEFT);
    bool DRIGHT = GET_BUTTON(gamepad, SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
    bool L = GET_BUTTON(gamepad, SDL_CONTROLLER_BUTTON_LEFTSHOULDER);
    bool R = GET_BUTTON(gamepad, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);
    bool CUP = GET_AXIS(gamepad, SDL_CONTROLLER_AXIS_RIGHTY) <= -127;
    bool CDOWN = GET_AXIS(gamepad, SDL_CONTROLLER_AXIS_RIGHTY) >= 127;
    bool CLEFT = GET_AXIS(gamepad, SDL_CONTROLLER_AXIS_RIGHTX) <= -127;
    bool CRIGHT = GET_AXIS(gamepad, SDL_CONTROLLER_AXIS_RIGHTX) >= 127;

    joybusDevices[channel].controller.a = A;
    joybusDevices[channel].controller.b = B;
    joybusDevices[channel].controller.z = Z;
    joybusDevices[channel].controller.start = START;
    joybusDevices[channel].controller.dp_up = DUP;
    joybusDevices[channel].controller.dp_down = DDOWN;
    joybusDevices[channel].controller.dp_left = DLEFT;
    joybusDevices[channel].controller.dp_right = DRIGHT;
    joybusDevices[channel].controller.joy_reset = L && R && START;
    joybusDevices[channel].controller.l = L;
    joybusDevices[channel].controller.r = R;
    joybusDevices[channel].controller.c_up = CUP;
    joybusDevices[channel].controller.c_down = CDOWN;
    joybusDevices[channel].controller.c_left = CLEFT;
    joybusDevices[channel].controller.c_right = CRIGHT;

    float xclamped = GET_AXIS(gamepad, SDL_CONTROLLER_AXIS_LEFTX);
    if(xclamped < 0) {
      xclamped /= float(std::abs(SDL_JOYSTICK_AXIS_MIN));
    } else {
      xclamped /= SDL_JOYSTICK_AXIS_MAX;
    }

    xclamped *= 86;

    float yclamped = GET_AXIS(gamepad, SDL_CONTROLLER_AXIS_LEFTY);
    if(yclamped < 0) {
      yclamped /= float(std::abs(SDL_JOYSTICK_AXIS_MIN));
    } else {
      yclamped /= SDL_JOYSTICK_AXIS_MAX;
    }
    yclamped *= 86;

    joybusDevices[channel].controller.joy_x = xclamped;
    joybusDevices[channel].controller.joy_y = -yclamped;

    if (joybusDevices[channel].controller.joy_reset) {
      joybusDevices[channel].controller.start = false;
      joybusDevices[channel].controller.joy_x = 0;
      joybusDevices[channel].controller.joy_y = 0;
    }
  } else {
    const uint8_t* state = SDL_GetKeyboardState(nullptr);
    joybusDevices[channel].controller.a = state[SDL_SCANCODE_X];
    joybusDevices[channel].controller.b = state[SDL_SCANCODE_C];
    joybusDevices[channel].controller.z = state[SDL_SCANCODE_Z];
    joybusDevices[channel].controller.start = state[SDL_SCANCODE_RETURN];
    joybusDevices[channel].controller.dp_up = state[SDL_SCANCODE_KP_8];
    joybusDevices[channel].controller.dp_down = state[SDL_SCANCODE_KP_5];
    joybusDevices[channel].controller.dp_left = state[SDL_SCANCODE_KP_4];
    joybusDevices[channel].controller.dp_right = state[SDL_SCANCODE_KP_6];
    joybusDevices[channel].controller.joy_reset = state[SDL_SCANCODE_RETURN] && state[SDL_SCANCODE_A] && state[SDL_SCANCODE_S];
    joybusDevices[channel].controller.l = state[SDL_SCANCODE_A];
    joybusDevices[channel].controller.r = state[SDL_SCANCODE_S];
    joybusDevices[channel].controller.c_up = state[SDL_SCANCODE_I];
    joybusDevices[channel].controller.c_down = state[SDL_SCANCODE_J];
    joybusDevices[channel].controller.c_left = state[SDL_SCANCODE_K];
    joybusDevices[channel].controller.c_right = state[SDL_SCANCODE_L];

    s16 xaxis = 0, yaxis = 0;
    if (state[SDL_SCANCODE_LEFT]) {
      xaxis = -86;
    } else if (state[SDL_SCANCODE_RIGHT]) {
      xaxis = 86;
    }

    if (state[SDL_SCANCODE_DOWN]) {
      yaxis = -86;
    } else if (state[SDL_SCANCODE_UP]) {
      yaxis = 86;
    }

    joybusDevices[channel].controller.joy_x = xaxis;
    joybusDevices[channel].controller.joy_y = yaxis;

    if (joybusDevices[channel].controller.joy_reset) {
      joybusDevices[channel].controller.start = false;
      joybusDevices[channel].controller.joy_x = 0;
      joybusDevices[channel].controller.joy_y = 0;
    }
  }
}

void PIF::DoPIFHLE(Mem& mem, Registers& regs, bool pal, CICType cicType) {
  mem.Write32(regs, PIF_RAM_REGION_START + 0x24, cicSeeds[cicType]);

  switch(cicType) {
    case UNKNOWN_CIC_TYPE:
      Util::warn("Unknown CIC type!");
      break;
    case CIC_NUS_6101:
      regs.gpr[0] = 0x0000000000000000;
      regs.gpr[1] = 0x0000000000000000;
      regs.gpr[2] = 0xFFFFFFFFDF6445CC;
      regs.gpr[3] = 0xFFFFFFFFDF6445CC;
      regs.gpr[4] = 0x00000000000045CC;
      regs.gpr[5] = 0x0000000073EE317A;
      regs.gpr[6] = 0xFFFFFFFFA4001F0C;
      regs.gpr[7] = 0xFFFFFFFFA4001F08;
      regs.gpr[8] = 0x00000000000000C0;
      regs.gpr[9] = 0x0000000000000000;
      regs.gpr[10] = 0x0000000000000040;
      regs.gpr[11] = 0xFFFFFFFFA4000040;
      regs.gpr[12] = 0xFFFFFFFFC7601FAC;
      regs.gpr[13] = 0xFFFFFFFFC7601FAC;
      regs.gpr[14] = 0xFFFFFFFFB48E2ED6;
      regs.gpr[15] = 0xFFFFFFFFBA1A7D4B;
      regs.gpr[16] = 0x0000000000000000;
      regs.gpr[17] = 0x0000000000000000;
      regs.gpr[18] = 0x0000000000000000;
      regs.gpr[19] = 0x0000000000000000;
      regs.gpr[20] = 0x0000000000000001;
      regs.gpr[21] = 0x0000000000000000;
      regs.gpr[23] = 0x0000000000000001;
      regs.gpr[24] = 0x0000000000000002;
      regs.gpr[25] = 0xFFFFFFFF905F4718;
      regs.gpr[26] = 0x0000000000000000;
      regs.gpr[27] = 0x0000000000000000;
      regs.gpr[28] = 0x0000000000000000;
      regs.gpr[29] = 0xFFFFFFFFA4001FF0;
      regs.gpr[30] = 0x0000000000000000;
      regs.gpr[31] = 0xFFFFFFFFA4001550;

      regs.lo = 0xFFFFFFFFBA1A7D4B;
      regs.hi = 0xFFFFFFFF997EC317;
      break;
    case CIC_NUS_7102:
      regs.gpr[0]  = 0x0000000000000000;
      regs.gpr[1]  = 0x0000000000000001;
      regs.gpr[2]  = 0x000000001E324416;
      regs.gpr[3]  = 0x000000001E324416;
      regs.gpr[4]  = 0x0000000000004416;
      regs.gpr[5]  = 0x000000000EC5D9AF;
      regs.gpr[6]  = 0xFFFFFFFFA4001F0C;
      regs.gpr[7]  = 0xFFFFFFFFA4001F08;
      regs.gpr[8]  = 0x00000000000000C0;
      regs.gpr[9]  = 0x0000000000000000;
      regs.gpr[10] = 0x0000000000000040;
      regs.gpr[11] = 0xFFFFFFFFA4000040;
      regs.gpr[12] = 0x00000000495D3D7B;
      regs.gpr[13] = 0xFFFFFFFF8B3DFA1E;
      regs.gpr[14] = 0x000000004798E4D4;
      regs.gpr[15] = 0xFFFFFFFFF1D30682;
      regs.gpr[16] = 0x0000000000000000;
      regs.gpr[17] = 0x0000000000000000;
      regs.gpr[18] = 0x0000000000000000;
      regs.gpr[19] = 0x0000000000000000;
      regs.gpr[20] = 0x0000000000000000;
      regs.gpr[21] = 0x0000000000000000;
      regs.gpr[22] = 0x000000000000003F;
      regs.gpr[23] = 0x0000000000000007;
      regs.gpr[24] = 0x0000000000000000;
      regs.gpr[25] = 0x0000000013D05CAB;
      regs.gpr[26] = 0x0000000000000000;
      regs.gpr[27] = 0x0000000000000000;
      regs.gpr[28] = 0x0000000000000000;
      regs.gpr[29] = 0xFFFFFFFFA4001FF0;
      regs.gpr[30] = 0x0000000000000000;
      regs.gpr[31] = 0xFFFFFFFFA4001554;

      regs.lo = 0xFFFFFFFFF1D30682;
      regs.hi = 0x0000000010054A98;
      break;
    case CIC_NUS_6102_7101:
      regs.gpr[0]  = 0x0000000000000000;
      regs.gpr[1]  = 0x0000000000000001;
      regs.gpr[2]  = 0x000000000EBDA536;
      regs.gpr[3]  = 0x000000000EBDA536;
      regs.gpr[4]  = 0x000000000000A536;
      regs.gpr[5]  = 0xFFFFFFFFC0F1D859;
      regs.gpr[6]  = 0xFFFFFFFFA4001F0C;
      regs.gpr[7]  = 0xFFFFFFFFA4001F08;
      regs.gpr[8]  = 0x00000000000000C0;
      regs.gpr[9]  = 0x0000000000000000;
      regs.gpr[10] = 0x0000000000000040;
      regs.gpr[11] = 0xFFFFFFFFA4000040;
      regs.gpr[12] = 0xFFFFFFFFED10D0B3;
      regs.gpr[13] = 0x000000001402A4CC;
      regs.gpr[14] = 0x000000002DE108EA;
      regs.gpr[15] = 0x000000003103E121;
      regs.gpr[16] = 0x0000000000000000;
      regs.gpr[17] = 0x0000000000000000;
      regs.gpr[18] = 0x0000000000000000;
      regs.gpr[19] = 0x0000000000000000;
      regs.gpr[20] = 0x0000000000000001;
      regs.gpr[21] = 0x0000000000000000;
      regs.gpr[23] = 0x0000000000000000;
      regs.gpr[24] = 0x0000000000000000;
      regs.gpr[25] = 0xFFFFFFFF9DEBB54F;
      regs.gpr[26] = 0x0000000000000000;
      regs.gpr[27] = 0x0000000000000000;
      regs.gpr[28] = 0x0000000000000000;
      regs.gpr[29] = 0xFFFFFFFFA4001FF0;
      regs.gpr[30] = 0x0000000000000000;
      regs.gpr[31] = 0xFFFFFFFFA4001550;

      regs.hi = 0x000000003FC18657;
      regs.lo = 0x000000003103E121;

      if (pal) {
        regs.gpr[20] = 0x0000000000000000;
        regs.gpr[23] = 0x0000000000000006;
        regs.gpr[31] = 0xFFFFFFFFA4001554;
      }
      break;
    case CIC_NUS_6103_7103:
      regs.gpr[0]  = 0x0000000000000000;
      regs.gpr[1]  = 0x0000000000000001;
      regs.gpr[2]  = 0x0000000049A5EE96;
      regs.gpr[3]  = 0x0000000049A5EE96;
      regs.gpr[4]  = 0x000000000000EE96;
      regs.gpr[5]  = 0xFFFFFFFFD4646273;
      regs.gpr[6]  = 0xFFFFFFFFA4001F0C;
      regs.gpr[7]  = 0xFFFFFFFFA4001F08;
      regs.gpr[8]  = 0x00000000000000C0;
      regs.gpr[9]  = 0x0000000000000000;
      regs.gpr[10] = 0x0000000000000040;
      regs.gpr[11] = 0xFFFFFFFFA4000040;
      regs.gpr[12] = 0xFFFFFFFFCE9DFBF7;
      regs.gpr[13] = 0xFFFFFFFFCE9DFBF7;
      regs.gpr[14] = 0x000000001AF99984;
      regs.gpr[15] = 0x0000000018B63D28;
      regs.gpr[16] = 0x0000000000000000;
      regs.gpr[17] = 0x0000000000000000;
      regs.gpr[18] = 0x0000000000000000;
      regs.gpr[19] = 0x0000000000000000;
      regs.gpr[20] = 0x0000000000000001;
      regs.gpr[21] = 0x0000000000000000;
      regs.gpr[23] = 0x0000000000000000;
      regs.gpr[24] = 0x0000000000000000;
      regs.gpr[25] = 0xFFFFFFFF825B21C9;
      regs.gpr[26] = 0x0000000000000000;
      regs.gpr[27] = 0x0000000000000000;
      regs.gpr[28] = 0x0000000000000000;
      regs.gpr[29] = 0xFFFFFFFFA4001FF0;
      regs.gpr[30] = 0x0000000000000000;
      regs.gpr[31] = 0xFFFFFFFFA4001550;

      regs.lo = 0x0000000018B63D28;
      regs.hi = 0x00000000625C2BBE;

      if (pal) {
        regs.gpr[20] = 0x0000000000000000;
        regs.gpr[23] = 0x0000000000000006;
        regs.gpr[31] = 0xFFFFFFFFA4001554;
      }
      break;
    case CIC_NUS_6105_7105:
      regs.gpr[0]  = 0x0000000000000000;
      regs.gpr[1]  = 0x0000000000000000;
      regs.gpr[2]  = 0xFFFFFFFFF58B0FBF;
      regs.gpr[3]  = 0xFFFFFFFFF58B0FBF;
      regs.gpr[4]  = 0x0000000000000FBF;
      regs.gpr[5]  = 0xFFFFFFFFDECAAAD1;
      regs.gpr[6]  = 0xFFFFFFFFA4001F0C;
      regs.gpr[7]  = 0xFFFFFFFFA4001F08;
      regs.gpr[8]  = 0x00000000000000C0;
      regs.gpr[9]  = 0x0000000000000000;
      regs.gpr[10] = 0x0000000000000040;
      regs.gpr[11] = 0xFFFFFFFFA4000040;
      regs.gpr[12] = 0xFFFFFFFF9651F81E;
      regs.gpr[13] = 0x000000002D42AAC5;
      regs.gpr[14] = 0x00000000489B52CF;
      regs.gpr[15] = 0x0000000056584D60;
      regs.gpr[16] = 0x0000000000000000;
      regs.gpr[17] = 0x0000000000000000;
      regs.gpr[18] = 0x0000000000000000;
      regs.gpr[19] = 0x0000000000000000;
      regs.gpr[20] = 0x0000000000000001;
      regs.gpr[21] = 0x0000000000000000;
      regs.gpr[23] = 0x0000000000000000;
      regs.gpr[24] = 0x0000000000000002;
      regs.gpr[25] = 0xFFFFFFFFCDCE565F;
      regs.gpr[26] = 0x0000000000000000;
      regs.gpr[27] = 0x0000000000000000;
      regs.gpr[28] = 0x0000000000000000;
      regs.gpr[29] = 0xFFFFFFFFA4001FF0;
      regs.gpr[30] = 0x0000000000000000;
      regs.gpr[31] = 0xFFFFFFFFA4001550;

      regs.lo = 0x0000000056584D60;
      regs.hi = 0x000000004BE35D1F;

      if (pal) {
        regs.gpr[20] = 0x0000000000000000;
        regs.gpr[23] = 0x0000000000000006;
        regs.gpr[31] = 0xFFFFFFFFA4001554;
      }

      mem.Write32(regs, IMEM_REGION_START + 0x00, 0x3C0DBFC0);
      mem.Write32(regs, IMEM_REGION_START + 0x04, 0x8DA807FC);
      mem.Write32(regs, IMEM_REGION_START + 0x08, 0x25AD07C0);
      mem.Write32(regs, IMEM_REGION_START + 0x0C, 0x31080080);
      mem.Write32(regs, IMEM_REGION_START + 0x10, 0x5500FFFC);
      mem.Write32(regs, IMEM_REGION_START + 0x14, 0x3C0DBFC0);
      mem.Write32(regs, IMEM_REGION_START + 0x18, 0x8DA80024);
      mem.Write32(regs, IMEM_REGION_START + 0x1C, 0x3C0BB000);
      break;
    case CIC_NUS_6106_7106:
      regs.gpr[0] = 0x0000000000000000;
      regs.gpr[1] = 0x0000000000000000;
      regs.gpr[2] = 0xFFFFFFFFA95930A4;
      regs.gpr[3] = 0xFFFFFFFFA95930A4;
      regs.gpr[4] = 0x00000000000030A4;
      regs.gpr[5] = 0xFFFFFFFFB04DC903;
      regs.gpr[6] = 0xFFFFFFFFA4001F0C;
      regs.gpr[7] = 0xFFFFFFFFA4001F08;
      regs.gpr[8] = 0x00000000000000C0;
      regs.gpr[9] = 0x0000000000000000;
      regs.gpr[10] = 0x0000000000000040;
      regs.gpr[11] = 0xFFFFFFFFA4000040;
      regs.gpr[12] = 0xFFFFFFFFBCB59510;
      regs.gpr[13] = 0xFFFFFFFFBCB59510;
      regs.gpr[14] = 0x000000000CF85C13;
      regs.gpr[15] = 0x000000007A3C07F4;
      regs.gpr[16] = 0x0000000000000000;
      regs.gpr[17] = 0x0000000000000000;
      regs.gpr[18] = 0x0000000000000000;
      regs.gpr[19] = 0x0000000000000000;
      regs.gpr[20] = 0x0000000000000001;
      regs.gpr[21] = 0x0000000000000000;
      regs.gpr[23] = 0x0000000000000000;
      regs.gpr[24] = 0x0000000000000002;
      regs.gpr[25] = 0x00000000465E3F72;
      regs.gpr[26] = 0x0000000000000000;
      regs.gpr[27] = 0x0000000000000000;
      regs.gpr[28] = 0x0000000000000000;
      regs.gpr[29] = 0xFFFFFFFFA4001FF0;
      regs.gpr[30] = 0x0000000000000000;
      regs.gpr[31] = 0xFFFFFFFFA4001550;
      regs.lo = 0x000000007A3C07F4;
      regs.hi = 0x0000000023953898;

      if (pal) {
        regs.gpr[20] = 0x0000000000000000;
        regs.gpr[23] = 0x0000000000000006;
        regs.gpr[31] = 0xFFFFFFFFA4001554;
      }
      break;
  }

  regs.gpr[22] = (cicSeeds[cicType] >> 8) & 0xFF;
  regs.cop0.Reset();
  mem.Write32(regs, 0x04300004, 0x01010101);
  memcpy(mem.mmio.rsp.dmem, mem.rom.cart, 0x1000);
  regs.SetPC32(0xA4000040);
}

void PIF::ExecutePIF(Mem& mem, Registers& regs) {
  CICType cicType = mem.rom.cicType;
  bool pal = mem.rom.pal;
  mem.Write32(regs, PIF_RAM_REGION_START + 0x24, cicSeeds[cicType]);
  switch(cicType) {
    case UNKNOWN_CIC_TYPE:
      Util::warn("Unknown CIC type!");
      break;
    case CIC_NUS_6101 ... CIC_NUS_6103_7103:
      mem.Write32(regs, 0x318, RDRAM_SIZE);
      break;
    case CIC_NUS_6105_7105:
      mem.Write32(regs, 0x3F0, RDRAM_SIZE);
      break;
    case CIC_NUS_6106_7106:
      break;
  }

  DoPIFHLE(mem, regs, pal, cicType);
}
}