#pragma once

#include "appsettings.h"
#include "ocrservice.h"
#include "providerregistry.h"
#include "resultcard.h"
#include "shortcutservice.h"
#include "translationservice.h"

#include <QObject>

class QAction;
class QImage;
class QMenu;
class QSystemTrayIcon;

namespace v4
{
class CaptureBackend;

class AppController : public QObject
{
  Q_OBJECT
public:
  explicit AppController(QObject* parent = nullptr);
  ~AppController() override;

  bool start();

private slots:
  void capture();
  void showSettings();
  void recognize(const QImage& image);
  void handleOcrResult(const OcrResult& result);
  void handleTranslationResult(const TranslationResult& result);
  void retryTranslation();

private:
  bool isConfigured() const;
  void applySettings();
  void setBusy(bool busy);
  void finishWithError(const QString& message, const QString& recognized = {});
  void notifyError(const QString& message);

  AppSettings settings_;
  ProviderRegistry providers_;
  OcrCatalog catalog_;
  CaptureBackend* captureBackend_{nullptr};
  OcrService ocr_;
  TranslationService translation_;
  ShortcutService shortcuts_;
  ResultCard resultCard_;
  QSystemTrayIcon* tray_{nullptr};
  QMenu* menu_{nullptr};
  QAction* captureAction_{nullptr};
  QAction* showLastAction_{nullptr};
  QAction* settingsAction_{nullptr};
  QString lastRecognized_;
  bool busy_{false};
};
}  // namespace v4
