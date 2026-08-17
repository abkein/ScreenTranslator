#include "resultcard.h"

#include <QApplication>
#include <QClipboard>
#include <QCursor>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QScreen>
#include <QVBoxLayout>

#include <algorithm>

namespace v4
{
ResultCard::ResultCard(QWidget* parent)
  : QWidget(parent)
  , status_(new QLabel(this))
  , recognizedTitle_(new QLabel(tr("Recognized"), this))
  , recognized_(new QPlainTextEdit(this))
  , translatedTitle_(new QLabel(tr("Translation"), this))
  , translated_(new QPlainTextEdit(this))
  , copy_(new QPushButton(tr("Copy"), this))
  , retry_(new QPushButton(tr("Retry translation"), this))
{
  setWindowTitle(tr("Screen Translator"));
  setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint);
  resize(460, 320);

  auto* layout = new QVBoxLayout(this);
  status_->setWordWrap(true);
  layout->addWidget(status_);
  layout->addWidget(recognizedTitle_);
  layout->addWidget(recognized_);
  layout->addWidget(translatedTitle_);
  layout->addWidget(translated_);

  recognized_->setReadOnly(true);
  translated_->setReadOnly(true);
  recognized_->setMaximumBlockCount(1000);
  translated_->setMaximumBlockCount(1000);

  auto* buttons = new QHBoxLayout;
  buttons->addWidget(copy_);
  buttons->addWidget(retry_);
  buttons->addStretch();
  auto* close = new QPushButton(tr("Close"), this);
  buttons->addWidget(close);
  layout->addLayout(buttons);

  connect(copy_, &QPushButton::clicked, this, [this] {
    const auto text = translated_->toPlainText().isEmpty()
                          ? recognized_->toPlainText()
                          : translated_->toPlainText();
    QApplication::clipboard()->setText(text);
  });
  connect(retry_, &QPushButton::clicked, this, &ResultCard::retryRequested);
  connect(close, &QPushButton::clicked, this, &QWidget::hide);
}

void ResultCard::showRecognizing()
{
  recognized_->clear();
  translated_->clear();
  status_->setText(tr("Recognizing text…"));
  retry_->setEnabled(false);
  updateCopyButton();
  showLast();
}

void ResultCard::showRecognized(const QString& text, bool translating)
{
  recognized_->setPlainText(text);
  translated_->clear();
  status_->setText(translating ? tr("Translating…")
                               : tr("Recognition complete"));
  retry_->setEnabled(false);
  updateCopyButton();
  showLast();
}

void ResultCard::showTranslated(const QString& text, const QString& provider)
{
  translated_->setPlainText(text);
  status_->setText(tr("Translated with %1").arg(provider));
  retry_->setEnabled(true);
  updateCopyButton();
  showLast();
}

void ResultCard::showError(const QString& error, const QString& recognized)
{
  if (!recognized.isNull())
    recognized_->setPlainText(recognized);
  translated_->clear();
  status_->setText(error);
  retry_->setEnabled(!recognized_->toPlainText().isEmpty());
  updateCopyButton();
  showLast();
}

void ResultCard::showLast()
{
  positionCard();
  show();
  raise();
  activateWindow();
}

QString ResultCard::recognizedText() const
{
  return recognized_->toPlainText();
}

QString ResultCard::translatedText() const
{
  return translated_->toPlainText();
}

void ResultCard::positionCard()
{
  auto* screen = QGuiApplication::screenAt(QCursor::pos());
  if (!screen)
    screen = QGuiApplication::primaryScreen();
  if (!screen)
    return;

  const auto available = screen->availableGeometry();
  auto topLeft = QCursor::pos() + QPoint(18, 18);
  if (topLeft.x() + width() > available.right())
    topLeft.setX(available.right() - width());
  if (topLeft.y() + height() > available.bottom())
    topLeft.setY(available.bottom() - height());
  topLeft.setX(std::max(topLeft.x(), available.left()));
  topLeft.setY(std::max(topLeft.y(), available.top()));
  move(topLeft);
}

void ResultCard::updateCopyButton()
{
  copy_->setEnabled(!recognized_->toPlainText().isEmpty() ||
                    !translated_->toPlainText().isEmpty());
}
}  // namespace v4
