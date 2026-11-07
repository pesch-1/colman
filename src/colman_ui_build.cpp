// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Definitions of the main-window UI build helpers.
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_UI_BUILD_HPP
#  include "colman_ui_build.hpp"
#endif
#ifndef SYSTEM_QWIDGET
#  include <QWidget>
#  define SYSTEM_QWIDGET
#endif
#ifndef SYSTEM_QABSTRACTITEMVIEW
#  include <QAbstractItemView>
#  define SYSTEM_QABSTRACTITEMVIEW
#endif
#ifndef SYSTEM_QSPLITTER
#  include <QSplitter>
#  define SYSTEM_QSPLITTER
#endif
#ifndef SYSTEM_QLABEL
#  include <QLabel>
#  define SYSTEM_QLABEL
#endif
#ifndef SYSTEM_QPIXMAP
#  include <QPixmap>
#  define SYSTEM_QPIXMAP
#endif
#ifndef SYSTEM_QLAYOUTITEM
#  include <QLayoutItem>
#  define SYSTEM_QLAYOUTITEM
#endif
#ifndef SYSTEM_QMARGINS
#  include <QMargins>
#  define SYSTEM_QMARGINS
#endif
#ifndef SYSTEM_QRECT
#  include <QRect>
#  define SYSTEM_QRECT
#endif
#ifndef SYSTEM_QPOINT
#  include <QPoint>
#  define SYSTEM_QPOINT
#endif
#ifndef SYSTEM_QSIZE
#  include <QSize>
#  define SYSTEM_QSIZE
#endif

void colman::set_size_policy(QWidget* w, QSizePolicy::Policy h,
                             QSizePolicy::Policy v, int hstretch, int vstretch)
{
  QSizePolicy policy(h, v);
  policy.setHorizontalStretch(static_cast<quint8>(hstretch));
  policy.setVerticalStretch(static_cast<quint8>(vstretch));
  policy.setHeightForWidth(w->sizePolicy().hasHeightForWidth());
  w->setSizePolicy(policy);
}

void colman::configure_item_view(QAbstractItemView* view)
{
  view->setAlternatingRowColors(true);
  view->setSelectionMode(QAbstractItemView::SingleSelection);
  view->setSelectionBehavior(QAbstractItemView::SelectRows);
}

QSplitter* colman::make_splitter(QWidget* parent, Qt::Orientation orientation)
{
  auto* splitter = new QSplitter(parent);
  splitter->setOrientation(orientation);
  return splitter;
}

QLabel* colman::make_icon_label(QWidget* parent, char const* resource,
                                int max_size)
{
  auto* label = new QLabel(parent);
  set_size_policy(label, QSizePolicy::Fixed, QSizePolicy::Fixed);
  label->setMaximumSize(QSize(max_size, max_size));
  label->setPixmap(QPixmap(QString::fromUtf8(resource)));
  label->setScaledContents(true);
  return label;
}

QFont colman::bold_font()
{
  QFont font;
  font.setBold(true);
  return font;
}

colman::FlowLayout::FlowLayout(QWidget* parent, int margin, int h_spacing,
                               int v_spacing)
    : QLayout(parent), h_space_(h_spacing), v_space_(v_spacing)
{
  setContentsMargins(margin, margin, margin, margin);
}

colman::FlowLayout::FlowLayout(int margin, int h_spacing, int v_spacing)
    : h_space_(h_spacing), v_space_(v_spacing)
{
  setContentsMargins(margin, margin, margin, margin);
}

colman::FlowLayout::~FlowLayout()
{
  while(QLayoutItem* item = takeAt(0)) {
    delete item;
  }
}

void colman::FlowLayout::addItem(QLayoutItem* item)
{
  item_list_.append(item);
}

int colman::FlowLayout::horizontal_spacing() const
{
  if(h_space_ >= 0) {
    return h_space_;
  }
  return smart_spacing_(QStyle::PM_LayoutHorizontalSpacing);
}

int colman::FlowLayout::vertical_spacing() const
{
  if(v_space_ >= 0) {
    return v_space_;
  }
  return smart_spacing_(QStyle::PM_LayoutVerticalSpacing);
}

Qt::Orientations colman::FlowLayout::expandingDirections() const
{
  return {};
}

bool colman::FlowLayout::hasHeightForWidth() const
{
  return true;
}

int colman::FlowLayout::heightForWidth(int width) const
{
  return do_layout_(QRect(0, 0, width, 0), true);
}

int colman::FlowLayout::count() const
{
  return static_cast<int>(item_list_.size());
}

QLayoutItem* colman::FlowLayout::itemAt(int index) const
{
  return item_list_.value(index);
}

QLayoutItem* colman::FlowLayout::takeAt(int index)
{
  if(index >= 0 && index < item_list_.size()) {
    return item_list_.takeAt(index);
  }
  return nullptr;
}

QSize colman::FlowLayout::minimumSize() const
{
  QSize size;
  for(QLayoutItem const* item : item_list_) {
    size = size.expandedTo(item->minimumSize());
  }
  QMargins const margins = contentsMargins();
  size += QSize(margins.left() + margins.right(),
                margins.top() + margins.bottom());
  return size;
}

QSize colman::FlowLayout::sizeHint() const
{
  return minimumSize();
}

void colman::FlowLayout::setGeometry(QRect const& rect)
{
  QLayout::setGeometry(rect);
  do_layout_(rect, false);
}

int colman::FlowLayout::do_layout_(QRect const& rect, bool test_only) const
{
  int left = 0;
  int top = 0;
  int right = 0;
  int bottom = 0;
  getContentsMargins(&left, &top, &right, &bottom);
  QRect const effective = rect.adjusted(left, top, -right, -bottom);
  int x = effective.x();
  int y = effective.y();
  int line_height = 0;

  for(QLayoutItem* item : item_list_) {
    QWidget const* wid = item->widget();
    int space_x = horizontal_spacing();
    if(space_x == -1) {
      space_x = wid->style()->layoutSpacing(
          QSizePolicy::PushButton, QSizePolicy::PushButton, Qt::Horizontal);
    }
    int space_y = vertical_spacing();
    if(space_y == -1) {
      space_y = wid->style()->layoutSpacing(
          QSizePolicy::PushButton, QSizePolicy::PushButton, Qt::Vertical);
    }

    int next_x = x + item->sizeHint().width() + space_x;
    if(next_x - space_x > effective.right() && line_height > 0) {
      x = effective.x();
      y = y + line_height + space_y;
      next_x = x + item->sizeHint().width() + space_x;
      line_height = 0;
    }

    if(!test_only) {
      item->setGeometry(QRect(QPoint(x, y), item->sizeHint()));
    }

    x = next_x;
    line_height = qMax(line_height, item->sizeHint().height());
  }
  return y + line_height - rect.y() + bottom;
}

int colman::FlowLayout::smart_spacing_(QStyle::PixelMetric pm) const
{
  QObject const* parent = this->parent();
  if(!parent) {
    return -1;
  }
  if(parent->isWidgetType()) {
    auto const* parent_widget = static_cast<QWidget const*>(parent);
    return parent_widget->style()->pixelMetric(pm, nullptr, parent_widget);
  }
  return static_cast<QLayout const*>(parent)->spacing();
}

/* EOF */
