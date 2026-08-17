#include "shortcutservice.h"

#include "globalaction.h"

#include <QAction>
#include <QApplication>
#include <QDBusArgument>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMetaType>
#include <QDBusObjectPath>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QGuiApplication>
#include <QUuid>

namespace
{
constexpr auto portalService = "org.freedesktop.portal.Desktop";
constexpr auto portalPath = "/org/freedesktop/portal/desktop";
constexpr auto shortcutsInterface = "org.freedesktop.portal.GlobalShortcuts";
constexpr auto requestInterface = "org.freedesktop.portal.Request";
constexpr auto sessionInterface = "org.freedesktop.portal.Session";

struct PortalShortcut {
  QString id;
  QVariantMap options;
};
using PortalShortcuts = QList<PortalShortcut>;

QDBusArgument& operator<<(QDBusArgument& argument,
                          const PortalShortcut& shortcut)
{
  argument.beginStructure();
  argument << shortcut.id << shortcut.options;
  argument.endStructure();
  return argument;
}

const QDBusArgument& operator>>(const QDBusArgument& argument,
                                PortalShortcut& shortcut)
{
  argument.beginStructure();
  argument >> shortcut.id >> shortcut.options;
  argument.endStructure();
  return argument;
}
}  // namespace

Q_DECLARE_METATYPE(PortalShortcut)
Q_DECLARE_METATYPE(PortalShortcuts)

