#include "appcontroller.h"

#include "capturebackend.h"
#include "settingsdialog.h"

#include <QAction>
#include <QApplication>
#include <QHash>
#include <QIcon>
#include <QImage>
#include <QMenu>
#include <QMessageBox>
#include <QSystemTrayIcon>
#include <QTimer>

namespace v4
{
namespace
{
QIcon trayIcon(const QString& state)
{
  static const QHash<QString, QIcon> icons{
      {"idle", QIcon(":/icons/app.png")},
      {"busy", QIcon(":/icons/st_busy.png")},
      {"success", QIcon(":/icons/st_success.png")},
      {"error", QIcon(":/icons/st_error.png")},
  };
  return icons.value(state);
}
}  // namespace

AppController::AppController(QObject* parent)
  : QObject(parent)
  , settings_(AppSettings::load())
  , captureBackend_(createCaptureBackend(this))
  , ocr_(this)
  , translation_(providers_, this)
  , shortcuts_(this)
  , tray_(new QSystemTrayIcon(this))
  , menu_(new QMenu)
{
  resultCard_.setWindowIcon(trayIcon("idle"));
  tray_->setIcon(trayIcon("idle"));
  tray_->setToolTip(tr("Screen Translator"));

  captureAction_ = menu_->addAction(tr("Capture area"));
  showLastAction_ = menu_->addAction(tr("Show last result"));
  menu_->addSeparator();
  settingsAction_ = menu_->addAction(tr("Settings…"));
  menu_->addSeparator();
  auto* quitAction = menu_->addAction(tr("Quit"));
  tray_->setContextMenu(menu_);

  connect(captureAction_, &QAction::triggered, this, &AppController::capture);
  connect(showLastAction_, &QAction::triggered, &resultCard_,
          &ResultCard::showLast);
  connect(settingsAction_, &QAction::triggered, this,
          &AppController::showSettings);
  connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);
  connect(
      tray_, &QSystemTrayIcon::activated, this,
      [this](QSystemTrayIcon::ActivationReason reason) {
        if (reason == QSystemTrayIcon::Trigger && showLastAction_->isEnabled())
          resultCard_.showLast();
      });

  connect(captureBackend_, &CaptureBackend::captured, this,
          &AppController::recognize);
  connect(captureBackend_, &CaptureBackend::canceled, this,
          [this] { setBusy(false); });
  connect(captureBackend_, &CaptureBackend::error, this,
          [this](const QString& message) { finishWithError(message); });
  connect(&ocr_, &OcrService::finished, this, &AppController::handleOcrResult);
  connect(&translation_, &TranslationService::finished, this,
          &AppController::handleTranslationResult);
  connect(&shortcuts_, &ShortcutService::activated, this,
          &AppController::capture);
  connect(&shortcuts_, &ShortcutService::statusChanged, this,
          [this](const QString& status) {
            tray_->setToolTip(tr("Screen Translator\n%1").arg(status));
          });
  connect(&resultCard_, &ResultCard::retryRequested, this,
          &AppController::retryTranslation);
}

AppController::~AppController()
{
  tray_->hide();
  tray_->setContextMenu(nullptr);
  delete menu_;
}

bool AppController::start()
{
  if (!QSystemTrayIcon::isSystemTrayAvailable()) {
    QMessageBox::critical(
        nullptr, tr("Screen Translator"),
        tr("No system tray is available. Screen Translator needs a system "
           "tray to run."));
    return false;
  }

  applySettings();
  tray_->show();

  if (!isConfigured()) {
    QTimer::singleShot(0, this, &AppController::showSettings);
  } else {
    tray_->showMessage(tr("Screen Translator is ready"),
                       tr("Use %1 or the tray menu to capture an area.")
                           .arg(shortcuts_.isPortalManaged()
                                    ? tr("your desktop shortcut")
                                    : settings_.captureShortcut.toString(
                                          QKeySequence::NativeText)),
                       QSystemTrayIcon::Information, 3500);
  }
  return true;
}

void AppController::capture()
{
  if (busy_)
    return;
  if (!isConfigured()) {
    notifyError(tr("Finish setup before capturing an area."));
    showSettings();
    return;
  }

  setBusy(true);
  captureBackend_->capture();
}

