// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Plain-data types describing an ID card layout.
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_ID_CARD_LAYOUT_HPP
#  define COLMAN_ID_CARD_LAYOUT_HPP

namespace colman
{

  /// Specifies which content a zone on an ID card displays.
  enum struct IdCardElementType { Barcode,
                                  Text,
                                  Logo };

  /// Position, size and orientation of one zone on an ID card.
  struct IdCardElement
  {
    IdCardElementType type_ = IdCardElementType::Barcode;
    int orientation_ = 0;   ///< rotation in degrees (0, 90, 180, 270)
    double x_mm_ = 0.0;        ///< left edge relative to card left (mm)
    double y_mm_ = 0.0;        ///< top edge relative to card top (mm)
    double width_mm_ = 0.0;    ///< zone width (mm)
    double height_mm_ = 0.0;   ///< zone height (mm)
  };

  /// Complete layout description for a library ID card.
  /**
     A zero @c card_width_mm_ indicates that no layout has been
     configured yet.
  */
  struct IdCardLayout
  {
    double card_width_mm_ = 0.0;   ///< card width (longer side, mm)
    double card_height_mm_ = 0.0;  ///< card height (shorter side, mm)
    IdCardElement barcode_;      ///< barcode zone
    IdCardElement text_;         ///< text zone (library name, date, name)
    IdCardElement logo_;         ///< logo zone (only valid when has_logo_)
    bool has_logo_ = false;        ///< true when a library logo is configured
  };

  /// Builds a sensible default zone arrangement for a card of the given size.
  /**
     Used as a fallback when a selected layout carries no saved arrangement
     (e.g. a geometry-only preset that was never opened in the editor): a
     text band of @p text_height_fraction across the top (with a logo to its
     left when @p has_logo is true) and the barcode filling the area below.
     @param w_mm                 card width in mm.
     @param h_mm                 card height in mm.
     @param has_logo             whether to reserve a logo zone.
     @param text_height_fraction fraction of the height used for the top band.
     @return the default layout (card dimensions set, zones placed).
  */
  inline IdCardLayout default_card_arrangement(
      double w_mm, double h_mm, bool has_logo,
      double text_height_fraction)
  {
    IdCardLayout lay;
    lay.card_width_mm_ = w_mm;
    lay.card_height_mm_ = h_mm;
    lay.has_logo_ = has_logo;
    double const top_h = h_mm * text_height_fraction;
    lay.barcode_.type_ = IdCardElementType::Barcode;
    lay.barcode_.x_mm_ = 0.0;
    lay.barcode_.y_mm_ = top_h;
    lay.barcode_.width_mm_ = w_mm;
    lay.barcode_.height_mm_ = h_mm - top_h;
    double const logo_w = has_logo ? w_mm * 0.25 : 0.0;
    lay.logo_.type_ = IdCardElementType::Logo;
    lay.logo_.x_mm_ = 0.0;
    lay.logo_.y_mm_ = 0.0;
    lay.logo_.width_mm_ = logo_w;
    lay.logo_.height_mm_ = top_h;
    lay.text_.type_ = IdCardElementType::Text;
    lay.text_.x_mm_ = logo_w;
    lay.text_.y_mm_ = 0.0;
    lay.text_.width_mm_ = w_mm - logo_w;
    lay.text_.height_mm_ = top_h;
    return lay;
  }

} // namespace colman

#endif /* #ifndef COLMAN_ID_CARD_LAYOUT_HPP */

/* EOF */
