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
    if(core.romLoaded) {
      if(!core.pause) {
        core.Run(window.settings.GetVolumeL(), window.settings.GetVolumeR());
      }
      UpdateScreenParallelRdp(core, window, core.GetVI());
    } else {
      UpdateScreenParallelRdpNoGame(core, window);
    }

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      ImGui_ImplSDL2_ProcessEvent(&event);
      switch(event.type) {
        case SDL_QUIT:
          window.done = true;
          break;
        case SDL_WINDOWEVENT:
          window.onClose(event);
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
              nfdchar_t *outpath;
              const nfdu8filteritem_t filter{"Nintendo 64 roms/archives", "n64,z64,v64,N64,Z64,V64,zip,tar,rar,7z"};
              nfdresult_t result = NFD_OpenDialog(&outpath, &filter, 1, nullptr);
              if (result == NFD_OKAY) {
                LoadROM(outpath);
                NFD_FreePath(outpath);
              }
            } break;
          }
          break;
        case SDL_DROPFILE: {
          char *droppedDir = event.drop.file;
          if (droppedDir) {
            LoadROM(droppedDir);
            free(droppedDir);
          }
        } break;
      }
    }
  }
}