// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Declaration of class Code128Item.
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_CODE128_ITEM_HPP
#  define COLMAN_CODE128_ITEM_HPP

#  ifndef SYSTEM_QGRAPHICSITEM
#    include <QGraphicsItem>
#    define SYSTEM_QGRAPHICSITEM
#  endif
#  ifndef SYSTEM_VECTOR
#    include <vector>
#    define SYSTEM_VECTOR
#  endif

namespace colman
{
  /// QGraphicsItem rendering a Code128 barcode.
  /**
     Encodes a text string as a Code128 barcode and paints it using
     alternating black and white bars.  Supports Code128 B (arbitrary
     printable ASCII) and Code128 C (pairs of decimal digits, more compact).
     Use adapt_size() to fit the barcode into a given display rectangle.
  */
  class Code128Item : public QGraphicsItem
  {
  public:
    /// Constructs the barcode item with the given parent.
    explicit Code128Item(QGraphicsItem* parent = nullptr);

    /// Sets the text to be encoded in the barcode.
    /**
       Re-encodes the barcode immediately.
       @param text the string to encode
    */
    void set_text(QString const& text);
    /// Returns the text encoded in the barcode.
    QString text() const;
    /// Sets whether to use Code128 C (true) or Code128 B (false).
    /**
       Code128 C encodes pairs of digits more compactly than Code128 B.
       @param b true for Code128 C, false for Code128 B
    */
    void set_use_code_c(bool b);
    /// Returns true if Code128 C encoding is active.
    bool use_code_c() const;

    /// Sets the bar width reduction used to compensate for ink/heat spread.
    /**
       Thermal printers (especially when set too dark) let the heated dots
       bleed: black bars come out wider than intended and the white gaps
       shrink, so adjacent narrow bars may merge into one.  To compensate,
       every black bar is painted narrower by this amount (split evenly on
       both sides), which keeps the module pitch but widens the white gaps.
       The value is capped per bar so the narrowest bar is never reduced by
       more than half its width.
       @param mm bar width reduction in millimetres (0 disables it)
    */
    void set_bar_width_reduction(qreal mm);
    /// Returns the bar width reduction in millimetres.
    qreal bar_width_reduction() const;

    /// Adapts the barcode dimensions to the given display area.
    /**
       Uses @p width_unit as the X-dimension (narrowest-bar width).  If the
       barcode would exceed @p width at that X-dimension it is scaled down to
       fit exactly.  The barcode is always horizontally centred within @p width.
       @param width      available width in scene units (1 unit = 0.1 mm)
       @param height     available height in scene units
       @param width_unit preferred X-dimension in scene units (0.33 mm = 3.3)
    */
    void adapt_size(qreal width, qreal height, qreal width_unit);
    /// Returns the bounding rectangle of the barcode item.
    QRectF boundingRect() const override;
    /// Paints the barcode bars and the text label below.
    void paint(QPainter* painter,
               QStyleOptionGraphicsItem const* option,
               QWidget* widget = nullptr) override;

  private:
    void encode();
    qreal width_ = 800;
    qreal height_ = 300;
    qreal unit_w_ = 10;
    qreal unit_h_ = 200;
    qreal font_h_ = 80;
    int rl_sum_ = 0;                 ///< sum of run lengths (barcode width in units)
    std::vector<int> run_lengths_; ///< run lengths of barcode (start is black)
    QString text_;                 ///< text to be encoded in barcode
    QRectF rect_;                  ///< rectangle needed for barcode
    bool code_c_ = true;             ///< encode as Code128 C (true) or B (false)
    qreal bar_reduction_mm_ = 0.0; ///< black-bar width reduction in mm (ink spread)
  };

  /// Default X-dimension (narrowest-bar width) of a Code128 barcode in mm.
  /**
     0.33 mm is the smallest module width that common barcode scanners read
     reliably and is the recommended X-dimension for the symbology.  It can be
     overridden by the "barcode_unit_mm" program setting; see
     barcode_unit_scene_units().
  */
  qreal constexpr DefaultBarcodeUnitMm = 0.33;

  /// Returns the configured barcode X-dimension in scene units (1 unit=0.1mm).
  /**
     Reads the "barcode_unit_mm" value from QSettings, falling back to
     DefaultBarcodeUnitMm, and converts it to scene units for adapt_size().
     @return the narrowest-bar width in scene units
  */
  qreal barcode_unit_scene_units();

} // namespace colman

#endif /* #ifndef COLMAN_CODE128_ITEM_HPP */

/* EOF */
