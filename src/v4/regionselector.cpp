#include "regionselector.h"

#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>

namespace v4
{
RegionSelector::RegionSelector(QWidget* parent)
  : QWidget(parent)
{
  setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint |
                 Qt::X11BypassWindowManagerHint);
  setCursor(Qt::CrossCursor);
  setMouseTracking(true);
  setFocusPolicy(Qt::StrongFocus);
}

void RegionSelector::begin(const QPixmap& snapshot, const QRect& screenGeometry)
{
  snapshot_ = snapshot;
  start_ = {};
  current_ = {};
  selecting_ = false;
  setGeometry(screenGeometry);
  show();
  raise();
  activateWindow();
  setFocus(Qt::ActiveWindowFocusReason);
}

QRect RegionSelector::selection() const
{
  return QRect(start_, current_).normalized().intersected(rect());
}

void RegionSelector::paintEvent(QPaintEvent*)
{
  QPainter painter(this);
  painter.drawPixmap(rect(), snapshot_);
  painter.fillRect(rect(), QColor(0, 0, 0, 110));

  const auto area = selection();
  if (selecting_ && !area.isEmpty()) {
    painter.save();
    painter.setClipRect(area);
    painter.drawPixmap(rect(), snapshot_);
    painter.restore();
    painter.setPen(QPen(QColor(64, 160, 255), 2));
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(area.adjusted(0, 0, -1, -1));
  }

  painter.setPen(Qt::white);
  painter.drawText(rect().adjusted(16, 16, -16, -16),
                   Qt::AlignLeft | Qt::AlignTop,
                   tr("Drag to select text • Esc to cancel"));
}

void RegionSelector::keyPressEvent(QKeyEvent* event)
{
  if (event->key() == Qt::Key_Escape) {
    hide();
    emit canceled();
    return;
  }
  QWidget::keyPressEvent(event);
}

void RegionSelector::mousePressEvent(QMouseEvent* event)
{
  if (event->button() != Qt::LeftButton)
    return;
  start_ = current_ = event->position().toPoint();
  selecting_ = true;
  update();
}

void RegionSelector::mouseMoveEvent(QMouseEvent* event)
{
  if (!selecting_)
    return;
  current_ = event->position().toPoint();
  update();
}

void RegionSelector::mouseReleaseEvent(QMouseEvent* event)
{
  if (!selecting_ || event->button() != Qt::LeftButton)
    return;
  current_ = event->position().toPoint();
  selecting_ = false;
  const auto area = selection();
  if (area.width() < 3 || area.height() < 3) {
    hide();
    emit canceled();
    return;
  }
  hide();
  emit selected(area);
}
}  // namespace v4
