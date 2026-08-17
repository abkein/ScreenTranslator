#include "settingsdialog.h"

#include "languagecodes.h"
#include "providerregistry.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDesktopServices>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QKeySequenceEdit>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QUrl>
#include <QVBoxLayout>

#include <algorithm>

namespace v4
{
SettingsDialog::SettingsDialog(const AppSettings& settings,
                               const ProviderRegistry& providerRegistry,
                               bool portalManaged, bool portalCanConfigure,
                               const QString& shortcutStatus, QWidget* parent)
  : QDialog(parent)
  , original_(settings)
  , shortcut_(new QKeySequenceEdit(settings.captureShortcut, this))
  , shortcutStatus_(new QLabel(shortcutStatus, this))
  , configurePortal_(new QPushButton(tr("Configure in desktop…"), this))
  , tessdataPath_(new QLineEdit(settings.tessdataOverride, this))
  , detectedPath_(new QLabel(this))
  , sourceLanguage_(new QComboBox(this))
  , translationEnabled_(new QCheckBox(tr("Translate recognized text"), this))
  , targetLanguage_(new QComboBox(this))
  , providers_(new QListWidget(this))
  , timeout_(new QSpinBox(this))
  , buttons_(new QDialogButtonBox(
        QDialogButtonBox::Save | QDialogButtonBox::Cancel, this))
  , portalManaged_(portalManaged)
{
  setWindowTitle(settings.setupComplete ? tr("Screen Translator Settings")
                                        : tr("Set up Screen Translator"));
  resize(600, 640);

  auto* root = new QVBoxLayout(this);
  if (!settings.setupComplete) {
    auto* intro = new QLabel(
        tr("Choose an installed OCR language, a target language, and at least "
           "one provider. Capture remains disabled until setup is saved."),
        this);
    intro->setWordWrap(true);
    root->addWidget(intro);
  }

  auto* captureBox = new QGroupBox(tr("Capture"), this);
  auto* captureForm = new QFormLayout(captureBox);
  shortcut_->setEnabled(!portalManaged_);
  configurePortal_->setVisible(portalManaged_);
  configurePortal_->setEnabled(portalCanConfigure);
  captureForm->addRow(tr("Shortcut:"), shortcut_);
  captureForm->addRow({}, shortcutStatus_);
  captureForm->addRow({}, configurePortal_);
  root->addWidget(captureBox);

  auto* ocrBox = new QGroupBox(tr("OCR"), this);
  auto* ocrForm = new QFormLayout(ocrBox);
  auto* pathRow = new QHBoxLayout;
  pathRow->addWidget(tessdataPath_);
  auto* browse = new QPushButton(tr("Browse…"), this);
  auto* detect = new QPushButton(tr("Auto-detect"), this);
  pathRow->addWidget(browse);
  pathRow->addWidget(detect);
  ocrForm->addRow(tr("tessdata override:"), pathRow);
  detectedPath_->setWordWrap(true);
  ocrForm->addRow(tr("Using:"), detectedPath_);
  ocrForm->addRow(tr("Source language:"), sourceLanguage_);
  root->addWidget(ocrBox);

  auto* translationBox = new QGroupBox(tr("Translation"), this);
  auto* translationLayout = new QVBoxLayout(translationBox);
  translationEnabled_->setChecked(settings.translationEnabled);
  translationLayout->addWidget(translationEnabled_);
  auto* translationForm = new QFormLayout;
  translationForm->addRow(tr("Target language:"), targetLanguage_);
  timeout_->setRange(3, 120);
  timeout_->setSuffix(tr(" seconds"));
  timeout_->setValue(settings.translationTimeoutSeconds);
  translationForm->addRow(tr("Provider timeout:"), timeout_);
  translationLayout->addLayout(translationForm);
  providers_->setDragDropMode(QAbstractItemView::InternalMove);
  providers_->setDefaultDropAction(Qt::MoveAction);
  translationLayout->addWidget(
      new QLabel(tr("Providers (drag to reorder):"), this));
  translationLayout->addWidget(providers_);
  auto* userProviders =
      new QPushButton(tr("Open user provider directory: %1")
                          .arg(providerRegistry.userDirectory()),
                      this);
  userProviders->setToolTip(providerRegistry.userDirectory());
  translationLayout->addWidget(userProviders);
  root->addWidget(translationBox, 1);
  root->addWidget(buttons_);

  populateTargets();
  populateProviders(providerRegistry, settings.providers);
  refreshLanguages();

  connect(browse, &QPushButton::clicked, this,
          &SettingsDialog::chooseTessdataDirectory);
  connect(detect, &QPushButton::clicked, this, [this] {
    tessdataPath_->clear();
    refreshLanguages();
  });
  connect(tessdataPath_, &QLineEdit::editingFinished, this,
          &SettingsDialog::refreshLanguages);
  connect(configurePortal_, &QPushButton::clicked, this,
          &SettingsDialog::configurePortalRequested);
  const auto userProviderDirectory = providerRegistry.userDirectory();
  connect(userProviders, &QPushButton::clicked, this, [userProviderDirectory] {
    QDesktopServices::openUrl(QUrl::fromLocalFile(userProviderDirectory));
  });
  connect(translationEnabled_, &QCheckBox::toggled, translationBox,
          [this](bool enabled) {
            targetLanguage_->setEnabled(enabled);
            providers_->setEnabled(enabled);
            timeout_->setEnabled(enabled);
          });
  targetLanguage_->setEnabled(translationEnabled_->isChecked());
  providers_->setEnabled(translationEnabled_->isChecked());
  timeout_->setEnabled(translationEnabled_->isChecked());
  connect(buttons_, &QDialogButtonBox::accepted, this,
          &SettingsDialog::validateAndAccept);
  connect(buttons_, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

AppSettings SettingsDialog::settings() const
{
  auto result = original_;
  result.setupComplete = true;
  result.captureShortcut = shortcut_->keySequence();
  result.tessdataOverride = tessdataPath_->text().trimmed();
  result.sourceLanguage = sourceLanguage_->currentData().toString();
  result.translationEnabled = translationEnabled_->isChecked();
  result.targetLanguage = targetLanguage_->currentData().toString();
  result.translationTimeoutSeconds = timeout_->value();
  result.providers.clear();
  for (int row = 0; row < providers_->count(); ++row) {
    const auto* item = providers_->item(row);
    if (item->checkState() == Qt::Checked)
      result.providers << item->data(Qt::UserRole).toString();
  }
  return result;
}

void SettingsDialog::chooseTessdataDirectory()
{
  const auto path = QFileDialog::getExistingDirectory(
      this, tr("Choose Tesseract language-data directory"),
      tessdataPath_->text());
  if (path.isEmpty())
    return;
  tessdataPath_->setText(path);
  refreshLanguages();
}

void SettingsDialog::refreshLanguages()
{
  const auto selected = sourceLanguage_->currentIndex() >= 0
                            ? sourceLanguage_->currentData().toString()
                            : original_.sourceLanguage;
  catalog_ = OcrService::discover(tessdataPath_->text());
  detectedPath_->setText(catalog_.isValid()
                             ? catalog_.path
                             : tr("No Tesseract language data found"));
  sourceLanguage_->clear();
  for (const auto& id : catalog_.languageIds)
    sourceLanguage_->addItem(LanguageCodes::name(id), id);
  const auto index = sourceLanguage_->findData(selected);
  if (index >= 0)
    sourceLanguage_->setCurrentIndex(index);
}

void SettingsDialog::validateAndAccept()
{
  if (!catalog_.isValid() || sourceLanguage_->currentIndex() < 0) {
    QMessageBox::warning(this, tr("Incomplete setup"),
                         tr("Install Tesseract language data or choose a valid "
                            "tessdata directory."));
    return;
  }
  if (!portalManaged_ && shortcut_->keySequence().isEmpty()) {
    QMessageBox::warning(this, tr("Incomplete setup"),
                         tr("Choose a capture shortcut."));
    return;
  }
  if (translationEnabled_->isChecked()) {
    if (targetLanguage_->currentIndex() < 0) {
      QMessageBox::warning(this, tr("Incomplete setup"),
                           tr("Choose a target language."));
      return;
    }
    bool gotProvider = false;
    for (int row = 0; row < providers_->count(); ++row)
      gotProvider |= providers_->item(row)->checkState() == Qt::Checked;
    if (!gotProvider) {
      QMessageBox::warning(this, tr("Incomplete setup"),
                           tr("Enable at least one translation provider."));
      return;
    }
  }
  accept();
}

void SettingsDialog::populateTargets()
{
  QVector<QPair<QString, QString>> languages;
  for (const auto& id : LanguageCodes::allIds()) {
    if (!LanguageCodes::iso639_1(id).isEmpty())
      languages.push_back({LanguageCodes::name(id), id});
  }
  std::sort(languages.begin(), languages.end(),
            [](const auto& left, const auto& right) {
              return left.first.localeAwareCompare(right.first) < 0;
            });
  for (const auto& language : languages)
    targetLanguage_->addItem(language.first, language.second);
  const auto index = targetLanguage_->findData(original_.targetLanguage);
  if (index >= 0)
    targetLanguage_->setCurrentIndex(index);
  else
    targetLanguage_->setCurrentIndex(-1);
}

void SettingsDialog::populateProviders(const ProviderRegistry& registry,
                                       const QStringList& enabled)
{
  for (const auto& id : enabled) {
    const auto* provider = registry.find(id);
    if (!provider)
      continue;
    auto* item = new QListWidgetItem(provider->displayName, providers_);
    item->setData(Qt::UserRole, provider->id);
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable |
                   Qt::ItemIsDragEnabled);
    item->setCheckState(Qt::Checked);
  }
  for (const auto& provider : registry.providers()) {
    if (enabled.contains(provider.id))
      continue;
    auto* item = new QListWidgetItem(provider.displayName, providers_);
    item->setData(Qt::UserRole, provider.id);
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable |
                   Qt::ItemIsDragEnabled);
    item->setCheckState(Qt::Unchecked);
  }
}
}  // namespace v4
