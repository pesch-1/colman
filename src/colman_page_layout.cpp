// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Implementation of class PageLayout.
/**
   @file
   @author Christoph Pesch
*/

#include "colman_page_layout.hpp"

/**
   Sets the paper width in mm.
   @param page_width paper width in millimetres.
*/
void colman::PageLayout::set_page_width(qreal page_width)
{
  page_width_ = page_width;
}

/**
   Sets the paper height in mm.
   @param page_height paper height in millimetres.
*/
void colman::PageLayout::set_page_height(qreal page_height)
{
  page_height_ = page_height;
}

/**
   Sets the printer top margin in mm.
   @param printer_top_margin printer top margin in millimetres.
*/
void colman::PageLayout::set_printer_top_margin(qreal printer_top_margin)
{
  printer_top_margin_ = printer_top_margin;
}

/**
   Sets the printer bottom margin in mm.
   @param printer_bottom_margin printer bottom margin in millimetres.
*/
void colman::PageLayout::set_printer_bottom_margin(qreal printer_bottom_margin)
{
  printer_bottom_margin_ = printer_bottom_margin;
}

/**
   Sets the printer left margin in mm.
   @param printer_left_margin printer left margin in millimetres.
*/
void colman::PageLayout::set_printer_left_margin(qreal printer_left_margin)
{
  printer_left_margin_ = printer_left_margin;
}

/**
   Sets the printer right margin in mm.
   @param printer_right_margin printer right margin in millimetres.
*/
void colman::PageLayout::set_printer_right_margin(qreal printer_right_margin)
{
  printer_right_margin_ = printer_right_margin;
}

/**
   Sets the number of label rows per sheet.
   @param rows number of label rows per sheet.
*/
void colman::PageLayout::set_rows(int rows)
{
  rows_ = rows;
}

/**
   Sets the number of label columns per sheet.
   @param columns number of label columns per sheet.
*/
void colman::PageLayout::set_columns(int columns)
{
  columns_ = columns;
}

/**
   Sets the label width in mm.
   @param label_width label width in millimetres.
*/
void colman::PageLayout::set_label_width(qreal label_width)
{
  label_width_ = label_width;
}

/**
   Sets the label height in mm.
   @param label_height label height in millimetres.
*/
void colman::PageLayout::set_label_height(qreal label_height)
{
  label_height_ = label_height;
}

/**
   Sets the horizontal gap between labels in mm.
   @param label_distance_x horizontal gap between labels in millimetres.
*/
void colman::PageLayout::set_label_distance_x(qreal label_distance_x)
{
  label_distance_x_ = label_distance_x;
}

/**
   Sets the vertical gap between labels in mm.
   @param label_distance_y vertical gap between labels in millimetres.
*/
void colman::PageLayout::set_label_distance_y(qreal label_distance_y)
{
  label_distance_y_ = label_distance_y;
}

/**
   Sets the top inner margin of a label in mm.
   @param label_top_margin top inner margin of a label in millimetres.
*/
void colman::PageLayout::set_label_top_margin(qreal label_top_margin)
{
  label_top_margin_ = label_top_margin;
}

/**
   Sets the left inner margin of a label in mm.
   @param label_left_margin left inner margin of a label in millimetres.
*/
void colman::PageLayout::set_label_left_margin(qreal label_left_margin)
{
  label_left_margin_ = label_left_margin;
}

/**
   Sets the bottom inner margin of a label in mm.
   @param label_bottom_margin bottom inner margin of a label in millimetres.
*/
void colman::PageLayout::set_label_bottom_margin(qreal label_bottom_margin)
{
  label_bottom_margin_ = label_bottom_margin;
}

/**
   Sets the right inner margin of a label in mm.
   @param label_right_margin right inner margin of a label in millimetres.
*/
void colman::PageLayout::set_label_right_margin(qreal label_right_margin)
{
  label_right_margin_ = label_right_margin;
}

/**
   Sets whether label borders are drawn.
   @param label_with_frames true to draw a border around each label.
*/
void colman::PageLayout::set_label_with_frames(bool label_with_frames)
{
  label_with_frames_ = label_with_frames;
}

/* EOF */
