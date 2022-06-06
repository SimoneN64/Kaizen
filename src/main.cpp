#ifdef FRONTEND_QT
#include <qt/Frontend.hpp>
#elifdef FRONTEND_SDL
#include <sdl/Frontend.hpp>
#endif

#include <util.hpp>

int main(int argc, char* argv[]) {
#ifdef FRONTEND_SDL
  if(argc < 2) {
    natsukashii::util::panic("Usage: natsukashii [rom]");
  }
  natsukashii::frontend::App app(argv[1]);
#else
  natsukashii::frontend::App app;
#endif
  app.Run();
  return 0;
}
