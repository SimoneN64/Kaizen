#include <PIF.hpp>
#include <MupenMovie.hpp>
#include <PIF/Device.hpp>
#include <Netplay.hpp>
#include "log.hpp"
#include <SDL2/SDL_keyboard.h>

namespace n64 {
JoybusDevice players[4]{};

void PIF::InitDevices(SaveType saveType) {
  for (int i = 0; i < 4; i++) { //TODO: make this configurable
    joybusDevices[i].type = JOYBUS_CONTROLLER;
    joybusDevices[i].accessoryType = ACCESSORY_MEMPACK;
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

    players[0].controller.a = A;
    players[0].controller.b = B;
    players[0].controller.z = Z;
    players[0].controller.start = START;
    players[0].controller.dp_up = DUP;
    players[0].controller.dp_down = DDOWN;
    players[0].controller.dp_left = DLEFT;
    players[0].controller.dp_right = DRIGHT;
    players[0].controller.joy_reset = L && R && START;
    players[0].controller.l = L;
    players[0].controller.r = R;
    players[0].controller.c_up = CUP;
    players[0].controller.c_down = CDOWN;
    players[0].controller.c_left = CLEFT;
    players[0].controller.c_right = CRIGHT;

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

    players[0].controller.joy_x = xclamped;
    players[0].controller.joy_y = -yclamped;

    if (players[0].controller.joy_reset) {
      players[0].controller.start = false;
      players[0].controller.joy_x = 0;
      players[0].controller.joy_y = 0;
    }
  } else {
    const uint8_t* state = SDL_GetKeyboardState(nullptr);
    players[0].controller.a = state[SDL_SCANCODE_X];
    players[0].controller.b = state[SDL_SCANCODE_C];
    players[0].controller.z = state[SDL_SCANCODE_Z];
    players[0].controller.start = state[SDL_SCANCODE_RETURN];
    players[0].controller.dp_up = state[SDL_SCANCODE_PAGEUP];
    players[0].controller.dp_down = state[SDL_SCANCODE_PAGEDOWN];
    players[0].controller.dp_left = state[SDL_SCANCODE_HOME];
    players[0].controller.dp_right = state[SDL_SCANCODE_END];
    players[0].controller.joy_reset = state[SDL_SCANCODE_RETURN] && state[SDL_SCANCODE_A] && state[SDL_SCANCODE_S];
    players[0].controller.l = state[SDL_SCANCODE_A];
    players[0].controller.r = state[SDL_SCANCODE_S];
    players[0].controller.c_up = state[SDL_SCANCODE_I];
    players[0].controller.c_down = state[SDL_SCANCODE_K];
    players[0].controller.c_left = state[SDL_SCANCODE_J];
    players[0].controller.c_right = state[SDL_SCANCODE_L];

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

    players[0].controller.joy_x = xaxis;
    players[0].controller.joy_y = yaxis;

    if (players[0].controller.joy_reset) {
      players[0].controller.start = false;
      players[0].controller.joy_x = 0;
      players[0].controller.joy_y = 0;
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