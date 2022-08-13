#define SDL_MAIN_HANDLED
#include <frontend/App.hpp>

int main(int argc, char* argv[]) {
  App* app = new App;
  app->Run();
  return 0;
}