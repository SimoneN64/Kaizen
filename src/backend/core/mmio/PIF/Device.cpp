#include <PIF.hpp>
#include <MupenMovie.hpp>
#include <Netplay.hpp>
#include <log.hpp>
#include <SDL2/SDL_keyboard.h>

namespace n64 {
void PIF::InitDevices(SaveType saveType) {
  joybusDevices[0].type = JOYBUS_CONTROLLER;
  joybusDevices[0].accessoryType = ACCESSORY_MEMPACK;
  for (int i = 1; i < 4; i++) { //TODO: make this configurable
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

#define GET_BUTTON(gamecontroller, i) SDL_GameControllerGetButton(gamecontroller, i)
#define GET_AXIS(gamecontroller, axis) SDL_GameControllerGetAxis(gamecontroller, axis)

void PIF::PollController() {
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
    joybusDevices[channel].controller.dp_up = state[SDL_SCANCODE_PAGEUP];
    joybusDevices[channel].controller.dp_down = state[SDL_SCANCODE_PAGEDOWN];
    joybusDevices[channel].controller.dp_left = state[SDL_SCANCODE_HOME];
    joybusDevices[channel].controller.dp_right = state[SDL_SCANCODE_END];
    joybusDevices[channel].controller.joy_reset = state[SDL_SCANCODE_RETURN] && state[SDL_SCANCODE_A] && state[SDL_SCANCODE_S];
    joybusDevices[channel].controller.l = state[SDL_SCANCODE_A];
    joybusDevices[channel].controller.r = state[SDL_SCANCODE_S];
    joybusDevices[channel].controller.c_up = state[SDL_SCANCODE_I];
    joybusDevices[channel].controller.c_down = state[SDL_SCANCODE_K];
    joybusDevices[channel].controller.c_left = state[SDL_SCANCODE_J];
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

bool PIF::ReadButtons(u8* res) const {
  if(channel >= 6) {
    res[0] = 0;
    res[1] = 0;
    res[2] = 0;
    res[3] = 0;
    return false;
  }

  switch (joybusDevices[channel].type) {
    case JOYBUS_NONE:
      res[0]  = 0x00;
      res[1]  = 0x00;
      res[2]  = 0x00;
      res[3]  = 0x00;
      return false; // Device not present
    case JOYBUS_CONTROLLER:
      if (TasMovieLoaded()) {
        Controller controller = TasNextInputs();
        res[0] = controller.byte1;
        res[1] = controller.byte2;
        res[2] = controller.joy_x;
        res[3] = controller.joy_y;
      } else {
        res[0] = joybusDevices[channel].controller.byte1;
        res[1] = joybusDevices[channel].controller.byte2;
        res[2] = joybusDevices[channel].controller.joy_x;
        res[3] = joybusDevices[channel].controller.joy_y;
      }
      return true;
    case JOYBUS_DANCEPAD:
    case JOYBUS_VRU:
    case JOYBUS_MOUSE:
    case JOYBUS_RANDNET_KEYBOARD:
    case JOYBUS_DENSHA_DE_GO:
    case JOYBUS_4KB_EEPROM:
    case JOYBUS_16KB_EEPROM:
      return false;
  }

  return true;
}
}