// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Plain-data type describing a signature label layout.
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_SIGNATURE_LABEL_LAYOUT_HPP
#  define COLMAN_SIGNATURE_LABEL_LAYOUT_HPP

#  ifndef SYSTEM_QSTRING
#    include <QString>
#    define SYSTEM_QSTRING
#  endif

namespace colman
{

/// Complete layout description for a signature label.
/**
   A zero @c card_width_mm_ indicates that no layout has been
   configured yet.
*/
  struct SignatureLabelLayout
  {
    double card_width_mm_ = 0.0;    ///< label width (mm)
    double card_height_mm_ = 0.0;   ///< label height (mm)
    double margin_mm_ = 0.0;        ///< free margin around edge (mm)
    double text_x_mm_ = 0.0;        ///< text zone left edge (mm)
    double text_y_mm_ = 0.0;        ///< text zone top edge (mm)
    double text_width_mm_ = 0.0;    ///< text zone width (mm)
    double text_height_mm_ = 0.0;   ///< text zone height (mm)
    int text_orientation_ = 0;      ///< text rotation in degrees
    QString label_layout_name_;     ///< name of the last-used label sheet preset
  };

} // namespace colman

#endif /* #ifndef COLMAN_SIGNATURE_LABEL_LAYOUT_HPP */

/* EOF */
