#pragma once

#include <QObject>
#include <QStringList>

class QTimer;
class QWebChannel;
class QWebEngineCertificateError;
class QWebEnginePage;

namespace v4
{
class ProviderRegistry;

struct TranslationResult {
  QString text;
  QString provider;
  QString error;
};

class ProviderBridge : public QObject
{
  Q_OBJECT
public:
  using QObject::QObject;

signals:
  void translate(const QString& text, const QString& source,
                 const QString& target);
  void succeeded(const QString& text);
  void failed(const QString& error);

public slots:
  void setTranslated(const QString& text) { emit succeeded(text); }
  void setFailed(const QString& error) { emit failed(error); }
};

class TranslationService : public QObject
{
  Q_OBJECT
public:
  explicit TranslationService(const ProviderRegistry& registry,
                              QObject* parent = nullptr);
  ~TranslationService() override;

  void translate(const QString& text, const QString& sourceLanguage,
                 const QString& targetLanguage, const QStringList& providers,
                 int timeoutSeconds);
  bool isBusy() const;

signals:
  void finished(const v4::TranslationResult& result);

private:
  void tryNextProvider();
  void startCurrentProvider();
  void providerSucceeded(const QString& text);
  void providerFailed(const QString& error);
  void resetPage();

  const ProviderRegistry& registry_;
  QWebEnginePage* page_{nullptr};
  ProviderBridge* bridge_{nullptr};
  QWebChannel* channel_{nullptr};
  QTimer* timeout_{nullptr};
  QString text_;
  QString source_;
  QString target_;
  QStringList remainingProviders_;
  QStringList errors_;
  QString currentProvider_;
  int timeoutSeconds_{15};
  bool pageReady_{false};
  bool busy_{false};
};
}  // namespace v4

Q_DECLARE_METATYPE(v4::TranslationResult)
