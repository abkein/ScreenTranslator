#pragma once

#include <QImage>
#include <QObject>
#include <QStringList>

template <typename T>
class QFutureWatcher;

namespace v4
{
struct OcrCatalog {
  QString path;
  QStringList languageIds;

  bool isValid() const { return !path.isEmpty() && !languageIds.isEmpty(); }
};

struct OcrResult {
  QString text;
  QString error;
};

class OcrService : public QObject
{
  Q_OBJECT
public:
  explicit OcrService(QObject* parent = nullptr);
  ~OcrService() override;

  static OcrCatalog discover(const QString& overridePath = {});
  void recognize(const QImage& image, const QString& tessdataPath,
                 const QString& languageId);
  bool isBusy() const;

signals:
  void finished(const v4::OcrResult& result);

private:
  QFutureWatcher<OcrResult>* watcher_{nullptr};
};
}  // namespace v4

Q_DECLARE_METATYPE(v4::OcrResult)
