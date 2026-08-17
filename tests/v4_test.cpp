#include "appsettings.h"
#include "languagecodes.h"
#include "ocrservice.h"
#include "providerregistry.h"

#include <QFile>
#include <QSettings>
#include <QTemporaryDir>
#include <QtTest>

class V4Test : public QObject
{
  Q_OBJECT

private slots:
  void languageCodesRoundTrip();
  void discoversInstalledTesseractLanguages();
  void loadsBundledAndUserProviders();
  void settingsRoundTrip();
};

void V4Test::languageCodesRoundTrip()
{
  QCOMPARE(LanguageCodes::idForTesseract("eng"), QString("eng"));
  QCOMPARE(LanguageCodes::tesseract("eng"), QString("eng"));
  QCOMPARE(LanguageCodes::iso639_1("eng"), QString("en"));
  QCOMPARE(LanguageCodes::idForTesseract("custom"), QString("custom"));
}

void V4Test::discoversInstalledTesseractLanguages()
{
  QTemporaryDir directory;
  QVERIFY(directory.isValid());
  for (const auto& name :
       {"eng.traineddata", "deu.traineddata", "osd.traineddata"}) {
    QFile file(directory.filePath(name));
    QVERIFY(file.open(QIODevice::WriteOnly));
  }

  const auto catalog = v4::OcrService::discover(directory.path());
  QCOMPARE(catalog.path, directory.path());
  QCOMPARE(catalog.languageIds, QStringList({"deu", "eng"}));
}

void V4Test::loadsBundledAndUserProviders()
{
  QTemporaryDir directory;
  QVERIFY(directory.isValid());
  for (const auto& name : {"google_api.js", "custom.js"}) {
    QFile file(directory.filePath(name));
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
    file.write("function init() {}\n");
  }

  const v4::ProviderRegistry registry(directory.path());
  QCOMPARE(registry.providers().size(), 8);
  QVERIFY(registry.find("google_api.js"));
  QVERIFY(registry.find("google_api.js")->builtIn);
  QVERIFY(registry.find("custom.js"));
  QVERIFY(!registry.find("custom.js")->builtIn);
}

void V4Test::settingsRoundTrip()
{
  QTemporaryDir directory;
  QVERIFY(directory.isValid());
  QSettings::setDefaultFormat(QSettings::IniFormat);
  QSettings::setPath(QSettings::IniFormat, QSettings::UserScope,
                     directory.path());
  QCoreApplication::setOrganizationName("ScreenTranslatorTest");
  QCoreApplication::setApplicationName("ScreenTranslatorTest");
  QSettings().clear();

  v4::AppSettings expected;
  expected.setupComplete = true;
  expected.captureShortcut = QKeySequence("Meta+Shift+T");
  expected.tessdataOverride = "/tmp/tessdata";
  expected.sourceLanguage = "eng";
  expected.translationEnabled = true;
  expected.targetLanguage = "deu";
  expected.providers = {"google_api.js", "bing.js"};
  expected.translationTimeoutSeconds = 23;
  expected.save();

  const auto actual = v4::AppSettings::load();
  QCOMPARE(actual.setupComplete, expected.setupComplete);
  QCOMPARE(actual.captureShortcut, expected.captureShortcut);
  QCOMPARE(actual.tessdataOverride, expected.tessdataOverride);
  QCOMPARE(actual.sourceLanguage, expected.sourceLanguage);
  QCOMPARE(actual.translationEnabled, expected.translationEnabled);
  QCOMPARE(actual.targetLanguage, expected.targetLanguage);
  QCOMPARE(actual.providers, expected.providers);
  QCOMPARE(actual.translationTimeoutSeconds,
           expected.translationTimeoutSeconds);
}

QTEST_APPLESS_MAIN(V4Test)

#include "v4_test.moc"