void AppController::showSettings()
{
  if (busy_) {
    notifyError(
        tr("Wait for the current capture to finish before changing "
           "settings."));
    return;
  }

  providers_.reload();
  SettingsDialog dialog(settings_, providers_, shortcuts_.isPortalManaged(),
                        shortcuts_.canConfigurePortal(),
                        shortcuts_.statusText());
  connect(&dialog, &SettingsDialog::configurePortalRequested, &shortcuts_,
          &ShortcutService::configurePortal);
  if (dialog.exec() != QDialog::Accepted)
    return;

  settings_ = dialog.settings();
  settings_.save();
  providers_.reload();
  applySettings();
  tray_->showMessage(tr("Settings saved"),
                     tr("Screen Translator is ready to capture text."),
                     QSystemTrayIcon::Information, 3000);
}

void AppController::recognize(const QImage& image)
{
  lastRecognized_.clear();
  showLastAction_->setEnabled(false);
  resultCard_.showRecognizing();
  ocr_.recognize(image, catalog_.path, settings_.sourceLanguage);
}

void AppController::handleOcrResult(const OcrResult& result)
{
  if (!result.error.isEmpty()) {
    finishWithError(result.error);
    return;
  }

  lastRecognized_ = result.text.trimmed();
  if (lastRecognized_.isEmpty()) {
    finishWithError(tr("No text was recognized in the selected area."));
    return;
  }

  showLastAction_->setEnabled(true);
  if (!settings_.translationEnabled) {
    setBusy(false);
    tray_->setIcon(trayIcon("success"));
    resultCard_.showRecognized(lastRecognized_, false);
    return;
  }

  resultCard_.showRecognized(lastRecognized_, true);
  translation_.translate(lastRecognized_, settings_.sourceLanguage,
                         settings_.targetLanguage, settings_.providers,
                         settings_.translationTimeoutSeconds);
}

void AppController::handleTranslationResult(const TranslationResult& result)
{
  setBusy(false);
  if (!result.error.isEmpty()) {
    finishWithError(result.error, lastRecognized_);
    return;
  }

  auto providerName = result.provider;
  if (const auto* provider = providers_.find(result.provider))
    providerName = provider->displayName;
  tray_->setIcon(trayIcon("success"));
  resultCard_.showTranslated(result.text, providerName);
}

void AppController::retryTranslation()
{
  if (busy_ || lastRecognized_.isEmpty())
    return;
  if (!settings_.translationEnabled) {
    notifyError(tr("Translation is disabled in Settings."));
    return;
  }

  setBusy(true);
  resultCard_.showRecognized(lastRecognized_, true);
  translation_.translate(lastRecognized_, settings_.sourceLanguage,
                         settings_.targetLanguage, settings_.providers,
                         settings_.translationTimeoutSeconds);
}

bool AppController::isConfigured() const
{
  if (!settings_.setupComplete || !catalog_.isValid() ||
      !catalog_.languageIds.contains(settings_.sourceLanguage))
    return false;
  if (!settings_.translationEnabled)
    return true;
  if (settings_.targetLanguage.isEmpty() || settings_.providers.isEmpty())
    return false;
  for (const auto& provider : settings_.providers) {
    if (providers_.find(provider))
      return true;
  }
  return false;
}

void AppController::applySettings()
{
  catalog_ = OcrService::discover(settings_.tessdataOverride);
  shortcuts_.apply(settings_.captureShortcut);
  captureAction_->setEnabled(!busy_ && isConfigured());
  showLastAction_->setEnabled(!lastRecognized_.isEmpty());
}

void AppController::setBusy(bool busy)
{
  busy_ = busy;
  captureAction_->setEnabled(!busy_ && isConfigured());
  settingsAction_->setEnabled(!busy_);
  if (busy_)
    tray_->setIcon(trayIcon("busy"));
  else if (tray_->icon().cacheKey() == trayIcon("busy").cacheKey())
    tray_->setIcon(trayIcon("idle"));
}

void AppController::finishWithError(const QString& message,
                                    const QString& recognized)
{
  setBusy(false);
  tray_->setIcon(trayIcon("error"));
  resultCard_.showError(message, recognized);
  notifyError(message);
}

void AppController::notifyError(const QString& message)
{
  tray_->showMessage(tr("Screen Translator error"), message,
                     QSystemTrayIcon::Warning, 5000);
}
}  // namespace v4