namespace v4
{
ShortcutService::ShortcutService(QObject* parent)
  : QObject(parent)
{
  status_ = tr("Shortcut is not configured");
}

ShortcutService::~ShortcutService()
{
  clearRequest();
  if (nativeAction_)
    service::GlobalAction::removeGlobal(nativeAction_);
  if (!sessionPath_.isEmpty()) {
    QDBusInterface session(portalService, sessionPath_, sessionInterface,
                           QDBusConnection::sessionBus());
    session.asyncCall("Close");
  }
}

void ShortcutService::apply(const QKeySequence& preferredShortcut)
{
  if (isPortalManaged()) {
    if (sessionPath_.isEmpty() && portalStage_ == PortalStage::None)
      startPortal();
    return;
  }

  static bool nativeFilterInstalled = false;
  if (!nativeFilterInstalled) {
    service::GlobalAction::init();
    nativeFilterInstalled = true;
  }
  if (!nativeAction_) {
    nativeAction_ = new QAction(this);
    connect(nativeAction_, &QAction::triggered, this,
            &ShortcutService::activated);
  }
  if (service::GlobalAction::update(nativeAction_, preferredShortcut))
    setStatus(tr("Global shortcut: %1")
                  .arg(preferredShortcut.toString(QKeySequence::NativeText)));
  else
    setStatus(tr("Could not register %1; use the tray Capture action")
                  .arg(preferredShortcut.toString(QKeySequence::NativeText)));
}

bool ShortcutService::isPortalManaged() const
{
  return QGuiApplication::platformName().startsWith("wayland");
}

bool ShortcutService::canConfigurePortal() const
{
  return isPortalManaged() && portalVersion_ >= 2 && !sessionPath_.isEmpty();
}

QString ShortcutService::statusText() const
{
  return status_;
}

void ShortcutService::configurePortal()
{
  if (!canConfigurePortal())
    return;
  QDBusInterface portal(portalService, portalPath, shortcutsInterface,
                        QDBusConnection::sessionBus());
  portal.asyncCall("ConfigureShortcuts", QDBusObjectPath(sessionPath_),
                   QString(), QVariantMap());
}

QString ShortcutService::requestPath(const QString& token) const
{
  auto sender = QDBusConnection::sessionBus().baseService();
  sender.remove(0, 1);
  sender.replace('.', '_');
  return QString("/org/freedesktop/portal/desktop/request/%1/%2")
      .arg(sender, token);
}

void ShortcutService::connectRequest(const QString& path, PortalStage stage)
{
  clearRequest();
  activeRequestPath_ = path;
  portalStage_ = stage;
  QDBusConnection::sessionBus().connect(
      portalService, path, requestInterface, "Response", this,
      SLOT(handleRequestResponse(uint, QVariantMap)));
}

void ShortcutService::startPortal()
{
  qDBusRegisterMetaType<PortalShortcut>();
  qDBusRegisterMetaType<PortalShortcuts>();

  QDBusInterface portal(portalService, portalPath, shortcutsInterface,
                        QDBusConnection::sessionBus());
  if (!portal.isValid()) {
    setStatus(tr("Global-shortcut portal unavailable; use the tray"));
    return;
  }
  portalVersion_ = portal.property("version").toUInt();
  if (portalVersion_ < 1) {
    setStatus(tr("Global-shortcut portal unavailable; use the tray"));
    return;
  }

  const auto token = "st_" + QUuid::createUuid().toString(QUuid::Id128);
  const auto sessionToken =
      "st_session_" + QUuid::createUuid().toString(QUuid::Id128);
  connectRequest(requestPath(token), PortalStage::CreatingSession);

  QVariantMap options;
  options.insert("handle_token", token);
  options.insert("session_handle_token", sessionToken);
  auto call = portal.asyncCall("CreateSession", options);
  auto* watcher = new QDBusPendingCallWatcher(call, this);
  connect(watcher, &QDBusPendingCallWatcher::finished, this,
          [this](QDBusPendingCallWatcher* done) {
            QDBusPendingReply<QDBusObjectPath> reply = *done;
            done->deleteLater();
            if (!reply.isError())
              return;
            clearRequest();
            setStatus(tr("Global-shortcut portal failed: %1")
                          .arg(reply.error().message()));
          });
  setStatus(tr("Waiting for the desktop shortcut portal…"));

  QDBusConnection::sessionBus().connect(
      portalService, portalPath, shortcutsInterface, "Activated", this,
      SLOT(handlePortalActivated(QDBusObjectPath, QString, qulonglong,
                                 QVariantMap)));
}

void ShortcutService::bindPortalShortcut()
{
  QDBusInterface portal(portalService, portalPath, shortcutsInterface,
                        QDBusConnection::sessionBus());
  const auto token = "st_" + QUuid::createUuid().toString(QUuid::Id128);
  connectRequest(requestPath(token), PortalStage::BindingShortcut);

  PortalShortcut capture;
  capture.id = "capture";
  capture.options.insert("description", tr("Capture text from the screen"));
  PortalShortcuts shortcuts{capture};

  QVariantMap options;
  options.insert("handle_token", token);
  auto call =
      portal.asyncCall("BindShortcuts", QDBusObjectPath(sessionPath_),
                       QVariant::fromValue(shortcuts), QString(), options);
  auto* watcher = new QDBusPendingCallWatcher(call, this);
  connect(watcher, &QDBusPendingCallWatcher::finished, this,
          [this](QDBusPendingCallWatcher* done) {
            QDBusPendingReply<QDBusObjectPath> reply = *done;
            done->deleteLater();
            if (!reply.isError())
              return;
            clearRequest();
            setStatus(tr("Could not bind the portal shortcut: %1")
                          .arg(reply.error().message()));
          });
}

void ShortcutService::handleRequestResponse(uint response,
                                            const QVariantMap& results)
{
  const auto stage = portalStage_;
  clearRequest();
  if (response != 0) {
    setStatus(response == 1 ? tr("No global shortcut assigned; use the tray")
                            : tr("The desktop rejected the global shortcut"));
    return;
  }

  if (stage == PortalStage::CreatingSession) {
    sessionPath_ = results.value("session_handle").toString();
    if (sessionPath_.isEmpty()) {
      setStatus(tr("The shortcut portal returned no session"));
      return;
    }
    bindPortalShortcut();
    return;
  }

  if (stage == PortalStage::BindingShortcut)
    setStatus(tr("Global shortcut is managed by the desktop"));
}

void ShortcutService::handlePortalActivated(const QDBusObjectPath& session,
                                            const QString& shortcutId,
                                            qulonglong, const QVariantMap&)
{
  if (session.path() == sessionPath_ && shortcutId == "capture")
    emit activated();
}

void ShortcutService::clearRequest()
{
  if (!activeRequestPath_.isEmpty()) {
    QDBusConnection::sessionBus().disconnect(
        portalService, activeRequestPath_, requestInterface, "Response", this,
        SLOT(handleRequestResponse(uint, QVariantMap)));
  }
  activeRequestPath_.clear();
  portalStage_ = PortalStage::None;
}

void ShortcutService::setStatus(const QString& status)
{
  status_ = status;
  emit statusChanged(status_);
}
}  // namespace v4
