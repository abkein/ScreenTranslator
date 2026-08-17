#pragma once

#include "appsettings.h"
#include "ocrservice.h"

#include <QDialog>

class QCheckBox;
class QComboBox;
class QDialogButtonBox;
class QKeySequenceEdit;
class QLabel;
class QLineEdit;
class QListWidget;
class QPushButton;
class QSpinBox;

namespace v4
{
class ProviderRegistry;

class SettingsDialog : public QDialog
{
  Q_OBJECT
public:
  SettingsDialog(const AppSettings& settings, const ProviderRegistry& providers,
                 bool portalManaged, bool portalCanConfigure,
                 const QString& shortcutStatus, QWidget* parent = nullptr);

  AppSettings settings() const;

signals:
  void configurePortalRequested();

private slots:
  void chooseTessdataDirectory();
  void refreshLanguages();
  void validateAndAccept();

private:
  void populateTargets();
  void populateProviders(const ProviderRegistry& providers,
                         const QStringList& enabled);

  AppSettings original_;
  QKeySequenceEdit* shortcut_{nullptr};
  QLabel* shortcutStatus_{nullptr};
  QPushButton* configurePortal_{nullptr};
  QLineEdit* tessdataPath_{nullptr};
  QLabel* detectedPath_{nullptr};
  QComboBox* sourceLanguage_{nullptr};
  QCheckBox* translationEnabled_{nullptr};
  QComboBox* targetLanguage_{nullptr};
  QListWidget* providers_{nullptr};
  QSpinBox* timeout_{nullptr};
  QDialogButtonBox* buttons_{nullptr};
  bool portalManaged_{false};
  OcrCatalog catalog_;
};
}  // namespace v4
