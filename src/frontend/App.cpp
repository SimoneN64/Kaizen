#include <App.hpp>
#include <parallel-rdp/ParallelRDPWrapper.hpp>

void App::Run() {
  // Main loop
  bool done = false;
  while (!done) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      ImGui_ImplSDL2_ProcessEvent(&event);
      if (event.type == SDL_QUIT)
        done = true;
      if (window.gotClosed(event))
        done = true;
    }

    if(core->initialized)
      core->Run();
    if(core->system == System::Nintendo64) {
      if(core->initialized) UpdateScreenParallelRdp(window, dynamic_cast<n64::Core*>(core.get())->GetVI());
      else UpdateScreenParallelRdpNoGame(window);
    }
  }
}