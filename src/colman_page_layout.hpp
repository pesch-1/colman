// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Declaration of class PageLayout.
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_PAGE_LAYOUT_HPP
#  define COLMAN_PAGE_LAYOUT_HPP

#  ifndef SYSTEM_QTGLOBAL
#    include <QtGlobal>
#    define SYSTEM_QTGLOBAL
#  endif

namespace colman
{

  /// Layout parameters for a sheet of barcode labels.
  /**
     Stores the paper size, printer margins, and the physical arrangement of
     labels on the paper.  All lengths are in millimetres.  Defaults correspond
     to A4 paper with 3 × 8 Herma-style labels.
  */
  class PageLayout
  {
  public:
    /// Returns the paper width in mm.
    inline qreal get_page_width() const { return page_width_; }
    /// Returns the paper height in mm.
    inline qreal get_page_height() const { return page_height_; }
    /// Returns the printer top margin in mm.
    inline qreal get_printer_top_margin() const
    {
      return printer_top_margin_;
    }
    /// Returns the printer bottom margin in mm.
    inline qreal get_printer_bottom_margin() const
    {
      return printer_bottom_margin_;
    }
    /// Returns the printer left margin in mm.
    inline qreal get_printer_left_margin() const
    {
      return printer_left_margin_;
    }
    /// Returns the printer right margin in mm.
    inline qreal get_printer_right_margin() const
    {
      return printer_right_margin_;
    }
    /// Returns the number of label rows per sheet.
    inline int get_rows() const { return rows_; }
    /// Returns the number of label columns per sheet.
    inline int get_columns() const { return columns_; }
    /// Returns the label width in mm.
    inline qreal get_label_width() const { return label_width_; }
    /// Returns the label height in mm.
    inline qreal get_label_height() const { return label_height_; }
    /// Returns the horizontal gap between labels in mm.
    inline qreal get_label_distance_x() const { return label_distance_x_; }
    /// Returns the vertical gap between labels in mm.
    inline qreal get_label_distance_y() const { return label_distance_y_; }
    /// Returns the top inner margin of a label in mm.
    inline qreal get_label_top_margin() const { return label_top_margin_; }
    /// Returns the left inner margin of a label in mm.
    inline qreal get_label_left_margin() const
    {
      return label_left_margin_;
    }
    /// Returns the bottom inner margin of a label in mm.
    inline qreal get_label_bottom_margin() const
    {
      return label_bottom_margin_;
    }
    /// Returns the right inner margin of a label in mm.
    inline qreal get_label_right_margin() const
    {
      return label_right_margin_;
    }
    /// Returns true if label borders are drawn.
    inline bool get_label_with_frames() const
    {
      return label_with_frames_;
    }

    /// Sets the paper width in mm.
    void set_page_width(qreal page_width);
    /// Sets the paper height in mm.
    void set_page_height(qreal page_height);
    /// Sets the printer top margin in mm.
    void set_printer_top_margin(qreal printer_top_margin);
    /// Sets the printer bottom margin in mm.
    void set_printer_bottom_margin(qreal printer_bottom_margin);
    /// Sets the printer left margin in mm.
    void set_printer_left_margin(qreal printer_left_margin);
    /// Sets the printer right margin in mm.
    void set_printer_right_margin(qreal printer_right_margin);
    /// Sets the number of label rows per sheet.
    void set_rows(int rows);
    /// Sets the number of label columns per sheet.
    void set_columns(int columns);
    /// Sets the label width in mm.
    void set_label_width(qreal label_width);
    /// Sets the label height in mm.
    void set_label_height(qreal label_height);
    /// Sets the horizontal gap between labels in mm.
    void set_label_distance_x(qreal label_distance_x);
    /// Sets the vertical gap between labels in mm.
    void set_label_distance_y(qreal label_distance_y);
    /// Sets the top inner margin of a label in mm.
    void set_label_top_margin(qreal label_top_margin);
    /// Sets the left inner margin of a label in mm.
    void set_label_left_margin(qreal label_left_margin);
    /// Sets the bottom inner margin of a label in mm.
    void set_label_bottom_margin(qreal label_bottom_margin);
    /// Sets the right inner margin of a label in mm.
    void set_label_right_margin(qreal label_right_margin);
    /// Sets whether label borders are drawn.
    void set_label_with_frames(bool label_with_frames);

  private:
    // paper size (mm):
    qreal page_width_ = 210;              ///< paper width in mm
    qreal page_height_ = 297;             ///< paper height in mm
    // printer margins (mm):
    qreal printer_top_margin_ = 5;        ///< printer top margin in mm
    qreal printer_bottom_margin_ = 5;     ///< printer bottom margin in mm
    qreal printer_left_margin_ = 5;       ///< printer left margin in mm
    qreal printer_right_margin_ = 5;      ///< printer right margin in mm
    // physical layout of labels on paper:
    int rows_ = 8;                        ///< label rows per sheet
    int columns_ = 3;                     ///< label columns per sheet
    qreal label_width_ = 70;              ///< label width in mm
    qreal label_height_ = 37;             ///< label height in mm
    qreal label_distance_x_ = 0;          ///< horizontal gap between labels in mm
    qreal label_distance_y_ = 0;          ///< vertical gap between labels in mm
    qreal label_top_margin_ = 0;          ///< top inner margin of a label in mm
    qreal label_left_margin_ = 0;         ///< left inner margin of a label in mm
    qreal label_bottom_margin_ = 0;       ///< bottom inner margin of a label in mm
    qreal label_right_margin_ = 0;        ///< right inner margin of a label in mm
    bool label_with_frames_ = false;      ///< true if label borders are drawn
  };

} // namespace colman

#endif /* #ifndef COLMAN_PAGE_LAYOUT_HPP */

/* EOF */
