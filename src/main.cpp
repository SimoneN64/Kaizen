#include <App.hpp>
#include <MupenMovie.hpp>

int main(int argc, char** argv) {
  App app;

  if(argc > 1) {
    if(argc > 2) {
      LoadTAS(argv[2]);
    }
    app.LoadROM(argv[1]);
  }

  app.Run();

  return 0;
}