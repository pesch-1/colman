// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Declaration of CardElementItem — shared drag/resize zone for card editors.
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_CARD_ELEMENT_ITEM_HPP
#  define COLMAN_CARD_ELEMENT_ITEM_HPP

#  ifndef COLMAN_ID_CARD_LAYOUT_HPP
#    include "colman_id_card_layout.hpp"
#  endif
#  ifndef SYSTEM_QGRAPHICSITEM
#    include <QGraphicsItem>
#    define SYSTEM_QGRAPHICSITEM
#  endif
#  ifndef SYSTEM_QPIXMAP
#    include <QPixmap>
#    define SYSTEM_QPIXMAP
#  endif
#  ifndef SYSTEM_QBYTEARRAY
#    include <QByteArray>
#    define SYSTEM_QBYTEARRAY
#  endif
#  ifndef SYSTEM_QSTRINGLIST
#    include <QStringList>
#    define SYSTEM_QSTRINGLIST
#  endif

namespace colman
{

/**
   Interactive zone item used in ID card and item card editors.

   Displays its type-specific content (barcode, text or logo) inside
   a coloured, draggable and resizable rectangle.  Resize handles are
   drawn at the eight cardinal / corner positions.

   The logo zone maintains the aspect ratio of the library logo and
   only supports corner handles.  Right-clicking a logo zone with a
   pixmap assigned opens a context menu to rotate it in 90° steps.

   Overlap with sibling CardElementItem instances is prevented: a
   drag that would cause overlap is reverted on mouse release.

   The text zone renders a configurable list of lines; the line at
   index @c emph_line_ is drawn larger and bold (pass -1 for none).
*/
  class CardElementItem : public QGraphicsItem
  {
  public:
  /**
     @param type         zone type (Barcode, Text or Logo).
     @param scene_rect   initial geometry in scene coordinates.
     @param card_bounds  scene rectangle of the card (used to clamp
                         the item inside the card).
  */
    CardElementItem(IdCardElementType type,
                    QRectF const& scene_rect,
                    QRectF const& card_bounds);

  /// Assigns the library logo pixmap (Logo zones only).
    void set_logo_pixmap(QPixmap const& px);

  /// Assigns raw SVG logo bytes; renders vector at paint time (Logo zones).
    void set_logo_raw(QByteArray const& raw);

  /// Sets the barcode text and regenerates the pixmap (Barcode zones only).
    void set_barcode_text(QString const& text);

  /// Sets the text lines shown in the Text zone.
  /**
     @param lines     lines to display (one per row).
     @param emph_line index of the line rendered larger and bold (-1 for none).
  */
    void set_text_lines(QStringList const& lines, int emph_line = -1);

  /// Sets the horizontal text alignment within the Text zone.
  /** Default: Qt::AlignHCenter. */
    void set_text_halign(Qt::AlignmentFlag align);

  /// Controls how the emphasized line is rendered.
  /**
     When @c true the emphasized line is drawn in italic at the same
     size as the other lines (suitable for item cards).  When @c false
     (default) it is drawn bold and larger (suitable for ID cards).
  */
    void set_text_emph_italic(bool italic);

  /// Returns the element type.
    IdCardElementType element_type() const { return type_; }

  /// Returns the current rotation in degrees (0, 90, 180, 270).
    int orientation() const { return orientation_; }

  /// Returns the item geometry as a scene-coordinate rectangle.
    QRectF scene_rect() const;

  /// Sets the initial orientation without changing the rect dimensions.
  /**
     Used when restoring a saved layout: the saved width/height already
     reflect the rotation, so only the rendering orientation needs to be
     updated.  Valid values: 0, 90, 180, 270.
  */
    void set_initial_orientation(int deg);

  /// Rotates the zone by 90°, swapping dimensions.
  /** Does nothing if the rotated rect would exceed the card or overlap a
      sibling. */
    void rotate_90();

  /// Rotates the zone by 180° (dimensions stay the same).
    void rotate_180();

    QRectF boundingRect() const override;
    void paint(QPainter* painter,
               QStyleOptionGraphicsItem const*,
               QWidget*) override;

  /// Updates the card boundary constraint and clamps position within it.
    void set_card_bounds(QRectF const& bounds);

  protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* e) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* e) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* e) override;
    void contextMenuEvent(QGraphicsSceneContextMenuEvent* e) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* e) override;

  private:
  /// Which resize handle is being dragged.
    enum struct Handle {
      NoHandle = 0,
      TopLeft,
      Top,
      TopRight,
      Right,
      BottomRight,
      Bottom,
      BottomLeft,
      Left
    };

    Handle handle_at(QPointF const& p) const;
    void apply_resize(QPointF const& delta);
    void enforce_logo_ratio(Handle h, QRectF& r, QPointF& pos_out) const;

    void draw_handles(QPainter* p) const;
    void draw_barcode(QPainter* p) const;
    void draw_text(QPainter* p) const;
    void draw_logo(QPainter* p) const;
    void regenerate_barcode_pixmap();
    void regenerate_logo_pixmap();

    QRectF free_space_rect() const;
    void align_left();
    void align_right();
    void align_hcenter();
    void align_top();
    void align_bottom();
    void align_vcenter();
    void maximize_in_card();

    bool would_overlap(QRectF const& proposed_rect) const;

    IdCardElementType type_;
    QRectF rect_;              ///< geometry in local coords (origin 0,0)
    QRectF card_bounds_;       ///< card scene rect (clamping constraint)
    int orientation_ = 0;    ///< logo rotation in degrees
    double logo_ratio_ = 1.0;   ///< logo width / height (current rotation)
    QPixmap logo_orig_;        ///< original logo pixmap (Logo, non-SVG)
    QPixmap logo_cache_;       ///< scaled logo for display (Logo, non-SVG)
    QByteArray logo_raw_;         ///< raw SVG bytes (Logo, SVG only)
    bool is_svg_logo_ = false; ///< true when logo_raw_ holds SVG data
    QPixmap barcode_px_;       ///< cached barcode pixmap (Barcode zones)
    QString barcode_text_;     ///< barcode text for regeneration
    QStringList text_lines_;      ///< text lines for the Text zone
    int emph_line_ = -1;     ///< index of the emphasized (bold/large) line
    Qt::AlignmentFlag text_halign_ = Qt::AlignHCenter; ///< horizontal text alignment
    bool text_emph_italic_ = false; ///< true: emph line italic/same size; false: bold/larger

    Handle active_handle_ = Handle::NoHandle;
    bool moving_ = false;
    QPointF drag_pos_;    ///< scene position at drag start
    QPointF item_pos_;    ///< item pos() at drag start
    QRectF drag_rect_;   ///< rect_ at drag start
  };

} // namespace colman

#endif /* #ifndef COLMAN_CARD_ELEMENT_ITEM_HPP */

/* EOF */
