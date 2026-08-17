#pragma once

#include "stfwd.h"

#include <QString>
#include <QImage>

#include <memory>

class QPixmap;
class Task;
namespace tesseract
{
class TessBaseAPI;
}

class Tesseract
{
public:
  Tesseract(const LanguageId& language, const QString& tessdataPath);
  ~Tesseract();

  QString recognize(const QPixmap& source);
  QString recognize(const QImage& source);
  bool isValid() const;
  const QString& error() const;

  static QStringList availableLanguageNames(const QString& path);

private:
  void init(const LanguageId& language, const QString& tessdataPath);

  std::unique_ptr<tesseract::TessBaseAPI> api_;
  QString error_;
};
