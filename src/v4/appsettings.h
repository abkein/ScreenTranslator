#pragma once

#include <QKeySequence>
#include <QString>
#include <QStringList>

namespace v4
{
struct AppSettings {
  bool setupComplete{false};
  QKeySequence captureShortcut{QStringLiteral("Ctrl+Alt+T")};
  QString tessdataOverride;
  QString sourceLanguage;
  bool translationEnabled{true};
  QString targetLanguage;
  QStringList providers;
  int translationTimeoutSeconds{15};

  static AppSettings load();
  void save() const;
};
}  // namespace v4
