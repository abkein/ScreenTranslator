#pragma once

#include <QImage>
#include <QObject>
#include <QString>
#include <QVariantMap>

class QScreen;

namespace v4
{
class RegionSelector;

class CaptureBackend : public QObject
{
  Q_OBJECT
public:
  using QObject::QObject;
  ~CaptureBackend() override = default;

  virtual void capture() = 0;
  virtual QString description() const = 0;

signals:
  void captured(const QImage& image);
  void canceled();
  void error(const QString& message);
};

class DesktopCaptureBackend : public CaptureBackend
{
  Q_OBJECT
public:
  explicit DesktopCaptureBackend(QObject* parent = nullptr);
  ~DesktopCaptureBackend() override;
  void capture() override;
  QString description() const override;

private:
  void finishSelection(const QRect& logicalRect);

  RegionSelector* selector_{nullptr};
  QScreen* screen_{nullptr};
  QImage snapshot_;
};

class PortalCaptureBackend : public CaptureBackend
{
  Q_OBJECT
public:
  explicit PortalCaptureBackend(QObject* parent = nullptr);
  ~PortalCaptureBackend() override;

  void capture() override;
  QString description() const override;

private slots:
  void handleResponse(uint response, const QVariantMap& results);

private:
  QString requestPath(const QString& token) const;
  void clearRequest();

  QString activeRequestPath_;
  bool busy_{false};
};

CaptureBackend* createCaptureBackend(QObject* parent = nullptr);
}  // namespace v4
