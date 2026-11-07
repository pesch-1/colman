// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Implementation of SvgDirectItem.
/**
   @file
   @author Christoph Pesch
*/

#include "colman_card_rendering.hpp"
#ifndef SYSTEM_QPAINTER
#  include <QPainter>
#  define SYSTEM_QPAINTER
#endif

/**
   Constructs the item, positions it at @p target_rect.topLeft(), and
   loads the SVG data into the cached renderer (parsed once, not per paint).
   @param raw         raw SVG bytes.
   @param target_rect position and size in scene coordinates.
   @param orientation rotation angle: 0, 90, 180, or 270.
*/
colman::SvgDirectItem::SvgDirectItem(QByteArray const& raw,
                                     QRectF const& target_rect,
                                     int orientation)
    : renderer_(raw),
      size_(target_rect.size()),
      orientation_(orientation % 360)
{
  setPos(target_rect.topLeft());
}


/**
   @return bounding rectangle in item coordinates.
*/
QRectF colman::SvgDirectItem::boundingRect() const
{
  return QRectF(QPointF(0, 0), size_);
}


/**
   Renders the cached SVG into @p p, applying the stored rotation.
   For 90°/270° the width and height of the render target are swapped.
*/
void colman::SvgDirectItem::paint(QPainter* p,
                                  QStyleOptionGraphicsItem const* /*option*/,
                                  QWidget* /*widget*/)
{
  p->save();
  if(orientation_ != 0) {
    qreal const rw = size_.width();
    qreal const rh = size_.height();
    p->translate(rw / 2.0, rh / 2.0);
    p->rotate(orientation_);
    if((orientation_ % 180) != 0) {
      renderer_.render(p, QRectF(-rh / 2.0, -rw / 2.0, rh, rw));
    }
    else {
      renderer_.render(p, QRectF(-rw / 2.0, -rh / 2.0, rw, rh));
    }
  }
  else {
    renderer_.render(p, QRectF(QPointF(0, 0), size_));
  }
  p->restore();
}

/* EOF */
