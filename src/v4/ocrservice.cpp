#include "ocrservice.h"

#include "languagecodes.h"
#include "tesseract.h"

#include <QDir>
#include <QFileInfo>
#include <QFutureWatcher>
#include <QSet>
#include <QtConcurrent>

namespace v4
{
namespace
{
QStringList candidates(const QString& overridePath)
{
  QStringList result;
  if (!overridePath.trimmed().isEmpty())
    result << QDir::cleanPath(overridePath.trimmed());

  const auto environmentPath = qEnvironmentVariable("TESSDATA_PREFIX");
  if (!environmentPath.isEmpty()) {
    result << QDir::cleanPath(environmentPath);
    result << QDir(environmentPath).filePath("tessdata");
  }

#ifdef ST_DEFAULT_TESSDATA_DIR
  result << QStringLiteral(ST_DEFAULT_TESSDATA_DIR);
#endif
  result << "/usr/share/tesseract-ocr/5/tessdata"
         << "/usr/share/tesseract-ocr/4.00/tessdata"
         << "/usr/share/tessdata";
  result.removeDuplicates();
  return result;
}
}  // namespace

OcrService::OcrService(QObject* parent)
  : QObject(parent)
  , watcher_(new QFutureWatcher<OcrResult>(this))
{
  qRegisterMetaType<OcrResult>();
  connect(watcher_, &QFutureWatcher<OcrResult>::finished, this,
          [this] { emit finished(watcher_->result()); });
}

OcrService::~OcrService()
{
  watcher_->waitForFinished();
}

OcrCatalog OcrService::discover(const QString& overridePath)
{
  for (const auto& path : candidates(overridePath)) {
    QDir dir(path);
    if (!dir.exists())
      continue;

    QStringList languages;
    const auto files =
        dir.entryList({"*.traineddata"}, QDir::Files, QDir::Name);
    for (const auto& file : files) {
      const auto code = QFileInfo(file).completeBaseName();
      if (code == "osd" || code.endsWith("_equ"))
        continue;
      languages << LanguageCodes::idForTesseract(code);
    }
    languages.removeDuplicates();
    if (!languages.isEmpty())
      return {dir.absolutePath(), languages};
  }
  return {};
}

void OcrService::recognize(const QImage& image, const QString& tessdataPath,
                           const QString& languageId)
{
  if (watcher_->isRunning()) {
    emit finished({{}, tr("Recognition is already in progress")});
    return;
  }

  watcher_->setFuture(
      QtConcurrent::run([image, tessdataPath, languageId]() -> OcrResult {
        Tesseract engine(languageId, tessdataPath);
        if (!engine.isValid())
          return {{}, engine.error()};
        const auto text = engine.recognize(image);
        return {text, text.isEmpty() ? engine.error() : QString()};
      }));
}

bool OcrService::isBusy() const
{
  return watcher_->isRunning();
}
}  // namespace v4
