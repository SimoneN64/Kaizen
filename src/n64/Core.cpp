#include <Core.hpp>
#include <ParallelRDPWrapper.hpp>
#include <Window.hpp>
#include <algorithm>
#include <util.hpp>

namespace n64 {
Core::Core() {
  Stop();
}

void Core::Stop() {
  cpu.Reset();
  mem.Reset();
  pause = true;
  romLoaded = false;
}

CartInfo Core::LoadROM(const std::string& rom_) {
  rom = rom_;
  cpu.Reset();
  mem.Reset();
  pause = false;
  romLoaded = true;
  
  CartInfo cartInfo = mem.LoadROM(rom);
  DoPIFHLE(mem, cpu.regs, cartInfo);

  return cartInfo;
}

void Core::Run(Window& window, float volumeL, float volumeR) {
  MMIO& mmio = mem.mmio;
  Controller& controller = mmio.si.controller;
  for(int field = 0; field < mmio.vi.numFields; field++) {
    int frameCycles = 0;
    if(!pause && romLoaded) {
      for (int i = 0; i < mmio.vi.numHalflines; i++) {
        mmio.vi.current = (i << 1) + field;

        if ((mmio.vi.current & 0x3FE) == mmio.vi.intr) {
          InterruptRaise(mmio.mi, cpu.regs, Interrupt::VI);
        }

        for(;cycles <= mmio.vi.cyclesPerHalfline; cycles++, frameCycles++) {
          cpu.Step(mem);
          mmio.rsp.Step(cpu.regs, mem);

          mmio.ai.Step(mem, cpu.regs, 1, volumeL, volumeR);
        }

        cycles -= mmio.vi.cyclesPerHalfline;
      }

      if ((mmio.vi.current & 0x3FE) == mmio.vi.intr) {
        InterruptRaise(mmio.mi, cpu.regs, Interrupt::VI);
      }

      UpdateScreenParallelRdp(*this, window, GetVI());

      int missedCycles = N64_CYCLES_PER_FRAME - frameCycles;
      mmio.ai.Step(mem, cpu.regs, missedCycles, volumeL, volumeR);
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
  Controller& controller = mem.mmio.si.controller;
  controller.raw = 0;
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
    bool CUP = GET_AXIS(gamepad, SDL_CONTROLLER_AXIS_RIGHTY) == 32767;
    bool CDOWN = GET_AXIS(gamepad, SDL_CONTROLLER_AXIS_RIGHTY) == -32768;
    bool CLEFT = GET_AXIS(gamepad, SDL_CONTROLLER_AXIS_RIGHTX) == -32768;
    bool CRIGHT = GET_AXIS(gamepad, SDL_CONTROLLER_AXIS_RIGHTX) == 32767;

    controller.b1 = (A << 7) | (B << 6) | (Z << 5) | (START << 4) |
                     (DUP << 3) | (DDOWN << 2) | (DLEFT << 1) | DRIGHT;

    controller.b2 = ((START && L && R) << 7) | (0 << 6) | (L << 5) | (R << 4) |
                     (CUP << 3) | (CDOWN << 2) | (CLEFT << 1) | CRIGHT;

    s8 xaxis = (s8)std::clamp((GET_AXIS(gamepad, SDL_CONTROLLER_AXIS_LEFTX) >> 8), -127, 127);
    s8 yaxis = (s8)std::clamp(-(GET_AXIS(gamepad, SDL_CONTROLLER_AXIS_LEFTY) >> 8), -127, 127);

    controller.b3 = xaxis;
    controller.b4 = yaxis;

    if((controller.b2 >> 7) & 1) {
      controller.b1 &= ~0x10;
      controller.b3 = 0;
      controller.b4 = 0;
    }
  } else {
    controller.b1 =
      (state[SDL_SCANCODE_X] << 7) |
      (state[SDL_SCANCODE_C] << 6) |
      (state[SDL_SCANCODE_Z] << 5) |
      (state[SDL_SCANCODE_RETURN] << 4) |
      (state[SDL_SCANCODE_KP_8] << 3) |
      (state[SDL_SCANCODE_KP_5] << 2) |
      (state[SDL_SCANCODE_KP_4] << 1) |
      (state[SDL_SCANCODE_KP_6]);
    controller.b2 =
      ((state[SDL_SCANCODE_RETURN] && state[SDL_SCANCODE_A] && state[SDL_SCANCODE_S]) << 7) |
      (0 << 6) |
      (state[SDL_SCANCODE_A] << 5) |
      (state[SDL_SCANCODE_S] << 4) |
      (state[SDL_SCANCODE_I] << 3) |
      (state[SDL_SCANCODE_J] << 2) |
      (state[SDL_SCANCODE_K] << 1) |
      (state[SDL_SCANCODE_L]);

    s8 xaxis = 0;
    if(state[SDL_SCANCODE_LEFT]) {
      xaxis = -127;
    } else if(state[SDL_SCANCODE_RIGHT]) {
      xaxis = 127;
    }

    s8 yaxis = 0;
    if(state[SDL_SCANCODE_DOWN]) {
      yaxis = -127;
    } else if(state[SDL_SCANCODE_UP]) {
      yaxis = 127;
    }

    controller.b3 = xaxis;
    controller.b4 = yaxis;

    if((controller.b2 >> 7) & 1) {
      controller.b1 &= ~0x10;
      controller.b3 = 0;
      controller.b4 = 0;
    }
  }
}
}
