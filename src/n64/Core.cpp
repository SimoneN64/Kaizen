#include <Core.hpp>
#include <ParallelRDPWrapper.hpp>
#include <Window.hpp>
#include <algorithm>
#include <util.hpp>
#include <debugger.hpp>

namespace n64 {
Core::Core() {
  Stop();
  DebuggerInit();
}

void Core::Stop() {
  cpu.Reset();
  mem.Reset();
  pause = true;
  romLoaded = false;
  rom.clear();
  DebuggerCleanup();
}

void Core::Reset() {
  cpu.Reset();
  mem.Reset();
  pause = true;
  romLoaded = false;
  DebuggerCleanup();
  DebuggerInit();
  if(!rom.empty()) {
    LoadROM(rom);
  }
}

void Core::LoadROM(const std::string& rom_) {
  rom = rom_;
  mem.LoadROM(rom);
  pause = false;
  romLoaded = true;
}

void Core::Step() {
  MMIO& mmio = mem.mmio;
  cpu.Step(mem);
  mmio.rsp.Step(mmio.mi, cpu.regs, mmio.rdp);
}

void Core::Run(Window& window) {
  MMIO& mmio = mem.mmio;
  int cycles = 0;
  for(int field = 0; field < mmio.vi.numFields; field++) {
    if(!pause && romLoaded) {
      for (int i = 0; i < mmio.vi.numHalflines; i++) {
        if ((mmio.vi.current & 0x3FE) == mmio.vi.intr) {
          InterruptRaise(mmio.mi, cpu.regs, Interrupt::VI);
        }

        for(;cycles <= mmio.vi.cyclesPerHalfline; cycles++) {
          #ifndef NDEBUG
          if (debuggerState.enabled && CheckBreakpoint(debuggerState, cpu.regs.pc)) {
            DebuggerBreakpointHit();
          }

          while (debuggerState.broken) {
            SDL_Delay(1000);
            DebuggerTick();
          }
          #endif
          cpu.Step(mem);
          mmio.rsp.Step(mmio.mi, cpu.regs, mmio.rdp);
          mmio.rsp.Step(mmio.mi, cpu.regs, mmio.rdp);
          mmio.rsp.Step(mmio.mi, cpu.regs, mmio.rdp);
          mmio.rsp.Step(mmio.mi, cpu.regs, mmio.rdp);
        }

        cycles -= mmio.vi.cyclesPerHalfline;
      }

      if ((mmio.vi.current & 0x3FE) == mmio.vi.intr) {
        InterruptRaise(mmio.mi, cpu.regs, Interrupt::VI);
      }

      UpdateScreenParallelRdp(*this, window, GetVI());
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
  if(gamepadConnected) {
    controller.b1 = 0;
    controller.b2 = 0;
    controller.b3 = 0;
    controller.b4 = 0;

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
    controller.b1 = 0;
    controller.b2 = 0;
    controller.b3 = 0;
    controller.b4 = 0;

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

    s8 xaxis = state[SDL_SCANCODE_LEFT] ? -128 : (state[SDL_SCANCODE_RIGHT] ? 127 : 0);
    s8 yaxis = state[SDL_SCANCODE_DOWN] ? -128 : (state[SDL_SCANCODE_UP] ? 127 : 0);

    controller.b3 = xaxis;
    controller.b4 = yaxis;

    if((controller.b2 >> 7) & 1) {
      controller.b1 &= ~0x10;
      controller.b3 = 0;
      controller.b4 = 0;
    }
  }
}

void Core::DebuggerInit() {
  gdbstub_config_t config;
  memset(&config, 0, sizeof(gdbstub_config_t));
  config.port                  = GDB_CPU_PORT;
  config.user_data             = this;
  config.start                 = (gdbstub_start_t) DebugStart;
  config.stop                  = (gdbstub_stop_t) DebugStop;
  config.step                  = (gdbstub_step_t) DebugStep;
  config.set_breakpoint        = (gdbstub_set_breakpoint_t) DebugSetBreakpoint;
  config.clear_breakpoint      = (gdbstub_clear_breakpoint_t) DebugClearBreakpoint;
  config.get_memory            = (gdbstub_get_memory_t) DebugGetMemory;
  config.get_register_value    = (gdbstub_get_register_value_t) DebugGetRegisterValue;
  config.get_general_registers = (gdbstub_get_general_registers_t) DebugGetGeneralRegisters;

  config.target_config = target_xml;
  config.target_config_length = strlen(target_xml);

  printf("Sizeof target: %zu\n", config.target_config_length);

  config.memory_map = memory_map;
  config.memory_map_length = strlen(memory_map);

  printf("Sizeof memory map: %zu\n", config.memory_map_length);

  debuggerState.gdb = gdbstub_init(config);
  if (!debuggerState.gdb) {
    util::panic("Failed to initialize GDB stub!");
  }
}

void Core::DebuggerTick() const {
  gdbstub_tick(debuggerState.gdb);
}

void Core::DebuggerBreakpointHit() {
  debuggerState.broken = true;
  gdbstub_breakpoint_hit(debuggerState.gdb);
}

void Core::DebuggerCleanup() const {
  if (debuggerState.enabled) {
    gdbstub_term(debuggerState.gdb);
  }
}
}
