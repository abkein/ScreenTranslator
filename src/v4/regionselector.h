#pragma once

#include <QPixmap>
#include <QWidget>

namespace v4
{
class RegionSelector : public QWidget
{
  Q_OBJECT
public:
  explicit RegionSelector(QWidget* parent = nullptr);
  void begin(const QPixmap& snapshot, const QRect& screenGeometry);

signals:
  void selected(const QRect& logicalRect);
  void canceled();

protected:
  void paintEvent(QPaintEvent* event) override;
  void keyPressEvent(QKeyEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;

private:
  QRect selection() const;

  QPixmap snapshot_;
  QPoint start_;
  QPoint current_;
  bool selecting_{false};
};
}  // namespace v4
