#include <App.hpp>
#include <nfd.hpp>

App::App() : window(core) {
  DiscordEventHandlers handlers{};
  Discord_Initialize("1049669178124148806", &handlers, 1, nullptr);
  Util::UpdateRPC(Util::Idling);
}

void App::Run() {
  SDL_EventState(SDL_DROPFILE, SDL_ENABLE);
  n64::SI& si = core.cpu->mem.mmio.si;

  while (!window.done) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      ImGui_ImplSDL2_ProcessEvent(&event);
      switch(event.type) {
        case SDL_QUIT:
          window.done = true;
          break;
        case SDL_WINDOWEVENT:
          window.handleEvents(event, core);
          break;
        case SDL_CONTROLLERDEVICEADDED: {
          const int index = event.cdevice.which;
          si.pif.gamepad = SDL_GameControllerOpen(index);
          si.pif.gamepadConnected = false;
          if (!si.pif.gamepad) {
            Util::warn("[WARN]: Could not initialize gamepad: {}", SDL_GetError());
          } else {
            si.pif.gamepadConnected = true;
          }
        } break;
        case SDL_CONTROLLERDEVICEREMOVED:
          SDL_GameControllerClose(si.pif.gamepad);
          si.pif.gamepadConnected = false;
          break;
        case SDL_KEYDOWN:
          switch (event.key.keysym.sym) {
            case SDLK_o: {
              OpenROMDialog(window, core);
            } break;
            case SDLK_F1: {
              if(core.romLoaded && event.key.keysym.mod == KMOD_SHIFT) {
                core.slot = 0;
              }
            } break;
            case SDLK_F2: {
              if(core.romLoaded && event.key.keysym.mod == KMOD_SHIFT) {
                core.slot = 1;
              }
            } break;
            case SDLK_F3: {
              if(core.romLoaded && event.key.keysym.mod == KMOD_SHIFT) {
                core.slot = 2;
              }
            } break;
            case SDLK_F4: {
              if(core.romLoaded && event.key.keysym.mod == KMOD_SHIFT) {
                core.slot = 3;
              }
            } break;
            case SDLK_F5: {
              if(core.romLoaded) {
                if(event.key.keysym.mod == KMOD_SHIFT) {
                  core.slot = 4;
                } else {
                  core.Deserialize();
                }
              }
            } break;
            case SDLK_F6: {
              if(core.romLoaded) {
                if(event.key.keysym.mod == KMOD_SHIFT) {
                  core.slot = 5;
                } else {
                  core.Serialize();
                }
              }
            } break;
            case SDLK_F7: {
              if(core.romLoaded && event.key.keysym.mod == KMOD_SHIFT) {
                core.slot = 6;
              }
            } break;
            case SDLK_F8: {
              if(core.romLoaded && event.key.keysym.mod == KMOD_SHIFT) {
                core.slot = 7;
              }
            } break;
            case SDLK_F9: {
              if(core.romLoaded && event.key.keysym.mod == KMOD_SHIFT) {
                core.slot = 8;
              }
            } break;
            case SDLK_F10: {
              if(core.romLoaded && event.key.keysym.mod == KMOD_SHIFT) {
                core.slot = 9;
              }
            } break;
          }
          break;
        case SDL_DROPFILE: {
          char *droppedDir = event.drop.file;
          if (droppedDir) {
            window.LoadROM(core, droppedDir);
            SDL_free(droppedDir);
          }
        } break;
      }
    }

    if(core.romLoaded) {
      if(!core.pause) {
        core.Run(window.settings.GetVolumeL(), window.settings.GetVolumeR());
      }
      if(core.render) {
        UpdateScreenParallelRdp(core, window, core.GetVI());
      }
    } else {
      if(core.render) {
        UpdateScreenParallelRdpNoGame(core, window);
      }
    }
  }
}