#include <App.hpp>
#include <parallel-rdp/ParallelRDPWrapper.hpp>
#include <nfd.hpp>

void App::Run() {
  // Main loop
  bool done = false;
  while (!done) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      //ImGui_ImplSDL2_ProcessEvent(&event);
      if (event.type == SDL_QUIT)
        done = true;
      if (window.gotClosed(event))
        done = true;
      if(event.type == SDL_KEYDOWN) {
        switch(event.key.keysym.sym) {
          case SDLK_o: {
            nfdchar_t* outpath;
            const nfdu8filteritem_t filter {"Nintendo 64 roms", "n64,z64,v64,N64,Z64,V64"};
            nfdresult_t result = NFD_OpenDialog(&outpath, &filter, 1, nullptr);
            if(result == NFD_OKAY) {
              core.LoadROM(outpath);
              NFD_FreePath(outpath);
            }
          } break;
        }
      }

      core.PollInputs(event);
    }

    if(core.romLoaded)
      core.Run();

    if(core.romLoaded)
      UpdateScreenParallelRdp(window, core.GetVI());
    else
      UpdateScreenParallelRdpNoGame(window);

    SDL_Delay(1);
  }
}
