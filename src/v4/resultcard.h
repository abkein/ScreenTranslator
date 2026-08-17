#pragma once

#include <QWidget>

class QLabel;
class QPlainTextEdit;
class QPushButton;

namespace v4
{
class ResultCard : public QWidget
{
  Q_OBJECT
public:
  explicit ResultCard(QWidget* parent = nullptr);

  void showRecognizing();
  void showRecognized(const QString& text, bool translating);
  void showTranslated(const QString& text, const QString& provider);
  void showError(const QString& error, const QString& recognized = {});
  void showLast();

  QString recognizedText() const;
  QString translatedText() const;

signals:
  void retryRequested();

private:
  void positionCard();
  void updateCopyButton();

  QLabel* status_{nullptr};
  QLabel* recognizedTitle_{nullptr};
  QPlainTextEdit* recognized_{nullptr};
  QLabel* translatedTitle_{nullptr};
  QPlainTextEdit* translated_{nullptr};
  QPushButton* copy_{nullptr};
  QPushButton* retry_{nullptr};
};
}  // namespace v4
