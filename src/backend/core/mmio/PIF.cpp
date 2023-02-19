#include <core/mmio/PIF.hpp>
#include <core/Mem.hpp>
#include <core/registers/Registers.hpp>
#include <log.hpp>
#include <MupenMovie.hpp>
#include <SDL_keyboard.h>

namespace n64 {
static int channel = 0;

void PIF::ProcessPIFCommands(Mem& mem) {
  u8 control = pifRam[63];

  if(control & 1) {
    channel = 0;
    for(int i = 0; i < 63;) {
      u8* cmd = &pifRam[i++];
      u8 t = cmd[0] & 0x3f;

      if(t == 0 || t == 0x3D) {
        channel++;
      } else if (t == 0x3E) {
        break;
      } else if (t == 0x3F) {
        continue;
      } else {
        u8 r = pifRam[i++];
        r |= (1 << 7);
        if(r == 0xFE) {
          break;
        }

        u8 rlen = r & 0x3F;
        u8* res = &pifRam[i + t];
        switch(cmd[2]) {
          case 0xff:
            res[0] = 0x05;
            res[1] = 0x00;
            res[2] = 0x01;
            channel++;
            break;
          case 0:
            res[0] = 0x05;
            res[1] = 0x00;
            res[2] = 0x01;
            break;
          case 1:
            if(tas_movie_loaded()) {
              controller = tas_next_inputs();
            } else {
              UpdateController();
            }
            res[0] = controller.byte1;
            res[1] = controller.byte2;
            res[2] = controller.joy_x;
            res[3] = controller.joy_y;
            break;
          case 2: case 3: res[0] = 0; break;
          default: Util::panic("Unimplemented PIF command {}", cmd[2]);
        }

        i += t + rlen;
      }
    }
  }

  if(control & 8) {
    pifRam[63] &= ~8;
  }

  if (control & 0x30) {
    pifRam[63] = 0x80;
  }
}

#define GET_BUTTON(gamepad, i) SDL_GameControllerGetButton(gamepad, i)
#define GET_AXIS(gamepad, axis) SDL_GameControllerGetAxis(gamepad, axis)

void PIF::UpdateController() {
  const uint8_t* state = SDL_GetKeyboardState(nullptr);
  s8 xaxis = 0, yaxis = 0;

  if(gamepadConnected) {
    bool A = GET_BUTTON(gamepad, SDL_CONTROLLER_BUTTON_A);
    bool B = GET_BUTTON(gamepad, SDL_CONTROLLER_BUTTON_X);
    bool Z = GET_AXIS(gamepad, SDL_CONTROLLER_AXIS_TRIGGERLEFT) == 32767;
    bool START = GET_BUTTON(gamepad, SDL_CONTROLLER_BUTTON_START);
    bool DUP = GET_BUTTON(gamepad, SDL_CONTROLLER_BUTTON_DPAD_UP);
    bool DDOWN = GET_BUTTON(gamepad, SDL_CONTROLLER_BUTTON_DPAD_DOWN);
    bool DLEFT = GET_BUTTON(gamepad, SDL_CONTROLLER_BUTTON_DPAD_LEFT);
    bool DRIGHT = GET_BUTTON(gamepad, SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
    bool L = GET_BUTTON(gamepad, SDL_CONTROLLER_BUTTON_LEFTSHOULDER);
    bool R = GET_BUTTON(gamepad, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);
    bool CUP = GET_AXIS(gamepad, SDL_CONTROLLER_AXIS_RIGHTY) <= -128;
    bool CDOWN = GET_AXIS(gamepad, SDL_CONTROLLER_AXIS_RIGHTY) >= 127;
    bool CLEFT = GET_AXIS(gamepad, SDL_CONTROLLER_AXIS_RIGHTX) <= -128;
    bool CRIGHT = GET_AXIS(gamepad, SDL_CONTROLLER_AXIS_RIGHTX) >= 127;

    controller.a = A;
    controller.b = B;
    controller.z = Z;
    controller.start = START;
    controller.dp_up = DUP;
    controller.dp_down = DDOWN;
    controller.dp_left = DLEFT;
    controller.dp_right = DRIGHT;
    controller.joy_reset = L && R && START;
    controller.l = L;
    controller.r = R;
    controller.c_up = CUP;
    controller.c_down = CDOWN;
    controller.c_left = CLEFT;
    controller.c_right = CRIGHT;

    xaxis = (s8) std::clamp((GET_AXIS(gamepad, SDL_CONTROLLER_AXIS_LEFTX) >> 8), -86, 86);
    yaxis = (s8) std::clamp(-(GET_AXIS(gamepad, SDL_CONTROLLER_AXIS_LEFTY) >> 8), -86, 86);

    controller.joy_x = xaxis;
    controller.joy_y = yaxis;

    if (controller.joy_reset) {
      controller.start = false;
      controller.joy_x = 0;
      controller.joy_y = 0;
    }
  } else {
    controller.a = state[SDL_SCANCODE_X];
    controller.b = state[SDL_SCANCODE_C];
    controller.z = state[SDL_SCANCODE_Z];
    controller.start = state[SDL_SCANCODE_RETURN];
    controller.dp_up = state[SDL_SCANCODE_KP_8];
    controller.dp_down = state[SDL_SCANCODE_KP_5];
    controller.dp_left = state[SDL_SCANCODE_KP_4];
    controller.dp_right = state[SDL_SCANCODE_KP_6];
    controller.joy_reset = state[SDL_SCANCODE_RETURN] && state[SDL_SCANCODE_A] && state[SDL_SCANCODE_S];
    controller.l = state[SDL_SCANCODE_A];
    controller.r = state[SDL_SCANCODE_S];
    controller.c_up = state[SDL_SCANCODE_I];
    controller.c_down = state[SDL_SCANCODE_J];
    controller.c_left = state[SDL_SCANCODE_K];
    controller.c_right = state[SDL_SCANCODE_L];

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

    controller.joy_x = xaxis;
    controller.joy_y = yaxis;

    if (controller.joy_reset) {
      controller.start = false;
      controller.joy_x = 0;
      controller.joy_y = 0;
    }
  }
}

void PIF::DoPIFHLE(Mem& mem, Registers& regs, CartInfo cartInfo) {
  u32 cicType = cartInfo.cicType;
  bool pal = cartInfo.isPAL;
  mem.Write32(regs, PIF_RAM_REGION_START + 0x24, cicSeeds[cicType]);

  switch(cicType) {
    case UNKNOWN_CIC_TYPE:
      Util::warn("Unknown CIC type!\n");
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
  regs.SetPC32(0xA4000040);
}

void PIF::ExecutePIF(Mem& mem, Registers& regs, CartInfo cartInfo) {
  u32 cicType = cartInfo.cicType;
  bool pal = cartInfo.isPAL;
  mem.Write32(regs, PIF_RAM_REGION_START + 0x24, cicSeeds[cicType]);
  switch(cicType) {
    case UNKNOWN_CIC_TYPE:
      Util::warn("Unknown CIC type!\n");
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

  DoPIFHLE(mem, regs, cartInfo);
}
}