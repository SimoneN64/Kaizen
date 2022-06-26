#include <Frontend.hpp>
#include <QGuiApplication>

int main(int argc, char* argv[]) {
  QGuiApplication app(argc, argv);
  natsukashii::frontend::Window window;
  window.show();

  return app.exec();
}
