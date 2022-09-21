#include <frontend/App.hpp>

#ifdef _WIN32
#define main SDL_main
#endif

int main(int argc, char** argv) {
  App* app = new App;
  if(argc > 1) {
    app->LoadROM(argv[1]);
  }
  app->Run();
  return 0;
}