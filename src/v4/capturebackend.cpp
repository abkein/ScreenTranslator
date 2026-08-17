#include "capturebackend.h"

#include "regionselector.h"

#include <QCursor>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusObjectPath>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QGuiApplication>
#include <QImageReader>
#include <QScreen>
#include <QUrl>
#include <QUuid>

namespace v4
{
namespace
{
constexpr auto portalService = "org.freedesktop.portal.Desktop";
constexpr auto portalPath = "/org/freedesktop/portal/desktop";
constexpr auto screenshotInterface = "org.freedesktop.portal.Screenshot";
constexpr auto requestInterface = "org.freedesktop.portal.Request";
}  // namespace

DesktopCaptureBackend::DesktopCaptureBackend(QObject* parent)
  : CaptureBackend(parent)
  , selector_(new RegionSelector)
{
  selector_->setAttribute(Qt::WA_DeleteOnClose, false);
  connect(selector_, &RegionSelector::selected, this,
          &DesktopCaptureBackend::finishSelection);
  connect(selector_, &RegionSelector::canceled, this,
          &DesktopCaptureBackend::canceled);
}

DesktopCaptureBackend::~DesktopCaptureBackend()
{
  delete selector_;
}

void DesktopCaptureBackend::capture()
{
  screen_ = QGuiApplication::screenAt(QCursor::pos());
  if (!screen_)
    screen_ = QGuiApplication::primaryScreen();
  if (!screen_) {
    emit error(tr("No screen is available for capture"));
    return;
  }

  const auto pixmap = screen_->grabWindow(0);
  if (pixmap.isNull()) {
    emit error(tr("The desktop could not be captured"));
    return;
  }
  snapshot_ = pixmap.toImage();
  selector_->begin(pixmap, screen_->geometry());
}

QString DesktopCaptureBackend::description() const
{
  return tr("Built-in single-screen selector");
}

void DesktopCaptureBackend::finishSelection(const QRect& logicalRect)
{
  if (snapshot_.isNull() || !screen_) {
    emit error(tr("The captured screen is no longer available"));
    return;
  }

  const auto logicalSize = screen_->geometry().size();
  const qreal scaleX = snapshot_.width() / qreal(logicalSize.width());
  const qreal scaleY = snapshot_.height() / qreal(logicalSize.height());
  const QRect pixels(qRound(logicalRect.x() * scaleX),
                     qRound(logicalRect.y() * scaleY),
                     qRound(logicalRect.width() * scaleX),
                     qRound(logicalRect.height() * scaleY));
  const auto image = snapshot_.copy(pixels.intersected(snapshot_.rect()));
  if (image.isNull()) {
    emit error(tr("The selected area is empty"));
    return;
  }
  emit captured(image);
}

PortalCaptureBackend::PortalCaptureBackend(QObject* parent)
  : CaptureBackend(parent)
{
}

PortalCaptureBackend::~PortalCaptureBackend()
{
  clearRequest();
}

QString PortalCaptureBackend::requestPath(const QString& token) const
{
  auto sender = QDBusConnection::sessionBus().baseService();
  sender.remove(0, 1);
  sender.replace('.', '_');
  return QString("/org/freedesktop/portal/desktop/request/%1/%2")
      .arg(sender, token);
}

void PortalCaptureBackend::capture()
{
  if (busy_)
    return;

  QDBusInterface portal(portalService, portalPath, screenshotInterface,
                        QDBusConnection::sessionBus());
  if (!portal.isValid()) {
    emit error(tr("The desktop screenshot portal is unavailable"));
    return;
  }

  const auto version = portal.property("version").toUInt();
  if (version < 2) {
    emit error(tr("The screenshot portal is too old; version 2 is required"));
    return;
  }
  if (version >= 3 && !(portal.property("AvailableTargets").toUInt() & 4U)) {
    emit error(tr("The screenshot portal does not support area capture"));
    return;
  }

  const auto token = "st_" + QUuid::createUuid().toString(QUuid::Id128);
  activeRequestPath_ = requestPath(token);
  auto bus = QDBusConnection::sessionBus();
  if (!bus.connect(portalService, activeRequestPath_, requestInterface,
                   "Response", this, SLOT(handleResponse(uint, QVariantMap)))) {
    activeRequestPath_.clear();
    emit error(tr("Could not listen for the screenshot portal response"));
    return;
  }

  QVariantMap options;
  options.insert("handle_token", token);
  options.insert("interactive", true);
  if (version >= 3)
    options.insert("target", uint(4));  // Area

  busy_ = true;
  auto call = portal.asyncCall("Screenshot", QString(), options);
  auto* watcher = new QDBusPendingCallWatcher(call, this);
  connect(watcher, &QDBusPendingCallWatcher::finished, this,
          [this](QDBusPendingCallWatcher* done) {
            QDBusPendingReply<QDBusObjectPath> reply = *done;
            done->deleteLater();
            if (!reply.isError())
              return;
            clearRequest();
            emit error(tr("Screenshot portal request failed: %1")
                           .arg(reply.error().message()));
          });
}

QString PortalCaptureBackend::description() const
{
  return tr("Wayland desktop portal area picker");
}

void PortalCaptureBackend::handleResponse(uint response,
                                          const QVariantMap& results)
{
  clearRequest();
  if (response == 1) {
    emit canceled();
    return;
  }
  if (response != 0) {
    emit error(tr("The screenshot portal rejected the request"));
    return;
  }

  const QUrl url(results.value("uri").toString());
  QImageReader reader(url.toLocalFile());
  const auto image = reader.read();
  if (image.isNull()) {
    emit error(tr("Could not read the portal screenshot: %1")
                   .arg(reader.errorString()));
    return;
  }
  emit captured(image);
}

void PortalCaptureBackend::clearRequest()
{
  if (!activeRequestPath_.isEmpty()) {
    QDBusConnection::sessionBus().disconnect(
        portalService, activeRequestPath_, requestInterface, "Response", this,
        SLOT(handleResponse(uint, QVariantMap)));
    activeRequestPath_.clear();
  }
  busy_ = false;
}

CaptureBackend* createCaptureBackend(QObject* parent)
{
  if (QGuiApplication::platformName().startsWith("wayland"))
    return new PortalCaptureBackend(parent);
  return new DesktopCaptureBackend(parent);
}
}  // namespace v4
