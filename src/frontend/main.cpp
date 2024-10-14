#include <KaizenQt.hpp>
#include <QApplication>
#include <QCommandLineParser>

int main(int argc, char **argv) {
  const QApplication app(argc, argv);
  QApplication::setStyle("fusion");
  QCoreApplication::setOrganizationName("kaizen");
  QCoreApplication::setApplicationName("Kaizen");
  QCommandLineParser parser;
  parser.setApplicationDescription(QCoreApplication::applicationName());
  parser.addHelpOption();
  parser.addOptions({{"rom", "Rom to launch from command-line", "path"}, {"movie", "Mupen Movie to replay", "path"}});
  parser.process(app);

  const KaizenQt kaizenQt;
  if (parser.isSet("rom")) {
    kaizenQt.LoadROM(parser.value("rom"));
    if (parser.isSet("movie")) {
      kaizenQt.LoadTAS(parser.value("movie"));
    }
  }

  return QApplication::exec();
}
