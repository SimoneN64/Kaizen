#include <App.hpp>
#include <nfd.hpp>

void App::Run() {
  const u8* state = SDL_GetKeyboardState(nullptr);
  SDL_EventState(SDL_DROPFILE, SDL_ENABLE);

  while (!core.done) {
    core.Run(window, window.volumeL, window.volumeR);
    core.UpdateController(state);

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      ImGui_ImplSDL2_ProcessEvent(&event);
      switch(event.type) {
        case SDL_QUIT:
          core.done = true;
          break;
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
              nfdresult_t result = NFD_OpenDialog(&outpath, &filter, 1, nullptr);
              if(result == NFD_OKAY) {
                LoadROM(outpath);
                NFD_FreePath(outpath);
              }
            } break;
          } break;
          case SDL_DROPFILE: {
            char* droppedDir = event.drop.file;
            if(droppedDir) {
              LoadROM(droppedDir);
              free(droppedDir);
            }
          } break;
      }
    }
  }
}
