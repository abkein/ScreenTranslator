#pragma once

#include <QKeySequence>
#include <QObject>
#include <QVariantMap>

class QAction;
class QDBusObjectPath;

namespace v4
{
class ShortcutService : public QObject
{
  Q_OBJECT
public:
  explicit ShortcutService(QObject* parent = nullptr);
  ~ShortcutService() override;

  void apply(const QKeySequence& preferredShortcut);
  bool isPortalManaged() const;
  bool canConfigurePortal() const;
  QString statusText() const;

public slots:
  void configurePortal();

signals:
  void activated();
  void statusChanged(const QString& status);

private slots:
  void handleRequestResponse(uint response, const QVariantMap& results);
  void handlePortalActivated(const QDBusObjectPath& session,
                             const QString& shortcutId, qulonglong timestamp,
                             const QVariantMap& options);

private:
  enum class PortalStage { None, CreatingSession, BindingShortcut };

  void startPortal();
  void bindPortalShortcut();
  void connectRequest(const QString& path, PortalStage stage);
  QString requestPath(const QString& token) const;
  void clearRequest();
  void setStatus(const QString& status);

  QAction* nativeAction_{nullptr};
  QString sessionPath_;
  QString activeRequestPath_;
  PortalStage portalStage_{PortalStage::None};
  uint portalVersion_{0};
  QString status_;
};
}  // namespace v4
