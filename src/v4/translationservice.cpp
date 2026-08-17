#include "translationservice.h"

#include "languagecodes.h"
#include "providerregistry.h"

#include <QFile>
#include <QTimer>
#include <QWebChannel>
#include <QWebEngineCertificateError>
#include <QWebEnginePage>
#include <QWebEngineProfile>
#include <QWebEngineScript>
#include <QWebEngineScriptCollection>
#include <QWebEngineSettings>

#include <algorithm>
#include <chrono>

namespace v4
{
namespace
{
QWebEngineScript script(const QString& name, const QString& source,
                        QWebEngineScript::InjectionPoint point)
{
  QWebEngineScript result;
  result.setName(name);
  result.setSourceCode(source);
  result.setWorldId(QWebEngineScript::UserWorld);
  result.setInjectionPoint(point);
  result.setRunsOnSubFrames(false);
  return result;
}

QString webChannelSource()
{
  QFile file(":/qtwebchannel/qwebchannel.js");
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    return {};
  return QString::fromUtf8(file.readAll()) + QStringLiteral(R"JS(
new QWebChannel(qt.webChannelTransport, function(channel) {
  window.proxy = channel.objects.proxy;
  if (typeof init === "function") init();
});
)JS");
}
}  // namespace

TranslationService::TranslationService(const ProviderRegistry& registry,
                                       QObject* parent)
  : QObject(parent)
  , registry_(registry)
  , timeout_(new QTimer(this))
{
  qRegisterMetaType<TranslationResult>();
  timeout_->setSingleShot(true);
  connect(timeout_, &QTimer::timeout, this,
          [this] { providerFailed(tr("timed out")); });
}

TranslationService::~TranslationService()
{
  resetPage();
}

void TranslationService::translate(const QString& text,
                                   const QString& sourceLanguage,
                                   const QString& targetLanguage,
                                   const QStringList& providers,
                                   int timeoutSeconds)
{
  if (busy_) {
    emit finished({{}, {}, tr("Translation is already in progress")});
    return;
  }

  text_ = text;
  source_ = LanguageCodes::iso639_1(sourceLanguage);
  target_ = LanguageCodes::iso639_1(targetLanguage);
  remainingProviders_ = providers;
  errors_.clear();
  timeoutSeconds_ = std::clamp(timeoutSeconds, 3, 120);
  busy_ = true;

  if (text_.trimmed().isEmpty()) {
    busy_ = false;
    emit finished({{}, {}, tr("There is no recognized text to translate")});
    return;
  }
  if (source_.isEmpty() || target_.isEmpty()) {
    busy_ = false;
    emit finished({{}, {}, tr("The selected language cannot be translated")});
    return;
  }
  tryNextProvider();
}

bool TranslationService::isBusy() const
{
  return busy_;
}

void TranslationService::tryNextProvider()
{
  resetPage();
  while (!remainingProviders_.isEmpty()) {
    currentProvider_ = remainingProviders_.takeFirst();
    const auto* provider = registry_.find(currentProvider_);
    if (!provider) {
      errors_ << tr("%1: provider not found").arg(currentProvider_);
      continue;
    }

    auto* profile = new QWebEngineProfile;
    page_ = new QWebEnginePage(profile, this);
    profile->setParent(page_);
    page_->settings()->setAttribute(QWebEngineSettings::AutoLoadImages, false);

    const auto channelSource = webChannelSource();
    if (channelSource.isEmpty()) {
      providerFailed(tr("Qt WebChannel script is unavailable"));
      return;
    }
    profile->scripts()->insert(script(currentProvider_, provider->script,
                                      QWebEngineScript::DocumentCreation));
    profile->scripts()->insert(script("qwebchannel.js", channelSource,
                                      QWebEngineScript::DocumentReady));

    bridge_ = new ProviderBridge(page_);
    channel_ = new QWebChannel(page_);
    channel_->registerObject("proxy", bridge_);
    page_->setWebChannel(channel_, QWebEngineScript::UserWorld);

    connect(bridge_, &ProviderBridge::succeeded, this,
            &TranslationService::providerSucceeded);
    connect(bridge_, &ProviderBridge::failed, this,
            &TranslationService::providerFailed);
    connect(
        page_, &QWebEnginePage::certificateError, this,
        [](QWebEngineCertificateError error) { error.rejectCertificate(); });
    connect(page_, &QWebEnginePage::renderProcessTerminated, this,
            [this] { providerFailed(tr("web process terminated")); });
    connect(page_, &QWebEnginePage::loadFinished, this, [this](bool ok) {
      if (!busy_ || pageReady_)
        return;
      if (!ok && page_->url() == QUrl("about:blank")) {
        providerFailed(tr("provider page initialization failed"));
        return;
      }
      pageReady_ = true;
      startCurrentProvider();
    });
    page_->setUrl(QUrl("about:blank"));
    return;
  }

  busy_ = false;
  currentProvider_.clear();
  emit finished(
      {{},
       {},
       tr("All translation providers failed:\n%1").arg(errors_.join('\n'))});
}

void TranslationService::startCurrentProvider()
{
  if (!busy_ || !pageReady_ || !bridge_)
    return;
  timeout_->start(std::chrono::seconds(timeoutSeconds_));
  emit bridge_->translate(text_, source_, target_);
}

void TranslationService::providerSucceeded(const QString& text)
{
  if (!busy_)
    return;
  if (text.trimmed().isEmpty()) {
    providerFailed(tr("provider returned an empty translation"));
    return;
  }
  timeout_->stop();
  const auto provider = currentProvider_;
  busy_ = false;
  resetPage();
  emit finished({text.trimmed(), provider, {}});
}

void TranslationService::providerFailed(const QString& error)
{
  if (!busy_)
    return;
  timeout_->stop();
  errors_ << tr("%1: %2").arg(currentProvider_, error);
  tryNextProvider();
}

void TranslationService::resetPage()
{
  timeout_->stop();
  pageReady_ = false;
  if (bridge_)
    disconnect(bridge_, nullptr, this, nullptr);
  bridge_ = nullptr;
  channel_ = nullptr;
  if (page_) {
    page_->deleteLater();
    page_ = nullptr;
  }
}
}  // namespace v4
