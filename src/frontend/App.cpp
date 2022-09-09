#include <App.hpp>
#include <parallel-rdp/ParallelRDPWrapper.hpp>
#include <nfd.hpp>

void App::Run() {
  // Main loop
  const u8* state = SDL_GetKeyboardState(nullptr);
  while (!core.done) {
    core.Run(window);
    core.UpdateController(state);

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      ImGui_ImplSDL2_ProcessEvent(&event);
      switch(event.type) {
        case SDL_QUIT: core.done = true; break;
        case SDL_WINDOWEVENT:
          core.done = window.gotClosed(event);
          break;
        case SDL_CONTROLLERDEVICEADDED: {
          const int index = event.cdevice.which;
          core.gamepad = SDL_GameControllerOpen(index);
          core.gamepadConnected = true;
        } break;
        case SDL_CONTROLLERDEVICEREMOVED:
          SDL_GameControllerClose(core.gamepad);
          core.gamepadConnected = false;
          break;
        case SDL_KEYDOWN:
          switch(event.key.keysym.sym) {
            case SDLK_o: {
              nfdchar_t* outpath;
              const nfdu8filteritem_t filter {"Nintendo 64 roms", "n64,z64,v64,N64,Z64,V64"};
              nfdresult_t result = NFD_OpenDialog(&outpath, &filter, 1, "/run/media/simuuz/HDD/n64_roms/tests");
              if(result == NFD_OKAY) {
                core.LoadROM(outpath);
                NFD_FreePath(outpath);
              }
            } break;
          } break;
      }
    }
  }
}
