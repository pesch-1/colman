// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Declaration of class TileButton.
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_TILE_BUTTON_HPP
#  define COLMAN_TILE_BUTTON_HPP

#  ifndef SYSTEM_QTOOLBUTTON
#    include <QToolButton>
#    define SYSTEM_QTOOLBUTTON
#  endif

namespace colman
{
  /// Tool button whose icon and text sit at the top instead of the centre.
  /**
     Navigation tiles are given one common fixed size, so a tile with a
     one-line label is higher than its content.  Most styles (e.g. Breeze)
     centre the content vertically, which puts the icons of one row at
     different heights.  TileButton draws the frame over the whole tile but
     the icon and the text in a rectangle of their natural height at the
     top, so that all icons of a row are aligned.
  */
  class TileButton : public QToolButton
  {
  public:
    /// Constructs the button.
    /**
       @param parent optional parent widget.
    */
    explicit TileButton(QWidget* parent = nullptr);

  protected:
    /// Paints the frame over the whole tile and the content at the top.
    /**
       Falls back to QToolButton::paintEvent() if the tile is not higher
       than its natural size.
       @param event the paint event.
    */
    void paintEvent(QPaintEvent* event) override;
  };

} // namespace colman

#endif /* #ifndef COLMAN_TILE_BUTTON_HPP */

/* EOF */
