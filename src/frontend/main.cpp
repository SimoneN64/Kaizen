#include <QApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>

int main(int argc, char** argv) {
  QApplication app(argc, argv);
  app.setStyle("fusion");
  QCoreApplication::setOrganizationName("kaizen");
  QCoreApplication::setApplicationName("Kaizen");
  QCommandLineParser parser;
  parser.setApplicationDescription(QCoreApplication::applicationName());
  parser.addHelpOption();
  parser.addPositionalArgument("rom", "Rom to launch from command-line");
  parser.process(app);
  return app.exec();
}