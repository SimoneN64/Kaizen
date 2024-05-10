#include <KaizenQt.hpp>
#include <QApplication>
#include <QCommandLineParser>

int main(int argc, char** argv) {
  QApplication app(argc, argv);
  app.setStyle("fusion");
  QCoreApplication::setOrganizationName("kaizen");
  QCoreApplication::setApplicationName("Kaizen");
  QCommandLineParser parser;
  parser.setApplicationDescription(QCoreApplication::applicationName());
  parser.addHelpOption();
  parser.addPositionalArgument("rom", "Rom to launch from command-line");
  parser.addPositionalArgument("m64", "Mupen Movie to replay");
  parser.process(app);
  
  KaizenQt kaizenQt;
  if (parser.positionalArguments().size() > 0) {
    kaizenQt.LoadROM(parser.positionalArguments().first());
    if (parser.positionalArguments().size() > 1) {
      kaizenQt.LoadTAS(parser.positionalArguments()[1]);
    }
  }

  return app.exec();
}