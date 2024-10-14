#include <Netplay.hpp>
#include <PIF.hpp>
#include <PIF/MupenMovie.hpp>
#include <log.hpp>

namespace n64 {
void PIF::InitDevices(SaveType saveType) {
  joybusDevices[0].type = JOYBUS_CONTROLLER;
  joybusDevices[0].accessoryType = ACCESSORY_MEMPACK;
  for (int i = 1; i < 4; i++) { // TODO: make this configurable
    joybusDevices[i].type = JOYBUS_NONE;
    joybusDevices[i].accessoryType = ACCESSORY_NONE;
  }

  if (saveType == SAVE_EEPROM_4k) {
    joybusDevices[4].type = JOYBUS_4KB_EEPROM;
  } else if (saveType == SAVE_EEPROM_16k) {
    joybusDevices[4].type = JOYBUS_16KB_EEPROM;
  } else {
    joybusDevices[4].type = JOYBUS_NONE;
  }
  joybusDevices[5].type = JOYBUS_NONE;
}

void PIF::ControllerID(u8 *res) const {
  if (channel < 6) {
    switch (joybusDevices[channel].type) {
    case JOYBUS_NONE:
      res[0] = 0x00;
      res[1] = 0x00;
      res[2] = 0x00;
      break;
    case JOYBUS_CONTROLLER:
      res[0] = 0x05;
      res[1] = 0x00;
      res[2] = joybusDevices[channel].accessoryType != ACCESSORY_NONE ? 0x01 : 0x02;
      break;
    case JOYBUS_DANCEPAD:
      res[0] = 0x05;
      res[1] = 0x00;
      res[2] = 0x00;
      break;
    case JOYBUS_VRU:
      res[0] = 0x00;
      res[1] = 0x01;
      res[2] = 0x00;
      break;
    case JOYBUS_MOUSE:
      res[0] = 0x02;
      res[1] = 0x00;
      res[2] = 0x00;
      break;
    case JOYBUS_RANDNET_KEYBOARD:
      res[0] = 0x00;
      res[1] = 0x02;
      res[2] = 0x00;
      break;
    case JOYBUS_DENSHA_DE_GO:
      res[0] = 0x20;
      res[1] = 0x04;
      res[2] = 0x00;
      break;
    case JOYBUS_4KB_EEPROM:
      res[0] = 0x00;
      res[1] = 0x80;
      res[2] = 0x00;
      break;
    case JOYBUS_16KB_EEPROM:
      res[0] = 0x00;
      res[1] = 0xC0;
      res[2] = 0x00;
      break;
    }
  } else {
    Util::panic("Device ID on unknown channel {}", channel);
  }
}

bool PIF::ReadButtons(u8 *res) {
  if (channel >= 6) {
    res[0] = 0;
    res[1] = 0;
    res[2] = 0;
    res[3] = 0;
    return false;
  }

  switch (joybusDevices[channel].type) {
  case JOYBUS_NONE:
    res[0] = 0x00;
    res[1] = 0x00;
    res[2] = 0x00;
    res[3] = 0x00;
    return false; // Device not present
  case JOYBUS_4KB_EEPROM:
  case JOYBUS_16KB_EEPROM:
  case JOYBUS_CONTROLLER:
    if (movie.IsLoaded()) {
      const Controller controller = movie.NextInputs();
      res[0] = controller.byte1;
      res[1] = controller.byte2;
      res[2] = controller.joyX;
      res[3] = controller.joyY;
    } else {
      res[0] = joybusDevices[channel].controller.byte1;
      res[1] = joybusDevices[channel].controller.byte2;
      res[2] = joybusDevices[channel].controller.joyX;
      res[3] = joybusDevices[channel].controller.joyY;
    }
    return true;
  case JOYBUS_DANCEPAD:
  case JOYBUS_VRU:
  case JOYBUS_MOUSE:
  case JOYBUS_RANDNET_KEYBOARD:
  case JOYBUS_DENSHA_DE_GO:
    return false;
  }

  return true;
}
} // namespace n64
