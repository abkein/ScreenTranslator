#include "appsettings.h"

#include <QSettings>

namespace v4
{
AppSettings AppSettings::load()
{
  QSettings store;
  AppSettings settings;
  settings.setupComplete = store.value("setup/complete", false).toBool();
  settings.captureShortcut = QKeySequence::fromString(
      store.value("capture/shortcut", "Ctrl+Alt+T").toString(),
      QKeySequence::PortableText);
  settings.tessdataOverride = store.value("ocr/tessdataOverride").toString();
  settings.sourceLanguage = store.value("ocr/sourceLanguage").toString();
  settings.translationEnabled =
      store.value("translation/enabled", true).toBool();
  settings.targetLanguage =
      store.value("translation/targetLanguage").toString();
  settings.providers = store.value("translation/providers").toStringList();
  settings.translationTimeoutSeconds =
      store.value("translation/timeoutSeconds", 15).toInt();
  return settings;
}

void AppSettings::save() const
{
  QSettings store;
  store.setValue("schemaVersion", 1);
  store.setValue("setup/complete", setupComplete);
  store.setValue("capture/shortcut",
                 captureShortcut.toString(QKeySequence::PortableText));
  store.setValue("ocr/tessdataOverride", tessdataOverride);
  store.setValue("ocr/sourceLanguage", sourceLanguage);
  store.setValue("translation/enabled", translationEnabled);
  store.setValue("translation/targetLanguage", targetLanguage);
  store.setValue("translation/providers", providers);
  store.setValue("translation/timeoutSeconds", translationTimeoutSeconds);
  store.sync();
}
}  // namespace v4
