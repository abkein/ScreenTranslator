#include "appcontroller.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QDir>
#include <QIcon>
#include <QLockFile>
#include <QStandardPaths>

#include <clocale>

int main(int argc, char* argv[])
{
  QApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
  QApplication app(argc, argv);
  QCoreApplication::setApplicationName("ScreenTranslator");
  QGuiApplication::setApplicationDisplayName("Screen Translator");
  QCoreApplication::setOrganizationName("OneMoreGres");
  QCoreApplication::setOrganizationDomain("github.com/OneMoreGres");
  QCoreApplication::setApplicationVersion(SCREEN_TRANSLATOR_VERSION);
  QGuiApplication::setDesktopFileName("io.github.OneMoreGres.ScreenTranslator");
  QApplication::setWindowIcon(QIcon(":/icons/app.png"));
  QApplication::setQuitOnLastWindowClosed(false);

  QCommandLineParser parser;
  parser.setApplicationDescription(
      QObject::tr("Capture, recognize, and translate text on screen"));
  parser.addHelpOption();
  parser.addVersionOption();
  parser.process(app);

  auto lockDirectory =
      QStandardPaths::writableLocation(QStandardPaths::RuntimeLocation);
  if (lockDirectory.isEmpty())
    lockDirectory =
        QStandardPaths::writableLocation(QStandardPaths::TempLocation);
  QDir().mkpath(lockDirectory);
  QLockFile lock(QDir(lockDirectory).filePath("screen-translator-v4.lock"));
  lock.setStaleLockTime(0);
  if (!lock.tryLock(0))
    return 1;

  // Tesseract expects locale-independent decimal parsing.
  std::setlocale(LC_NUMERIC, "C");

  v4::AppController controller;
  if (!controller.start())
    return 1;
  return app.exec();
}
