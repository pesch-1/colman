// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// QGraphicsItem rendering SVG content via a cached QSvgRenderer.
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_CARD_RENDERING_HPP
#  define COLMAN_CARD_RENDERING_HPP

#  ifndef SYSTEM_QBYTEARRAY
#    include <QByteArray>
#    define SYSTEM_QBYTEARRAY
#  endif
#  ifndef SYSTEM_QGRAPHICSITEM
#    include <QGraphicsItem>
#    define SYSTEM_QGRAPHICSITEM
#  endif
#  ifndef SYSTEM_QRECTF
#    include <QRectF>
#    define SYSTEM_QRECTF
#  endif
#  ifndef SYSTEM_QSVGRENDERER
#    include <QSvgRenderer>
#    define SYSTEM_QSVGRENDERER
#  endif

namespace colman
{

/// QGraphicsItem that renders raw SVG bytes via a cached QSvgRenderer.
/**
   Uses direct QPainter commands rather than an intermediate raster, so
   the output is resolution-independent for both screen and printer.
   The SVG is parsed once at construction and cached; subsequent paint()
   calls reuse the renderer without re-parsing the XML.
   Supports 0/90/180/270° rotation; for 90°/270° the target rectangle
   dimensions must already be swapped in @p target_rect.
*/
  class SvgDirectItem : public QGraphicsItem
  {
  public:
  /// Constructs the item at @p target_rect and loads the SVG into the cache.
  /**
     @param raw         raw SVG bytes.
     @param target_rect position and size in scene coordinates.
     @param orientation rotation angle: 0, 90, 180, or 270.
  */
    SvgDirectItem(QByteArray const& raw, QRectF const& target_rect,
                  int orientation = 0);

  /// Returns the bounding rectangle in item coordinates.
    QRectF boundingRect() const override;
  /// Renders the SVG into the painter, applying the stored rotation.
    void paint(QPainter* p,
               QStyleOptionGraphicsItem const* option,
               QWidget* widget = nullptr) override;

  private:
    QSvgRenderer renderer_;   ///< SVG parsed once, reused on every paint()
    QSizeF size_;       ///< item dimensions in scene units
    int orientation_ = 0; ///< rotation in degrees
  };

} // namespace colman

#endif /* #ifndef COLMAN_CARD_RENDERING_HPP */

/* EOF */
