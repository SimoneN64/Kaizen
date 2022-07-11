#include <App.hpp>

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
    window.Update(core);
  }
}