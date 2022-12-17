#include <Core.hpp>
#include <ParallelRDPWrapper.hpp>
#include <Window.hpp>
#include <algorithm>
#include "m64.hpp"
#include <Scheduler.hpp>

namespace n64 {
Core::Core() {
  Stop();
}

void Core::Stop() {
  CpuReset();
  mem.Reset();
  pause = true;
  romLoaded = false;
}

CartInfo Core::LoadROM(const std::string& rom_) {
  rom = rom_;
  CpuReset();
  mem.Reset();
  pause = false;
  romLoaded = true;
  
  CartInfo cartInfo = mem.LoadROM(rom);
  DoPIFHLE(mem, CpuGetRegs(), cartInfo);

  return cartInfo;
}

void Core::Run(Window& window, float volumeL, float volumeR) {
  MMIO& mmio = mem.mmio;
  Controller& controller = mmio.si.controller;
  Registers& regs = CpuGetRegs();

  for(int field = 0; field < mmio.vi.numFields; field++) {
    int frameCycles = 0;
    if(!pause && romLoaded) {
      for (int i = 0; i < mmio.vi.numHalflines; i++) {
        mmio.vi.current = (i << 1) + field;

        if ((mmio.vi.current & 0x3FE) == mmio.vi.intr) {
          InterruptRaise(mmio.mi, regs, Interrupt::VI);
        }

        for(;cycles <= mmio.vi.cyclesPerHalfline; cycles++, frameCycles++) {
          CpuStep(mem);
          if(!mmio.rsp.spStatus.halt) {
            regs.steps++;
            if(regs.steps > 2) {
              mmio.rsp.steps += 2;
              regs.steps -= 3;
            }

            while(mmio.rsp.steps > 0) {
              mmio.rsp.steps--;
              mmio.rsp.Step(regs, mem);
            }
          }

          mmio.ai.Step(mem, regs, 1, volumeL, volumeR);
          scheduler.tick(1, mem, regs);
        }

        cycles -= mmio.vi.cyclesPerHalfline;
      }

      if ((mmio.vi.current & 0x3FE) == mmio.vi.intr) {
        InterruptRaise(mmio.mi, regs, Interrupt::VI);
      }

      UpdateScreenParallelRdp(*this, window, GetVI());

      int missedCycles = N64_CYCLES_PER_FRAME - frameCycles;
      mmio.ai.Step(mem, CpuGetRegs(), missedCycles, volumeL, volumeR);
    } else if(pause && romLoaded) {
      UpdateScreenParallelRdp(*this, window, GetVI());
    } else if(pause && !romLoaded) {
      UpdateScreenParallelRdpNoGame(*this, window);
    }
  }
}

#define GET_BUTTON(gamepad, i) SDL_GameControllerGetButton(gamepad, i)
#define GET_AXIS(gamepad, axis) SDL_GameControllerGetAxis(gamepad, axis)

void Core::UpdateController(const u8* state) {
  Controller &controller = mem.mmio.si.controller;
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
}
