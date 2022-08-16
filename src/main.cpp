#define SDL_MAIN_HANDLED
#include <frontend/App.hpp>

int main(int argc, char* argv[]) {
  App app;
  if(argc > 1) {
    app.LoadROM(argv[1]);
  }
  app.Run();
  return 0;
}