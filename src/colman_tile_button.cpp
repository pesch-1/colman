// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Implementation of class TileButton.
/**
   @file
   @author Christoph Pesch
*/

#include "colman_tile_button.hpp"

#ifndef SYSTEM_QSTYLEOPTIONTOOLBUTTON
#  include <QStyleOptionToolButton>
#  define SYSTEM_QSTYLEOPTIONTOOLBUTTON
#endif
#ifndef SYSTEM_QSTYLEPAINTER
#  include <QStylePainter>
#  define SYSTEM_QSTYLEPAINTER
#endif

/**
   Constructs an empty tile button.
   @param parent optional parent widget.
*/
colman::TileButton::TileButton(QWidget* parent)
    : QToolButton(parent)
{
}


/**
   Draws the button in two passes: first the complex control without icon
   and text, which yields the frame and background over the whole tile,
   then the label (icon and text) in a rectangle that is shortened at the
   bottom by the height the tile exceeds its natural size hint.  The style
   thus lays out the content as in a tile of natural height, aligned to the
   top edge.
   @param event the paint event.
*/
void colman::TileButton::paintEvent(QPaintEvent* event)
{
  auto const extra = height() - sizeHint().height();
  if(extra <= 0) {
    QToolButton::paintEvent(event);
    return;
  }
  QStylePainter painter(this);
  QStyleOptionToolButton opt;
  initStyleOption(&opt);

  auto bevel = opt;
  bevel.text.clear();
  bevel.icon = QIcon();
  painter.drawComplexControl(QStyle::CC_ToolButton, bevel);

  auto label = opt;
  auto const fw = style()->pixelMetric(QStyle::PM_DefaultFrameWidth, &opt,
                                       this);
  label.rect = style()
                   ->subControlRect(QStyle::CC_ToolButton, &opt,
                                    QStyle::SC_ToolButton, this)
                   .adjusted(fw, fw, -fw, -fw - extra);
  painter.drawControl(QStyle::CE_ToolButtonLabel, label);
}

/* EOF */
